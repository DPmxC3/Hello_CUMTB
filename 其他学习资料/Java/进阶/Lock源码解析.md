# ReentrantLock()干了啥

看了`美团技术团队`分享的这篇AQS觉着自己确实是白学了。想要深入理解AQS还是推荐看：https://tech.meituan.com/2019/12/05/aqs-theory-and-apply.html


在lock的构造函数中，定义了一个`NonFairSync`。

```java
public ReentrantLock() {
    sync = new NonfairSync();
}
public ReentrantLock(boolean fair) {
    sync = fair ? new FairSync() : new NonfairSync();
}
```

看看`NonfairSync`.

```java
private final Sync sync;
static final class NonfairSync extends Sync
```

在看`Sync`

```java
abstract static class Sync extends AbstractQueuedSynchronizer
```

一步一步网上找，找到了这个`AbstractQueuedSynchronizer(AQS)`,最后到了这个，又是继承于`AbstractOwnableSynchronizer(AOS)`, AOS主要是保存获取当前锁的线程对象。这里不在展开。主要看看继承结构图：

![20210305193443](https://user-images.githubusercontent.com/57765968/112604046-b4655600-8e50-11eb-9d4e-d71ba2d67e7d.png)

FairSync 与 NonfairSync的区别在于，是不是保证获取锁的公平性，因为默认是NonfairSync，我们以这个为例了解其背后的原理。

其他几个类代码不多，最后的主要代码都是在AQS中，我们先看看这个类的主体结构。



# AbstractQueuedSynchronizer是什么

![20210305193418](https://user-images.githubusercontent.com/57765968/112604057-b6c7b000-8e50-11eb-8cf0-9ba8cc4a0b5f.png)

我们在看看Node是什么

![image-20210305172226207](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210305193425.png)

看到这应该明白了，这......不就是**双向链表**吗!!!

**最后我们可以发现锁的存储结构就两个东西:"双向链表" + "int类型状态"**需要注意的是，他们的变量都被" `transient`和`volatile`修饰。

![165e52d59ae1b838](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210305193428.jpg)

一个int值和双向链表是如何处理的呢？让我们来看看`Doug Lea`大神的操作。



# lock.lock()怎么获取锁

我们看到调用的是`NonfairSync.lock()`

```java
final void lock() {
    if (compareAndSetState(0, 1)) // 就是这里修改state转变状态
        setExclusiveOwnerThread(Thread.currentThread());
    else
        acquire(1);
}
```

看到这里，我们基本有了一个大概的了解，还记得之前AQS中的int类型的state值，这里就是通过CAS（乐观锁）去修改state的值。 **lock的基本操作还是通过乐观锁来实现的**。

获取锁通过CAS，那么没有获取到锁，等待获取锁是如何实现的？我们可以看一下else分支的逻辑，acquire方法：

```java
public final void acquire(int arg) {
    if (!tryAcquire(arg) &&
        acquireQueued(addWaiter(Node.EXCLUSIVE), arg))
        selfInterrupt();
}
```

这里干了三件事情：

- tryAcquire：会尝试再次通过CAS获取一次锁。
- addWaiter：将当前线程加入上面锁的双向链表（等待队列）中
- acquireQueued：通过自旋，判断当前队列节点是否可以获取锁。


waitStatus有下面几个枚举值：

| 枚举      | 含义                                           |
| :-------- | :--------------------------------------------- |
| 0         | 当一个Node被初始化的时候的默认值               |
| CANCELLED | 为1，表示线程获取锁的请求已经取消了            |
| CONDITION | 为-2，表示节点在等待队列中，节点线程等待唤醒   |
| PROPAGATE | 为-3，当前线程处在SHARED情况下，该字段才会使用 |
| SIGNAL    | 为-1，表示线程已经准备好了，就等资源释放了     |
## TryAcquire

state=0表示还没有被线程持有锁，直接通过CAS修改，能修改成功的就获取到锁，修改失败的线程先判断exclusiveOwnerThread是不是当前线程，是则state+1，表示重入次数+1并返回true，加锁成功，否则则返回false表示尝试加锁失败并调用**acquireQueued**入队。

```java
protected final boolean tryAcquire(int acquires) {
    return nonfairTryAcquire(acquires);
}
final boolean nonfairTryAcquire(int acquires) {
    final Thread current = Thread.currentThread();
    int c = getState();
    if (c == 0) {
        if (compareAndSetState(0, acquires)) {
            setExclusiveOwnerThread(current);
            return true;
        }
    }
    else if (current == getExclusiveOwnerThread()) {
        int nextc = c + acquires;
        if (nextc < 0) // overflow
        throw new Error("Maximum lock count exceeded");
        setState(nextc);
        return true;
    }
    return false;
}
```

## addWaiter 添加当前线程到等待链表中

```java
private Node addWaiter(Node mode) {
    Node node = new Node(Thread.currentThread(), mode);
    // Try the fast path of enq; backup to full enq on failure
    Node pred = tail; // 尾节点表示是否初始化了
    // 尾节点不为null表示已经存在队列，直接将当前线程作为尾节点
    if (pred != null) {
        node.prev = pred;
        // CAS保证线程安全情况下插入到链表尾部
        if (compareAndSetTail(pred, node)) { 
            pred.next = node;
            return node;
        }
    }
    // 尾结点不存在则表示还没有初始化队列，需要初始化队列
    enq(node);
    return node;
}
private Node enq(final Node node) {
    for (;;) {
        Node t = tail;
        if (t == null) { // Must initialize
            if (compareAndSetHead(new Node()))
                tail = head;
        } else {
            node.prev = t;
            if (compareAndSetTail(t, node)) {
                t.next = node;
                return t;
            }
        }
    }
}
```

这里还有个细节的地方，为什么设置尾节点时都要先将之前的尾节点设置为node.pre的值呢，而不是CAS之后再设置?如下：

```java
if (compareAndSetTail(pred, node)) {
    node.prev = pred;
    pred.next = node;
    return node;
}
```

因为如果这样做的话，在CAS设置完tail后会存在一瞬间的tail.pre=null的情况，而Doug Lea正是考虑到这种情况，不论何时获取tail.pre都不会为null。

## acquireQueued

总的来说，一个线程获取锁失败了，被放入等待队列，acquireQueued会把放入队列中的线程不断去获取锁，直到获取成功或者不再需要获取（中断）。来看看源码：
```java
// java.util.concurrent.locks.AbstractQueuedSynchronizer

private void cancelAcquire(Node node) {
  // 将无效节点过滤
	if (node == null)
		return;
  // 设置该节点不关联任何线程，也就是虚节点
	node.thread = null;
	Node pred = node.prev;
  // 通过前驱节点，跳过取消状态的node
	while (pred.waitStatus > 0)
		node.prev = pred = pred.prev;
  // 获取过滤后的前驱节点的后继节点
	Node predNext = pred.next;
  // 把当前node的状态设置为CANCELLED
	node.waitStatus = Node.CANCELLED;
  // 如果当前节点是尾节点，将从后往前的第一个非取消状态的节点设置为尾节点
  // 更新失败的话，则进入else，如果更新成功，将tail的后继节点设置为null
	if (node == tail && compareAndSetTail(node, pred)) {
		compareAndSetNext(pred, predNext, null);
	} else {
		int ws;
    // 如果当前节点不是head的后继节点，1:判断当前节点前驱节点的是否为SIGNAL，2:如果不是，则把前驱节点设置为SINGAL看是否成功
    // 如果1和2中有一个为true，再判断当前节点的线程是否为null
    // 如果上述条件都满足，把当前节点的前驱节点的后继指针指向当前节点的后继节点
		if (pred != head && ((ws = pred.waitStatus) == Node.SIGNAL || (ws <= 0 && compareAndSetWaitStatus(pred, ws, Node.SIGNAL))) && pred.thread != null) {
			Node next = node.next;
			if (next != null && next.waitStatus <= 0)
				compareAndSetNext(pred, predNext, next);
		} else {
      // 如果当前节点是head的后继节点，或者上述条件不满足，那就唤醒当前节点的后继节点
			unparkSuccessor(node);
		}
		node.next = node; // help GC
	}
}

private static boolean shouldParkAfterFailedAcquire(Node pred, Node node) {
    int ws = pred.waitStatus;
    if (ws == Node.SIGNAL)
        return true;
    if (ws > 0) {
        do {
            node.prev = pred = pred.prev;
        } while (pred.waitStatus > 0);
        pred.next = node;
    } else {
        compareAndSetWaitStatus(pred, ws, Node.SIGNAL);
    }
    return false;
}
private final boolean parkAndCheckInterrupt() {
    LockSupport.park(this);
    return Thread.interrupted();
}
```

这里就是队列中线程加锁/睡眠的核心逻辑，首先判断刚刚调用addWaiter方法添加到队列的节点是否是头节点，如果是则再次尝试加锁，这个刚刚分析过了，非公平锁在这里就会再次抢一次锁，抢锁成功则设置为head节点并返回打断标记；否则则和公平锁一样调用**shouldParkAfterFailedAcquire**判断是否应该调用park方法进入睡眠。





# lock.unlock()

```java
public void unlock() {
    sync.release(1);
}
public final boolean release(int arg) {
    if (tryRelease(arg)) {
        Node h = head;
        if (h != null && h.waitStatus != 0)
            unparkSuccessor(h);
        return true;
    }
    return false;
}

protected final boolean tryRelease(int releases) {
    
    int c = getState() - releases;
    if (Thread.currentThread() != getExclusiveOwnerThread())
        throw new IllegalMonitorStateException();
    boolean free = false;
    if (c == 0) {
        free = true;
        setExclusiveOwnerThread(null);
    }
    
    setState(c);
    return free;
}
```

**解锁就比较简单了，先调用tryRelease对state执行减一操作，如果state==0，则表示完全释放锁；若果存在后继节点，则调用unparkSuccessor唤醒后继节点，唤醒后的节点的waitStatus会重新被设置为0.**
![未命名文件 (1)](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210305193748.png)

从队列中释放节点的疑虑打消了，那么又有新问题了：

- shouldParkAfterFailedAcquire中取消节点是怎么生成的呢？什么时候会把一个节点的waitStatus设置为-1？
- 是在什么时间释放节点通知到被挂起的线程呢？

# CANCELLED状态节点生成

acquireQueued方法中的Finally代码：

```java
// java.util.concurrent.locks.AbstractQueuedSynchronizer

final boolean acquireQueued(final Node node, int arg) {
	boolean failed = true;
	try {
    ...
		for (;;) {
			final Node p = node.predecessor();
			if (p == head && tryAcquire(arg)) {
				...
				failed = false;
        ...
			}
			...
	} finally {
		if (failed)
			cancelAcquire(node);
		}
}
```

通过cancelAcquire方法，将Node的状态标记为CANCELLED。接下来，我们逐行来分析这个方法的原理：

```java
// java.util.concurrent.locks.AbstractQueuedSynchronizer

private void cancelAcquire(Node node) {
  // 将无效节点过滤
	if (node == null)
		return;
  // 设置该节点不关联任何线程，也就是虚节点
	node.thread = null;
	Node pred = node.prev;
  // 通过前驱节点，跳过取消状态的node
	while (pred.waitStatus > 0)
		node.prev = pred = pred.prev;
  // 获取过滤后的前驱节点的后继节点
	Node predNext = pred.next;
  // 把当前node的状态设置为CANCELLED
	node.waitStatus = Node.CANCELLED;
  // 如果当前节点是尾节点，将从后往前的第一个非取消状态的节点设置为尾节点
  // 更新失败的话，则进入else，如果更新成功，将tail的后继节点设置为null
	if (node == tail && compareAndSetTail(node, pred)) {
		compareAndSetNext(pred, predNext, null);
	} else {
		int ws;
    // 如果当前节点不是head的后继节点，1:判断当前节点前驱节点的是否为SIGNAL，2:如果不是，则把前驱节点设置为SINGAL看是否成功
    // 如果1和2中有一个为true，再判断当前节点的线程是否为null
    // 如果上述条件都满足，把当前节点的前驱节点的后继指针指向当前节点的后继节点
		if (pred != head && ((ws = pred.waitStatus) == Node.SIGNAL || (ws <= 0 && compareAndSetWaitStatus(pred, ws, Node.SIGNAL))) && pred.thread != null) {
			Node next = node.next;
			if (next != null && next.waitStatus <= 0)
				compareAndSetNext(pred, predNext, next);
		} else {
      // 如果当前节点是head的后继节点，或者上述条件不满足，那就唤醒当前节点的后继节点
			unparkSuccessor(node);
		}
		node.next = node; // help GC
	}
}
```

当前的流程：

- 获取当前节点的前驱节点，如果前驱节点的状态是CANCELLED，那就一直往前遍历，找到第一个waitStatus <= 0的节点，将找到的Pred节点和当前Node关联，将当前Node设置为CANCELLED。
- 根据当前节点的位置，考虑以下三种情况：

(1) 当前节点是尾节点。

(2) 当前节点是Head的后继节点。

(3) 当前节点不是Head的后继节点，也不是尾节点。

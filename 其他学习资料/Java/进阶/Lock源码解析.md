# ReentrantLock()干了啥

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

![image-20210305171835645](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210305193443.png)

FairSync 与 NonfairSync的区别在于，是不是保证获取锁的公平性，因为默认是NonfairSync，我们以这个为例了解其背后的原理。

其他几个类代码不多，最后的主要代码都是在AQS中，我们先看看这个类的主体结构。



# AbstractQueuedSynchronizer是什么

![image-20210305172104494](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210305193418.png)

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

```java
final boolean acquireQueued(final Node node, int arg) {
    boolean failed = true;
    try {
        boolean interrupted = false;
        for (;;) {
            final Node p = node.predecessor();
            if (p == head && tryAcquire(arg)) {
                setHead(node);
                p.next = null; // help GC
                failed = false;
                return interrupted;
            }
            if (shouldParkAfterFailedAcquire(p, node) &&
                parkAndCheckInterrupt())
                interrupted = true;
        }
    } finally {
        if (failed)
            cancelAcquire(node);
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

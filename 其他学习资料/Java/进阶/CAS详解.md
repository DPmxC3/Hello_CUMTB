# 什么是CAS

CAS(Compare-and-Swap)，即比较并替换，是一种实现并发算法时常用到的技术，Java并发包中很多类都是用了CAS技术。

先来看一个例子：

```java
    private static int race = 0;

    public static void increase() {
        race++;
    }

    @Test
    public void demo1() throws InterruptedException {
        CountDownLatch countDownLatch = new CountDownLatch(20);
        for (int i = 0; i < 20; i++) {
            new Thread(() -> {
                for (int j = 0; j < 1000; j++) {
                    increase();
                }
                countDownLatch.countDown();
            }).start();
        }
        countDownLatch.await();
        System.out.println(race);

    }
```



上面这个例子在volatile关键字详解文中用过，我们知道，运行完这段代码之后，并不会获得期望的结果，而且会发现每次运行程序，输出的结果都不一样，都是一个小于20000的数字。

原因是通过字节码发现`race++`并不是一个原子操作。那怎么办呢？

我们首先想到的就是通过加锁实现

```java
public synchronized static void increase() {
    race++;
}
```

但是使用synchronized修饰后，性能方面可能有些问题。那么还有办法吗？当然有！这时候我们可以用Java并发包原子操作类(Atomic开头)，例如以下代码：

```java
private static int race = 0;

private static AtomicInteger atomicInteger = new AtomicInteger(0);

public synchronized static void increase() {
    race++;
}

public static void increase1() {
    atomicInteger.getAndIncrement();
}

@Test
public void demo1() throws InterruptedException {
    CountDownLatch countDownLatch = new CountDownLatch(20);
    for (int i = 0; i < 20; i++) {
        new Thread(() -> {
            for (int j = 0; j < 1000; j++) {
                increase1();
            }
            countDownLatch.countDown();
        }).start();
    }
    countDownLatch.await();
    System.out.println(atomicInteger);

}
```

我们将例子中的代码稍做修改：`race`改成使用`AtomicInteger`定义，`race++`改成使用`AtomicInteger.getAndIncrement()`是原子操作，因此我们可以确保每次都可以获得正确的结果，并且在性能上有不错的提升。

我们来看一下源码：

```java
public final int getAndIncrement() { // 方法签名
    // 这里的valueOffset表示偏移量，1是固定的，this表示我们创建的AtomicInteger对象
    return unsafe.getAndAddInt(this, valueOffset, 1);
}
// 注意，此处的valueOffset在我们初始化AtomicInteger的时候已经初始化好了
static {
    try {
        valueOffset = unsafe.objectFieldOffset
            (AtomicInteger.class.getDeclaredField("value"));
    } catch (Exception ex) { throw new Error(ex); }
}
```

继续点进去最终的方法是这个：

```java
public final int getAndAddInt(Object var1, long var2, int var4) {
    int var5;
    do {
        var5 = this.getIntVolatile(var1, var2);
    } while(!this.compareAndSwapInt(var1, var2, var5, var5 + var4));

    return var5;
}
```

这样看着参数会舒服一点：

![image-20210222175052946](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210223131612.png)



**getAndAddInt方法解析：**`getIntBolatile`此方法为拿到内存位置的最新值var5，使用CAS尝试将内存位置的值修改为目标值var5 + var4，如果修改失败，则获取该内存位置的新值v，然后，继续尝试，直到修改成功。



CAS是英文单词**CompareAndSwap**的缩写,中文意思是：比较并替换。CAS需要三个操作数：内存地址V，旧的预期值A，即将要更新的目标值B。

CAS指令执行时，当且仅当内存地址V的值与预期值A相等时，将内存地址V的值修改为B，否则就什么都不做。整个比较并替换的操作是一个原子操作。



## Unsafe类

UnSafe是CAS的核心类，由于Java方法无法直接访问底层系统，需要通过本地（native）方法来访问， UnSafe相当于一个后门，基于该类可以直接操作特定内存的数据，Unsafe类存在于 sun.misc包中，其 内部方法操作可以像C的指针一样直接操作内存，因为Java中CAS操作的执行依赖于Unsafe类的方法。 

**注意：Unsafe类中的所有方法都是Native修饰的，也就是说Unsafe类中的方法都直接调用操作系统底层资源执行相应任务**



# CAS的缺点

* 循环时间长，开销大
  * 可以看到源码中存在一个do...while操作，如果CAS失败就会一直进行尝试。
* 只能保证一个共享变量的原子操作
* ABA问题

前面两点可以很好理解，那这个ABA问题是个啥呢？我们来看看吧~

## ABA问题

CAS算法实现一个重要前提：需要取出内存中某时刻的数据并在当下时刻比较并交换，那么在这个时间差内会导致数据的变化

比如说一个线程one从内存位置V中取出A，这个时候另一个线程two也从内存中取出A，并且线程two进行了一些操作将值变成了B，然后线程two又将 V位置的数据变成A，这时候线程one进行CAS操作发现内 存中仍然是A，然后线程one操作成功。 

`尽管线程one的CAS操作成功，但是不代表这个过程就是没问题的。`

## 原子引用AtomicReference

```java
package zou;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.ToString;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

/**
 * @author VirtualR
 * @date 2021/2/22
 * @blog http://zouyishan.com
 * @email 2983748714@qq.com
 */
public class atomic {
    public static void main(String[] args) throws InterruptedException {
        AtomicReference<Integer> atomicReference = new AtomicReference<>(new Integer(0));
        CountDownLatch countDownLatch = new CountDownLatch(10);
        for (int i = 0; i < 10; i++) {
            new Thread(() -> {
                for (int j = 0; j < 2000; j++) {
                    while (true) {
                        Integer old = atomicReference.get();
                        if (atomicReference.compareAndSet(old, old + 1)) break;
                    }
                }
                countDownLatch.countDown();
            }).start();
        }
        countDownLatch.await();
        System.out.println(atomicReference.get());
    }
}
```

上述示例，最终打印“20000”。

该示例并没有使用锁，而是使用**自旋+CAS**的无锁操作保证共享变量的线程安全。10个线程，每个线程对金额增加2000，最终结果为20000，如果线程不安全，最终结果应该会小于20000。

通过示例，可以总结出AtomicReference的一般使用模式如下：

```java
AtomicReference<Object> ref = new AtomicReference<>(new Object());
Object oldCache = ref.get();

// 对缓存oldCache做一些操作
Object newCache  =  someFunctionOfOld(oldCache); 

// 如果期间没有其它线程改变了缓存值，则更新
boolean success = ref.compareAndSet(oldCache , newCache);
```



## 怎么解决ABA问题

ABA问题的解决类似于乐观锁。就是我们有一个版本号，如果对原值进行了修改就增加1。

类似于这样：

> T1     100   1
> T2     100   1   =>   101    2   =>  100    3

例如(ABA)案例：

```java
package zou;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

/**
 * @author VirtualR
 * @date 2021/2/22
 * @blog http://zouyishan.com
 * @email 2983748714@qq.com
 */
public class aba {
    public static void main(String[] args) {
        AtomicReference<Integer> atomicReference = new AtomicReference<>(new Integer(200));
        new Thread(() -> {
            Integer old = atomicReference.get();
            atomicReference.compareAndSet(old, old + 1);
            Integer old1 = atomicReference.get();
            atomicReference.compareAndSet(old1, old1 - 1);
        }).start();

        new Thread(() -> {
            try {
                TimeUnit.SECONDS.sleep(2);
                Integer old = atomicReference.get();
                System.out.println(atomicReference.compareAndSet(old, old + 1000));
                System.out.println(atomicReference.get());
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }).start();
    }
}
```

上面的输出为：

> true
> 1200

这里我们的one线程已经修改过这个值，但是因为值是不变的，修改过后还是原来的值，在进行比较的时候返回的是true。就有问题！



**如何解决**

其实针对这个问题为我们提供了一个类`AtomicStampedReference`。

还是看看如何解决以上案例：

```java
package zou;

import java.util.concurrent.atomic.AtomicStampedReference;

/**
 * @author VirtualR
 * @date 2021/2/22
 * @blog http://zouyishan.com
 * @email 2983748714@qq.com
 */
public class abar {
    public static void main(String[] args) {
        AtomicStampedReference<Integer> stampedReference = new
                AtomicStampedReference<>(new Integer(1000), 1);
        int stampBegin = stampedReference.getStamp(); // 开始的版本号

        new Thread(() -> {
            Integer old = stampedReference.getReference();
            stampedReference.compareAndSet(old, old + 10,
                    stampedReference.getStamp(), stampedReference.getStamp() + 1);
            Integer old1 = stampedReference.getReference();
            stampedReference.compareAndSet(old, old - 10,
                    stampedReference.getStamp(), stampedReference.getStamp() + 1);
        }).start();

        new Thread(() -> {
            System.out.println("新的版本号" + stampedReference.getStamp() + "\n" +
                    "以前的版本号" + stampBegin);
            Integer old = stampedReference.getReference();
            System.out.println(stampedReference.compareAndSet(old, old + 10000,
                    stampBegin, stampBegin + 1));
        }).start();
    }
}
```

以上代码的输出就为:

> 新的版本号2
> 以前的版本号1
> false

这要通过这个类，加上一个版本号，这个ABA问题就能完美解决啦！

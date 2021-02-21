
# JMM

**什么是JMM**: JMM本身是一种抽象的概念，并不真实存在，他描述的是一组规则或者规范！

通过定义这种"Java内存模型"来屏蔽各种硬件和操作系统的内存访问差异，以实现让Java程序在各种平台下都能达到一致的内存访问效果。在此之前**主流程序语言(如C和C++)直接使用物理硬件和操作系统的内存模型**，因此，**由于不同平台上内存模型的差异，可能导致程序在一套平台上并发完全正常，而在另外一套平台上并发访问却经常出错**，所以在某些场景下必须针对不同的平台来编写程序。



## 主内存和工作内存

Java内存模型的主要目的是**定义程序中各种变量的访问规则，即关注在虚拟机中把变量值存储到内存和内存中取出变量值这样的底层细节。**

为了获得更好的执行效能，Java内存模型并没有限制执行引擎使用处理器的特定寄存器或缓存来和主内存进行交互，**也没有限制即时编译器是否要进行调整代码执行顺序这类优化措施**

Java内存模型规定了所有的变量都存储在主内存(虚拟机内存的一部分)中。每条线程还有自己的工作内存，线程的工作内存中保存了该线程使用的变量的主内存副本，线程对变量的所有操作(读取，赋值)都必须在工作内存中进行，而不能直接读写主内存中的数据。不同的线程之间也无法直接访问对方工作内存中的变量，**线程间变量值的传递均需要通过主内存来完成**。关系图如下：

![image-20210221142128856](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210221230805.png)



这里所讲的主内存，工作内存和Java堆，栈，方法区并不是同一个层次对内存的划分，这两者基本上是没有任何关系的。如果一定要勉强对应起来，**那么从变量，主内存，工作内存的定义来看，主内存主要对应于Java堆中的对象实例数据部分(Java堆还保存了对象的其他信息，堆HotSpot虚拟机来讲，有Mark Word，对象哈希码，GC标志..........)，而工作内存则对应虚拟机栈中的部分区域。从更基础的层次上说，主内存直接对应于物理硬件的内存**，而为了更好的运行速度，虚拟机可能会让工作内存优先存储于寄存器和告诉缓存中，因为程序运行时主要访问的是工作内存。

TIPS：

* 如果局部变量是一个reference类型，它引用的对象在Java堆中可被各个线程共享，但是reference本身在Java栈的局部变量表中是线程私有的。

* 有部分读者会对这段描述中的“副本”提出疑问，如“假设线程中访问一个10MB大小的对象，也会把 这10MB的内存复制一份出来吗？”，事实上并不会如此，**这个对象的引用、对象中某个在`线程访问到的字段是有可能被复制的`，但不会有虚拟机把整个对象复制一次。 **

* 根据《Java虚拟机规范》的约定，volatile变量依然有工作内存的拷贝，但是由于它**特殊的操作顺序性规定，所以看起来如同直接在主内存中读写访问一般**，因此这里的描述对于volatile也并不存在例外。

  

## 内存间的交互操作

关于主内存与工作内存之间具体的交互协议，即一个变量如何从主内存拷贝到工作内存，如何从工作内存同步回主内存这一类的实现细节，Java内存模型中定义了以下8中操作来完成。**Java虚拟机实现时必须保证下面提及的每一种操作都是原子的，不可再分的(double和long类型的变量load,store,read和write操作在某些平台可能有例外)**

* lock(锁定)：作用于主内存的变量，它把一个变量标识为一条线程独占的状态。
* unlock(解锁): 作用于主内存的变量，它把一个处于锁定状态的变量释放出来，释放后的变量才可以被其他线程锁定。
* read(读取): 作用于主内存的变量，它把一个变量的值从主内存传输到线程的工作内存中，以便随后的load动作使用。
* load(载入): 作用于工作内存的变量，它把read操作从主内存中得到的变量值放入工作内存的变量副本中。
* use(使用): 作用于工作内存的变量，它把工作内存中一个变量的值传递给执行引擎，每当虚拟机遇到一个需要使用变量的值的字节码指令时将会执行这个操作。
* assign(赋值): 作用于工作内存的变量，它把一个从执行引擎接收的值赋给工作内存的变量， 每当虚拟机遇到一个给变量赋值的字节码指令时执行这个操作。
* store(存储): 作用于工作内存的变量，它把工作内存中一个变量的值传送到主内存中，以便随后的write操作使用。
* write(写入): 作用于主内存的变量，它把store操作从工作内存中得到的变量的值放入主内存的变量中。

如果要把一个变量从主内存拷贝到工作内存，那就要**按顺序执行read和load操作**，如果要把变量从工作内存同步回主内存，就要**按顺序执行store和write操作**。但可以不必是连续，如可以是以下顺序：read a, read b, load b, load a。除此之外，还应该满足以下规则：

* 不允许read和load、store和write操作之一单独出现。即使用了read必须load，使用了store必须 write 
* 不允许线程丢弃他近的assign操作，即工作变量的数据改变了之后，必须告知主存 不允许一个线程将没有assign的数据从工作内存同步回主内存 
* 一个新的变量必须在主内存中诞生，不允许工作内存直接使用一个未被初始化的变量。就是对变量 实施use、store操作之前，必须经过assign和load操作 
* 一个变量同一时间只有一个线程能对其进行lock。多次lock后，必须执行相同次数的unlock才能解 锁 
* 如果对一个变量进行lock操作，会清空所有工作内存中此变量的值，在执行引擎使用这个变量前， 必须重新load或assign操作初始化变量的值 
* 如果一个变量没有被lock，就不能对其进行unlock操作。也不能unlock一个被其他线程锁住的变量 
* 对一个变量进行unlock操作之前，必须把此变量同步回主内存。





# 对于volatile型变量的特殊规则

关键字volatile可以说是Java虚拟机提供的最轻量级的同步机制，但是它并不容易被正确、完整地理解。但了解volatile变量的语义对后面理解多线程操作的其他特性很有意义。

Java内存模型为volatile专门定义了一些特殊的访问规则，我们先简单说一说这个关键字的作用:

## Volatile的两大特性

### 可见性

* 保证此**变量对所有线程的可见性**，这里的"可见性"指当一条线程修改了这个变量的值，新值对于其他线程来说是可以立即得知的。而普通变量并不能做到这一点，普通变量的值在线程间传递时需要通过主内存来完成。

> 关于volatile变量的可见性，经常会被开发人员误解，会误以为这个描述是正确的："volatile变量对所有线程是理解可见的，对volatile变量所有的写操作都能立刻反映到其他线程中，换句话说，volatile变量在各个线程中是一致的，所以基于volatile变量的运算在并发下是线程安全的"。不能得到"基于volatile变量的运算在并发下是线程安全的"这样的结论。`volatile变量在各个线程的工作内存中是不能存在一致性问题的(每个volatile变量可以存在不一致，但是每次使用都要刷新，执行引擎看不到不一致的情况，因此可以任务不存在一致性问题)，但是Java里面的运算操作符并非原子操作，这导致volatile变量的运算在并发下一样是不安全的`

**验证一下volatile的可见性：**

```java
package zou.Volatile;

/**
 * @author 2983748714@qq.com
 * @date 2021/2/21 15:56
 * @blog http://zouyishan.com/ [尊重原创]
 */
public class test1 {
    private static int num = 0;
    public static void main(String[] args) throws InterruptedException {
        new Thread(() -> {
            while (num == 0) {

            }
            System.out.println(num);
        }).start();
        Thread.sleep(1000);
        num = 1;
    }
}
```

只要是num的签名没有volatile我们新开的线程是跳不出循环的。当为volatile的时候就能！

**验证一下volatile的不保证原子性:**

```java
package zou.Volatile;

import java.util.concurrent.CountDownLatch;

/**
 * @author 2983748714@qq.com
 * @date 2021/2/21 16:24
 * @blog http://zouyishan.com/ [尊重原创]
 */
public class test2 {
    public static volatile int race = 0;
    public static void increase() {
        race++;
    }
    public static void main(String[] args) throws InterruptedException {
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
}
```

我们看下输出：

![image-20210221172654760](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210221230815.png)

这里可以看到并没有像我们预期的一样输出20000。我们来看看反编译后的代码：

![image-20210221162915082](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210221230818.png)

可以看到这里的`race++`并不像我们想的那么简单。其中有4条字节码，我们看一看过程：

* getstatic: 这里volatile可以保证是相同的，此字节码的意思是将race这个变量放入栈顶
* iconst_1: 将int类型1放入栈顶
* iadd: 将栈顶两个元素相加，结果压入栈顶。
* putstatic: 将栈顶元素赋值给race

volatile在`getstatic`阶段能保证相同，但是如果很多线程都进入了iadd阶段，将相加结果放入栈顶，`这个栈顶值现在不是race，不能改变`。然后一个线程执行了`putstatic`，对race已经做了修改，但是另一个线程的`栈顶元素并没有修改,是一个过期元素`，然后这个线程执行`putstatic`就会导致将较小的值赋给race。

实事求是地说，使用字节码分析并发问题不够严谨，**因为即使编译出来就一条字节指令，也不意味着这条指令就是一个原子操作。**可能底层要有很多代码才能实现它的语义。因为这里的字节码已经能很好说明问题了，所以就用字节码来解释。`所以这里也解释了volatile不具有原子性`



**volatile的使用场景**

```java
volatile boolean shutdownRequested;
public void shutdown() {    
    shutdownRequested = true; 
} 
public void doWork() {    
    while (!shutdownRequested) {
        // 代码的业务逻辑    
    } 
}
```

而这种场景下用volatile变量来控制并发就非常好，当shutdown()方法被调用时，能保证所有线程中执行的dowork()方法都立即停下来



### 禁止指令重排

计算机在执行程序，为了提高性能，编译器和处理器常常会对指令做重排，一般分为一下三种：

![6464086-bd967393fb8abcbe](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210221230826.png)

单线程环境里面确保程序终执行结果和代码顺序执行的结果一致。

处理器在进行重排序时必须要考虑指令之间的数据依赖性。

多线程环境中线程交替执行，由于编译器优化重排的存在，**两个线程中使用的变量能否保证一致性是无 法确定的，结果无法预测。**例如：

```java
package zou.Volatile;

/**
 * @author 2983748714@qq.com
 * @date 2021/2/21 17:31
 * @blog http://zouyishan.com/ [尊重原创]
 */
public class test3 {
    public static void main(String[] args) {
        int x = 11; // 1
        int y = 12; // 2
        x = x + 5;  // 3
        y = x * x;  // 4
    }
}
```

指令重排顺序可以为：1234, 2134, 1324

但是绝不可能4先执行。

volatile实现了禁止指令重排优化，从而避免多线程环境下程序出现乱序执行的现象。

先了解一个概念，内存屏障(Memory Barrier)，是一个CPU命令，其作用有两个：

1. 保证特定操作的顺序执行。
2. 保证某些变量的内存可见性(利用该特性实现volatile的内存可见性)

由于编译器和处理器都能执行指令重排优化。如果在指令间插入一条 Memory Barrier 则会告诉编译器 和CPU，不管什么指令都不能和这条 Memory Barrier 指令重排序，也就是说，**通过插入内存屏障禁止 在内存屏障前后的指令执行重排序优化**。内存屏障另外一个作用是**强制刷出各种CPU的缓存数据，因此 任何CPU上的线程都能读取到这些数据的新版本。**



## 懒汉单例解释

所以在我们的一种懒汉单例中用volatile就很好理解了：

```java
package zou.DCL;

/**
 * @author 2983748714@qq.com
 * @date 2021/2/20 21:04
 * @blog http://zouyishan.com/ [尊重原创]
 */
public class Lazy {
    private Lazy() {

    }

    private static volatile Lazy lazy = null;

    public Lazy getLazy() {
        if (lazy == null) {
            synchronized (Lazy.class) {
                if (lazy == null) {
                    lazy = new Lazy();
                }
            }
        }
        return lazy;
    }

    public static void main(String[] args) {

    }
}
```



```java
lazy = new Lazy();
```

但是以上new不是原子性操作，至少会经过三个步骤：

1. 分配对象内存空间 
2. 执行构造方法初始化对象 
3. 设置instance指向刚分配的内存地址，此时instance != null；

由于指令重排，导致A线程执行`lazy = new Lazy;`的时候，**可能先执行了第三步（还没执行第二步）**，此时线程B又进来了，发现lazyMan已经不为空了，直接返回了lazy，并且后面使用了返回的lazy，由于线程A还没有执行第二步，导致此时lazyMan还不完整，可能会有一些意想不到的错误

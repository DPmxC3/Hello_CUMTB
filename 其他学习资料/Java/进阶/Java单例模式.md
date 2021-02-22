# 单例模式

单例模式本身很简单，就是两个，一个懒汉式，一个饿汉式，但是值得思考的是其中的原理。直接贴下代码：

**饿汉式：**

```java
package zou.DCL;

/**
 * @author 2983748714@qq.com
 * @date 2021/2/20 21:06
 * @blog http://zouyishan.com/ [尊重原创]
 */
public class Eager {
    private Eager() {

    }

    private static Eager eager = new Eager();

    public Eager getEager() {
        return eager;
    }

    public static void main(String[] args) {

    }
}
```



**懒汉式：**

> 加锁版

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

> 静态内部类版本

```java
package zou.DCL;

/**
 * @author 2983748714@qq.com
 * @date 2021/2/20 23:49
 * @blog http://zouyishan.com/ [尊重原创]
 */
public class Eager1 {

    private Eager1() {}
    
    public Eager1 getInstance() {
        return SingleTonHoler.instance;
    }
    
    public static class SingleTonHoler {
        private static final Eager1 instance = new Eager1();
    }
    public static void main(String[] args) {

    }
}
```

> 枚举版本

```java
public enum SingleTon{
  INSTANCE;
        public void method(){
        //TODO
     }
}
```



饿汉模式在类被初始化时就已经在内存中创建了对象，故不存在线程安全问题。

懒汉模式在方法被调用后才创建对象，在多线程环境下存在风险。

我们就来主要分析一下懒汉模式。



# 懒汉分析

**加锁版：**

如果指令重排搞不清楚可以看一下我的[这篇文章](http://zouyishan.com/read/76696b4c886648db91b95cb43b8b3289)

```java
lazy = new Lazy();
```

但是以上new不是原子性操作，至少会经过三个步骤：

1. 分配对象内存空间 
2. 执行构造方法初始化对象 
3. 设置instance指向刚分配的内存地址，此时instance != null；

由于指令重排，导致A线程执行`lazy = new Lazy;`的时候，**可能先执行了第三步（还没执行第二步）**，此时线程B又进来了，发现lazyMan已经不为空了，直接返回了lazy，并且后面使用了返回的lazy，由于线程A还没有执行第二步，导致此时lazyMan还不完整，可能会有一些意想不到的错误





**静态内部类版：**

调用的是`SingleTonHoler.INSTANCE`，取的是SingleTonHoler里的INSTANCE对象，跟上面那个DCL方法不同的是，getInstance()方法并没有多次去new对象，故不管多少个线程去调用getInstance()方法，取的都是同一个INSTANCE对象，而不用去重新创建。当getInstance()方法被调用时，SingleTonHoler才在SingleTon的运行时常量池里，把符号引用替换为直接引用，这时静态对象INSTANCE也真正被创建，然后再被getInstance()方法返回出去，这点同饿汉模式。那么INSTANCE在创建过程中又是如何保证线程安全的呢？在《深入理解JAVA虚拟机》中，有这么一句话:

> 虚拟机会保证一个类的< clinit>()方法在多线程环境中被正确地加锁、同步，如果多个线程同时去初始化一个类，那么只会有一个线程去执行这个类的< clinit>()方法，其他线程都需要阻塞等待，直到活动线程执行< clinit>()方法完毕。如果在一个类的< clinit>()方法中有耗时很长的操作，就可能造成多个进程阻塞(需要注意的是，其他线程虽然会被阻塞，但如果执行< clinit>()方法后，其他线程唤醒之后不会再次进入< clinit>()方法。同一个加载器下，一个类型只会初始化一次。)，在实际应用中，这种阻塞往往是很隐蔽的。

故而，可以看出INSTANCE在创建过程中是线程安全的，所以说静态内部类形式的单例可保证线程安全，也能保证单例的唯一性，同时也延迟了单例的实例化。

那么，是不是可以说静态内部类单例就是最完美的单例模式了呢？其实不然，静态内部类也有着一个致命的缺点，就是传参的问题，由于是静态内部类的形式去创建单例的，故外部无法传递参数进去，例如Context这种参数，所以，我们创建单例时，可以在静态内部类与DCL模式里自己斟酌。



> 枚举版

```java
public enum SingleTon{



  INSTANCE;



        public void method(){



        //TODO



     }



}
```

枚举在java中与普通类一样，都能拥有字段与方法，而且枚举实例创建是线程安全的，在任何情况下，它都是一个单例。我们可直接以

```java
SingleTon.INSTANCE
```


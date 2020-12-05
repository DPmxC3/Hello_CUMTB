# 类加载器

​	类加载就是Java虚拟机通过一个类的全限定名来获取描述该类的二进制字节流。并创建一个对应的`java.lang.Class`对象，存储在自己的运行时方法区内存空间中，然后，**这个类的数据便通过这个Class对象来访问，包括其类，方法等。**

​	同时要比较两个类是否"相等"，必须要在同一个类加载器下比较才有意义，不然，就算是同一个字节码文件。如果是不同的类加载器加载，那么两个类必定不相等。



类加载器按其实现可以分为两类：引导类加载器（Bootstrap Class Loader），用户类加载器（User-defined Class Loader）

- 引导类加载器：加载`$JAVA_HOME/jre/lib/`下核心类库，如rt.jar，hotspot jvm中由C++实现
- 用户类加载器：所有用户类加载器都继承了`java.lang.ClassLoader`抽象类，sun提供了两个用户类加载器，我们也可以定义自己的类加载器
  - 扩展类加载器（ExtClassLoader）：`sun.misc.Launcher$ExtClassLoader`，负责加载`$JAVA_HOME/jre/lib/ext`下的一些扩展类
  
  - 应用类加载器（AppClassLoader）：可由`ClassLoader.getSystemClassLoader()`方法获得，也称系统类加载器，负责加载用户（classpath中）定义的类。
  
  - 自定义类加载器（Custom ClassLoader）：用户也可以定义自己的类加载器，实现一些定制的功能
  
 ![image-20201205161201900](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201205161204.png)
 
关于类加载器有以下几点需要注意一下:

1. 用户类加载器都实现了`java.lang.ClassLoader`抽象类，该类有一个叫做`private final ClassLoader parent`的父加载器。是用来保存父类加载器的。![image-20201124002130698](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012455.png)

2. 引导类加载器只加载jre lib目录(或-Xbootclasspath指定)下的类，并且只识别特定文件名如`rt.jar`,所以不会加载到用户类
3. 对于数组，并不存在数组类型的字节码表示形式，由jvm负责创建，一般在碰到`newarray`指令进行初始化时，如果数组的元素类型不是基本类型(如 int[])，而是引用类型，则会先加载本身类型，这可能由引导类加载器或者用户类加载器加载，具体看是什么引用类型。
4. jvm会缓存已经加载过的类，并设置加载相应类的加载器。具体实现在源码中是这个![image-20201124002555610](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012458.png)



自定义一个类加载器通过实例来体验一下：

```java
package zou;
import java.io.InputStream;

class myClassLoader extends ClassLoader {
    @Override
    public Class<?> loadClass(String name) throws ClassNotFoundException {
        try {
            String fileName = name.substring(name.lastIndexOf(".") + 1) + ".class";
            InputStream is = getClass().getResourceAsStream(fileName);
            if (is == null) {
                return super.loadClass(name);
            }
            byte[] bytes = new byte[is.available()];
            is.read(bytes);
            return defineClass(name, bytes, 0, bytes.length);
        } catch (Exception e) {
            throw new ClassNotFoundException(name);
        }
    }
}
public class loadclass {
    public static void main(String[] args) throws Exception{
        myClassLoader loader = new myClassLoader();
        Class obj = loader.loadClass("zou.loadclass");
        Object zz = obj.newInstance();

        System.out.println(zz.getClass());
        System.out.println(zz instanceof zou.loadclass);
    }
}
/*
输出:
class zou.loadclass
false
*/
```

由此可见，输出是false，虽然是同一个类，但是由于不同的类加载器不同，所以Class实例并不是同一个



# 双亲委派机制

​	所谓的双亲委派模式是指一个类加载器在加载某个类时，首先委派给父类加载，父加载器又委派给它的父类加载器加载，如此顶层的引导类加载器为止，如果父类加载器没有找到相应类则会自己尝试加载。

​	从双亲委派模式可以看出，他要求每个类加载器都有一个父加载器，如果某个类加载器的父类加载器为NULL，则搜索引导类加载器是否加载过它要加载的类。

​	

​	让我们看看源码中的ClassLoader怎么实现双亲委派机制

![image-20201124003340326](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012502.png)



**emmmm 分析就看看我对写的注释吧~~**

```java
protected Class<?> loadClass(String name, boolean resolve)
        throws ClassNotFoundException
    {   // 一个类的加载是放在代码同步块中的，所以不会出现一个类加载多次的情况
        synchronized (getClassLoadingLock(name)) { 
            // First, check if the class has already been loaded
            // 先检查这个类是否被加载过
            Class<?> c = findLoadedClass(name); // 查看双亲委派模型缓存
            // 如果类没有在缓存中找到，则被双亲委派模型加载
            if (c == null) {
                long t0 = System.nanoTime();
                try {
                    if (parent != null) {
                        // 如果父类加载器不为null，就代理给父类加载器加载。emmm这个父类加载器后面说~~~
                        // 父类加载器如果在自己的搜索范围找不到该类
                        // 就会抛出ClassNotFoundExecption的异常，这点可以在源码中简单的看出来。
                        
                        c = parent.loadClass(name, false);
                    } else {
                        // 如果父类加载器为null，则从引导类加载器加载过的类中
                        // 找是否加载过此类，找不到返回null
                        c = findBootstrapClassOrNull(name);
                    }
                } catch (ClassNotFoundException e) {
                    // ClassNotFoundException thrown if class not found
                    // from the non-null parent class loader
                    // 存在父加载器但父加载器没有找到要加载的类触发此异常
                    // 只捕获不处理，交给字加载器自身去加载
                }

                if (c == null) {
                    // If still not found, then invoke findClass in order
                    // to find the class.
                    // 如果父类加载器，到顶层加载器都找不到这个类，则自己来加载
                    long t1 = System.nanoTime();
                    c = findClass(name);

                    // this is the defining class loader; record the stats
                    sun.misc.PerfCounter.getParentDelegationTime().addTime(t1 - t0);
                    sun.misc.PerfCounter.getFindClassTime().addElapsedTimeFrom(t1);
                    sun.misc.PerfCounter.getFindClasses().increment();
                }
            }
            if (resolve) {
                resolveClass(c);
            }
            return c;
        }
    }
```



知道了双亲委派的基本过程。那就开始一个函数一个函数的看啦。。 这过程可能有点长~~

* getClassLoadingLock

  ​			这个方法很简单~~ 就是如果parallelLockMap不为空，就是具有并行能力。先创建一个Object对象，调用`parallelLockMap`的`putIfAbsent(className, newlock);`方法，这方法的作用就是，如果这个关联过值了就直接返回，如果没有就把我们之前写的Object当value的值。

![image-20201124004510879](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012507.png)

这个parallelLockMap呢可以从私有构造方法看到，就是一个`ConcurrentHashMap<String, Object>()`：

![image-20201124004859129](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012510.png)

从私有构造函数可以看到，ClassLoader是否被加载2中情况ParallelLoaders.isRegistered(this.getClass())**来判断是否可以并行。**从源码可以看到，如果当前classloader注册为可并行加载的，则为每一个类名维护一个锁对象供`synchronized`使用，可并行加载不同类，否则以当前classloader作为锁对象，只能串行加载。

那这个是怎么判断的呢。我们又可以愉快的点进源码看看

![image-20201124010146341](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012513.png)

```java
static boolean isRegistered(Class<? extends ClassLoader> c) {
            synchronized (loaderTypes) {
                return loaderTypes.contains(c);
            }
        }
```

这个逻辑很简单了，如果loaderTypes中有c这个就返回true；那我们再来看看这个loaderTypes是什么~~



在仔细看看 原来这里是一个静态内部类！

![image-20201124011009640](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012516.png)

我们要的loaderTypes就可以在之中找到。 是一个`WeakHashMap`好吧 这我没看。。。 不过这里面有个静态代码块。就给loaderTypes添加ClassLoader。**这是什么原理呢 真的没有搞明白，有什么必要性吗？希望大佬给指出**

```java
private static final Set<Class<? extends ClassLoader>> loaderTypes =
            Collections.newSetFromMap(
                new WeakHashMap<Class<? extends ClassLoader>, Boolean>());
        static {
            synchronized (loaderTypes) { loaderTypes.add(ClassLoader.class); } // 这里注册了父类，留个悬念。
        }
```

怎么可以注册进去呢。原来答案就在源码中，原来需要自己手动注册进去。

![image-20201124024857239](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012519.png)

让我们再来看看源码里面注册的方法：

![image-20201125003442352](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012523.png)

这里就可以解释为什么开始要把父类ClassLoader注册进去，因为必须是要父类先注册了，子类才可以注册。





* findBootstrapClassOrNull()

![image-20201124022137759](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012526.png)

![image-20201124022401824](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012527.png)

这是jvm的原生实现，查找Boot ClassLoader已加载的类，没有返回null





* findClass()

![image-20201124022509088](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012529.png)

`findClass`交给子类加载器实现，我们一般重写该方法来实现自己的类加载器，这样实现的类加载器也符合双亲委派模式。当然，双亲委派模式都是在`loadClass`实现的，可以自己重写`loadClass`来打破双亲委派模式。



#### 还是原来的代码 我们只是需要改掉loadClass变为findClass 我们就可以了。这样得出来的就是一个类加载器加载的了！！！

```java
package zou;
import java.io.InputStream;

class myClassLoader extends ClassLoader {
    @Override
    public Class<?> findClass(String name) throws ClassNotFoundException {
        try {
            String fileName = name.substring(name.lastIndexOf(".") + 1) + ".class";
            InputStream is = getClass().getResourceAsStream(fileName);
            if (is == null) {
                return super.loadClass(name);
            }
            byte[] bytes = new byte[is.available()];
            is.read(bytes);
            return defineClass(name, bytes, 0, bytes.length);
        } catch (Exception e) {
            throw new ClassNotFoundException(name);
        }
    }
}
public class loadclass {
    public static void main(String[] args) throws Exception{
        myClassLoader loader = new myClassLoader();
        Class obj = loader.loadClass("zou.loadclass");
        Object zz = obj.newInstance();
        System.out.println(zz.getClass());
        System.out.println(zz instanceof zou.loadclass);
    }
}
```

最后的结果：

![image-20201124023019085](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201125012532.png)





* defineClass();
   defineClass方法 的主要作用是将byte 字节流解析成JVM能够识别的class对象，这个方法意味着 我们不仅仅可以通class文件去实例化对象，还可以其他方式实例化对象，例如我们通过网络接收到一个类的字节码；defineClass的代码如下：

```java
 protected final Class<?> defineClass(String name, byte[] b, int off, int len,
                                         ProtectionDomain protectionDomain)
        throws ClassFormatError
    {
        protectionDomain = preDefineClass(name, protectionDomain);
        String source = defineClassSourceLocation(protectionDomain);
        Class<?> c = defineClass1(name, b, off, len, protectionDomain, source);
        postDefineClass(c, protectionDomain);
        return c;
    }


    protected final Class<?> defineClass(String name, java.nio.ByteBuffer b,
                                         ProtectionDomain protectionDomain)
        throws ClassFormatError
    {
        int len = b.remaining();

        // Use byte[] if not a direct ByteBufer:
        if (!b.isDirect()) {
            if (b.hasArray()) {
                return defineClass(name, b.array(),
                                   b.position() + b.arrayOffset(), len,
                                   protectionDomain);
            } else {
                // no array, or read-only array
                byte[] tb = new byte[len];
                b.get(tb);  // get bytes out of byte buffer.
                return defineClass(name, tb, 0, len, protectionDomain);
            }
        }

        protectionDomain = preDefineClass(name, protectionDomain);
        String source = defineClassSourceLocation(protectionDomain);
        Class<?> c = defineClass2(name, b, b.position(), len, protectionDomain, source);
        postDefineClass(c, protectionDomain);
        return c;
    }
```





* resolveClass();

  连接指定的一个类，如果你想在类被加载到JVM中的时候就被链接(Link)，则调用resolveClass()方法。通过源码发现，这是个用native方法实现。

这里就是最基本的类加载过程了。以后会慢慢更新`其他用户类加载器`的源码

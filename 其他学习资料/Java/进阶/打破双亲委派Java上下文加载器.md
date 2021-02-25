# 线程上下文加载器

双亲委派很好地解决了各个类加载器协作时基础类型的一致性问题（越基础的类由越上层的加载器进行加载），基础类型之所以被称为“基础”，**是因为它们总是作为被用户代码继承、调用的API存在**，但程序设计往往没有绝对不变的完美规则，如果有基础类型又要调用回用户的代码，那该怎么办呢？

这并非是不可能出现的事情，一个典型的例子便是JNDI(JNDI 在 J2EE 应用程序中的主要角色就是提供间接层，这样组件就可以发现所需要的资源,详细可以[看这里](https://www.cnblogs.com/study-everyday/p/6723313.html))服务，JNDI现在已经是Java的标准服务， 它的代码由**启动类加载器来完成加载（在JDK 1.3时加入到rt.jar的）**，肯定属于Java中很基础的类型 了。但**JNDI存在的目的就是对资源进行查找和集中管理，它需要调用由其他厂商实现并部署在应用程序的ClassPath下的JNDI服务提供者接口（Service Provider Interface，SPI）的代码**，现在问题来了，启动类加载器是绝不可能认识、加载这些代码的，那该怎么办？

为了解决这个困境，Java的设计团队只好引入了一个不太优雅的设计：线程上下文类加载器 （Thread Context ClassLoader）。这个类加载器可以通过java.lang.Thread类的setContextClassLoader()方法进行设置，如果创建线程时还未设置，它将会从父线程中继承一个，如果在应用程序的全局范围内都没有设置过的话，那这个类加载器默认就是应用程序类加载器。

有了线程上下文类加载器,JNDI服务使用这个线程上下文类 加载器去加载所需的SPI服务代码，这是一种父类加载器去请求子类加载器完成类加载的行为，这种行 为实际上是打通了双亲委派模型的层次结构来逆向使用类加载器，已经违背了双亲委派模型的一般性 原则，但也是无可奈何的事情。不过，当SPI的服务提供者多于一个的时候，**代码就只能根据具体提供者的类型来硬编码判断**，为了消除这种极不优雅的实现方式，在JDK 6时，JDK提供了 java.util.ServiceLoader类，以META-INF/services中的配置信息，辅以责任链模式，这才算是给SPI的加 载提供了一种相对合理的解决方案。

线程上下文加载器是从JDK1.2开始引入的。类java.lang.Thread中的方法`getContextClassLoader()`和`setContextClassLoader(ClassLoader cl)`用来获取和设置线程的上下文类加载器。

如果没有通过`setContextClassLoader(ClassLoader cl)`方法进行设置的话，线程将继承其父线程的上下文类加载器，默认为系统类加载器，这点可以从下面的JDK中的源码得到验证。

以下代码摘自`sun.misc.Launch`：

```java
public class Launcher {
    private static Launcher launcher = new Launcher();
    public Launcher() {
        Launcher.ExtClassLoader var1;
        try {
            var1 = Launcher.ExtClassLoader.getExtClassLoader();
        } catch (IOException var10) {
            throw new InternalError("Could not create extension class loader", var10);
        }

        try {
            this.loader = Launcher.AppClassLoader.getAppClassLoader(var1);
        } catch (IOException var9) {
            throw new InternalError("Could not create application class loader", var9);
        }
		// 上下文线程
        Thread.currentThread().setContextClassLoader(this.loader);
       	// ....................security代码
    }
}
```



# SPI

Service Provider Interface：服务提供者接口，简称SPI，是Java提供的一套用来被第三方实现或者扩展的API。常见的SPI有JDBC，JNDI，JAXP等，**这些SPI接口由Java核心库实现，而这些SPI的具体实现由第三方jar包实现**

下面看一段经典的JDBC获取连接池的代码：

```java
// Class.forName("com.mysql.jdbc.Driver");
DriverManager.getConnection("jdbc:mysql://localhost:3306/login", "root", "root");
```

我们可以将Class.forName这一行注释掉，但依然可以正常运行，这是为什么呢？我们通过源码一步一步分析。

先看DriverManager这个类，调用静态方法`getConnection()`，就会导致该类的初始化，也就是会执行这个类的静态代码块，DriverManager的静态代码块如下：

```java
    /**
     * Load the initial JDBC drivers by checking the System property
     * jdbc.properties and then use the {@code ServiceLoader} mechanism
     */
    static {
        loadInitialDrivers();
        println("JDBC DriverManager initialized");
    }
```

这里的注释说的很清楚了，翻译过来就是：通过检查系统属性JDBC来加载初始的JDBC驱动程序。属性，然后使用ServiceLoader机制，再来看看这个`loadInitialDrivers()`方法：

```java
private static void loadInitialDrivers() {
    	// .............读取配置
        AccessController.doPrivileged(new PrivilegedAction<Void>() {
            public Void run() {
                // 读取META-INF/services
                ServiceLoader<Driver> loadedDrivers = ServiceLoader.load(Driver.class);
                Iterator<Driver> driversIterator = loadedDrivers.iterator();
                try{
                    while(driversIterator.hasNext()) {
                        driversIterator.next();
                    }
                } catch(Throwable t) {
                }
                return null;
            }
        });
    	// .................
    }
```

在来看看`ServiceLoader.load`方法：

```java
public static <S> ServiceLoader<S> load(Class<S> service) {
    ClassLoader cl = Thread.currentThread().getContextClassLoader();
    return ServiceLoader.load(service, cl);
}
```

从load方法中可以看出取出了线程上下文加载器也就是系统类加载器传递给了后面的代码。

一路跟踪下去会发现系统类加载这个参数传递给了里面的一个内部类`LazyIterator`:

```java
private class LazyIterator
    implements Iterator<S>
{

    Class<S> service;
    ClassLoader loader;
    Enumeration<URL> configs = null;
    Iterator<String> pending = null;
    String nextName = null;

    private LazyIterator(Class<S> service, ClassLoader loader) {
        this.service = service;
        this.loader = loader;
    }
}
```

`LazyIterator`实现了Iterator接口，后面`loadInitialDrivers`中获取这个迭代器进行遍历，最终会调用下面的两个方法：

```java
private static final String PREFIX = "META-INF/services/";
private boolean hasNextService() {
            if (nextName != null) {
                return true;
            }
            if (configs == null) {
                try {
                    // fullName为：META-INF/services/java.sql.Driver
                    String fullName = PREFIX + service.getName();
                    if (loader == null)
                        configs = ClassLoader.getSystemResources(fullName);
                    else
                        configs = loader.getResources(fullName);
                } catch (IOException x) {
                    fail(service, "Error locating configuration files", x);
                }
            }
            while ((pending == null) || !pending.hasNext()) {
                if (!configs.hasMoreElements()) {
                    return false;
                }
                // 解析META-INF/services/java.sql.Driver文件
                pending = parse(service, configs.nextElement());
            }
            nextName = pending.next();
            return true;
        }

        private S nextService() {
            if (!hasNextService())
                throw new NoSuchElementException();
            String cn = nextName;
            nextName = null;
            Class<?> c = null;
            try {
                // 使用上下文加载器加载META-INF/services/java.sql.Driver中指定的驱动类
                c = Class.forName(cn, false, loader);
            } catch (ClassNotFoundException x) {
                fail(service,
                     "Provider " + cn + " not found");
            }
            if (!service.isAssignableFrom(c)) {
                fail(service,
                     "Provider " + cn  + " not a subtype");
            }
            try {
                S p = service.cast(c.newInstance());
                providers.put(cn, p);
                return p;
            } catch (Throwable x) {
                fail(service,
                     "Provider " + cn + " could not be instantiated",
                     x);
            }
            throw new Error();          // This cannot happen
        }
```

来看一下mysql-connector-java.jar下的META-INF/services/java.sql.Driver中的内容：

```properties
com.mysql.cj.jdbc.Driver
```

最后看一下`com.mysql.jdbc.Driver`的静态代码块:

```java
    static {
        try {
            java.sql.DriverManager.registerDriver(new Driver());
        } catch (SQLException E) {
            throw new RuntimeException("Can't register driver!");
        }
    }
```

`registerDriver`方法将driver实例注册到JDK的`java.sql.DriverManager`类中，其实就是add到它的一个类型为`CopyOnWriteArrayList`,名为`registeredDrivers`的静态属性中，**到此驱动注册基本完成。**

# 总结

如果我们使用JDBC时没有主动使用`Class.forName()`加载mysql的驱动时，那么JDBC会使用SPI机制取查找所有的jar下面的`META-INF/services/java.sql.Driver`文件,使用Class.forName反射加载其中指定的驱动类。

DriverManager类和ServiceLoader类都是输入rt.jar的，它们的类加载器时根类加载器。而具体的数据库驱动，却属于业务代码，这个根类加载器时无法加载的。而线程上下文类加载器破坏了"双亲委派机制"，可以在执行线程中抛弃双亲委派加载链模式，使程序可以逆向使用类加载器。



## 使用上下文类加载器校验实例归属

下面再来看一下java.sql.DriverManager.getConnection()这个方法，这里面有个小细节：

```java
    @CallerSensitive
    public static Connection getConnection(String url,
        String user, String password) throws SQLException {
        java.util.Properties info = new java.util.Properties();

        if (user != null) {
            info.put("user", user);
        }
        if (password != null) {
            info.put("password", password);
        }

        return (getConnection(url, info, Reflection.getCallerClass()));
    }

private static Connection getConnection(
        String url, java.util.Properties info, Class<?> caller) throws SQLException {
        // callerCL是调用这个方法得类所对应得类加载器
        ClassLoader callerCL = caller != null ? caller.getClassLoader() : null;
        synchronized(DriverManager.class) {
            // synchronize loading of the correct classloader.
            if (callerCL == null) {
                callerCL = Thread.currentThread().getContextClassLoader();
            }
        }
        //.................
    	// 遍历注册到registeredDrivers里得Driver类
        for(DriverInfo aDriver : registeredDrivers) {
            // 使用上下文类加载器检查Driver类得有效性，重点在isDriverAllowed中！
            if(isDriverAllowed(aDriver.driver, callerCL)) {
                try {
                    println("    trying " + aDriver.driver.getClass().getName());
                    Connection con = aDriver.driver.connect(url, info);
                    if (con != null) {
                        // Success!
                        println("getConnection returning " + aDriver.driver.getClass().getName());
                        return (con);
                    }
                } catch (SQLException ex) {
                    if (reason == null) {
                        reason = ex;
                    }
                }

            } else {
                println("    skipping: " + aDriver.getClass().getName());
            }

        }

        // .........
    }




    private static boolean isDriverAllowed(Driver driver, ClassLoader classLoader) {
        boolean result = false;
        if(driver != null) {
            Class<?> aClass = null;
            try {
                // 传入得classLoader为调用getConnection的上下文加载器，使用这个类加载器
                // 使用这个类加载器再次加载驱动类
                aClass =  Class.forName(driver.getClass().getName(), true, classLoader);
            } catch (Exception ex) {
                result = false;
            }
			// 这里只有aClass和driver.getClass()是由同一个类加载器加载才会相等。
             result = ( aClass == driver.getClass() ) ? true : false;
        }

        return result;
    }
```

`isDriverAllowed`这个方法的意义：例如在tomcat中，多个webapp都有自己的Classloader，如果它们都自带mysql-connect.jar包，那底层Classloader的DriverManager里将注册多个不同类加载器加载的Driver实例，webapp想要从DriverManager中获得连接，只有通过线程上下文加载器区分了。


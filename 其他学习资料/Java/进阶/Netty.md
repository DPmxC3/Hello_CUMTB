# 1. 什么是Netty

Netty是由[JBOSS](https://baike.baidu.com/item/JBOSS)提供的一个[java开源](https://baike.baidu.com/item/java开源/10795649)框架，现为 Github上的独立项目。Netty提供异步的、[事件驱动](https://baike.baidu.com/item/事件驱动/9597519)的网络应用程序框架和工具，用以快速开发高性能、高可靠性的[网络服务器](https://baike.baidu.com/item/网络服务器/99096)和客户端程序。

也就是说，Netty 是一个基于NIO的客户、服务器端的编程框架，使用Netty 可以确保你快速和简单的开发出一个网络应用，例如实现了某种协议的客户、服务端应用。Netty相当于简化和流线化了网络应用的编程开发过程，例如：基于TCP和UDP的socket服务开发。

## 为什么不用NIO

说不用NIO那肯定就是说说NIO的缺点了。

1. NIO的类库和API繁杂，学习成本高，需要熟练掌握`Selector`，`ServerSocketChannel`，`SocketChannel`，`ByteBuffer`等。
2. 需要熟悉Java多线成编程。这时因为NIO编程涉及到Reactor模式，必须要对多线程和网络编程非常熟悉，才能写出高质量的NIO程序。
3. [臭名昭著的epoll bug](https://cloud.tencent.com/developer/article/1543810)。会导致Selector空轮询，最终导致CPU100%。直到JDK1.7版本依然没有得到根本性解决

## 为什么用Netty

相对的，Netty优点很多：

1. API使用简单，学习成本低。
2. 功能强大，内置多种解码编码器，支持多种协议。
3. 性能高，对比其他主流的NIO框架，Netty的性能最优。
4. 社区活跃
5. Dubbo，Elasticsearch，Redis都采用了Netty，质量得到验证。



## Netty架构

我们可以去[官网](https://netty.io/)看一看。

进入页面我们会看见这样一张图, 这也是Netty的架构图：

![components](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208195924.png)

我们从上到下开始分析：

> 绿色的部分Core核心模块，包括零拷贝，API库，可扩展的事件模型。

> 橙色部分Protocol Support 协议支持，包括Http协议，webSocket，SSL，谷歌Protobuf协议，zlib/gzip压缩与解压缩，Large File Transfer大文件传输等等。

> 红色部分Transport Services传输服务，包括Socket，Datagram，Http Tunnel等等

更多好处可以去[官网](https://netty.io/index.html)看看，这里就不赘述

![image-20210206154940181](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208195928.png)



# 2. Hello Netty

## 搭建一个HelloWord工程



**引入maven**

这里直接用[最新版](https://mvnrepository.com/artifact/io.netty/netty-all/4.1.58.Final)：

```xml
<!-- https://mvnrepository.com/artifact/io.netty/netty-all -->
<dependency>
    <groupId>io.netty</groupId>
    <artifactId>netty-all</artifactId>
    <version>4.1.58.Final</version>
</dependency>
```

**服务端启动类：**

```java
package com.zou.server;

import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelOption;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;

/**
 * 主服务器
 */
public class MyServer {
    public static void main(String[] args) {
        // 创建连个线程组，parentGroup, childGroup，用于事件监听
        EventLoopGroup parentGroup = new NioEventLoopGroup();
        EventLoopGroup childGroup = new NioEventLoopGroup();

        // 创建服务端的启动对象
        ServerBootstrap bootstrap = new ServerBootstrap();

        // 设置两个线程组bossGroup和workerGroup
        bootstrap.group(parentGroup, childGroup)
                // 设置服务端通道实现类型
                .channel(NioServerSocketChannel.class)
                // 设置线程队列得到连接个数
                .option(ChannelOption.SO_BACKLOG, 128)
                // 设置保持活动连接状态
                .childOption(ChannelOption.SO_KEEPALIVE, true)

                // 使用匿名内部类的形式初始化通道对象
                .childHandler(new ChannelInitializer<SocketChannel>() {
                    @Override
                    protected void initChannel(SocketChannel socketChannel) throws Exception {
                        // 给pipeline管道设置处理器
                        socketChannel.pipeline().addLast(new MyServerHandler());
                    }
                });

        System.out.println("服务端准备就绪");

        try {
            // 绑定端口号， 启动服务端
            ChannelFuture channelFuture = bootstrap.bind(9999).sync();
            // 对关闭通道进行监听
            channelFuture.channel().closeFuture().sync();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            parentGroup.shutdownGracefully();
            childGroup.shutdownGracefully();
        }

    }
}
```



**服务端事件处理：**

```java
package com.zou.server;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.util.CharsetUtil;

import java.nio.ByteBuffer;


/**
 * 事件处理器
 */

public class MyServerHandler extends ChannelInboundHandlerAdapter {
    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        // 获取客户端发送过来的信息
        ByteBuf byteBuf = (ByteBuf) msg;
        System.out.println("收到客户端" + ctx.channel().remoteAddress() + "发送的信息：" + byteBuf.toString(CharsetUtil.UTF_8));
    }

    @Override
    public void channelReadComplete(ChannelHandlerContext ctx) throws Exception {
        // 发送消息给客户端
        ctx.writeAndFlush(Unpooled.copiedBuffer("Hello Netty Client", CharsetUtil.UTF_8));
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
        // 发生异常，关闭通道
        ctx.close();
    }
}
```



**客户端启动类：**

```java
package com.zou.client;

import io.netty.bootstrap.Bootstrap;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;

public class MyClient {
    public static void main(String[] args) {
        NioEventLoopGroup eventExecutors = new NioEventLoopGroup();

        // 创建bootstrap对象
        Bootstrap bootstrap = new Bootstrap();

        // 设置线程组
        bootstrap.group(eventExecutors)
                // 设置客户端的通道实现类型
                .channel(NioSocketChannel.class)
                // 使用匿名内部类初始化通道
                .handler(new ChannelInitializer<SocketChannel>() {
                    @Override
                    protected void initChannel(SocketChannel socketChannel) throws Exception {
                        // 添加我们自己写的处理器
                        socketChannel.pipeline().addLast(new MyClientHandler());
                    }
                });
        System.out.println("客户端准备就绪");

        try {
            // 连接服务器
            ChannelFuture channelFuture = bootstrap.connect("localhost", 9999).sync();
            // 对通道进行监听
            channelFuture.channel().closeFuture().sync();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            eventExecutors.shutdownGracefully();
        }


    }
}
```



**客户端事件处理：**

```java
package com.zou.client;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.util.CharsetUtil;

public class MyClientHandler extends ChannelInboundHandlerAdapter {
    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        // 发送消息到服务器
        ctx.writeAndFlush(Unpooled.copiedBuffer("Hello Netty Server", CharsetUtil.UTF_8));
    }

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        // 接收服务器发送来的消息
        ByteBuf byteBuf = (ByteBuf) msg;
        System.out.println("收到服务端" + ctx.channel().remoteAddress() + "的信息: " + byteBuf.toString(CharsetUtil.UTF_8));
    }
}
```



## 测试

先启动`MyServer`在启动`MyClient`。最后是这个效果：

![image-20210206163157546](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208195942.png)

![image-20210206163207434](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208195944.png)

这样我们就成功完成了HelloWorld了啊！接下来我们就来探究HelloWorld的各个API和原理吧！



# 3. Netty的特性与重要组件

## taskQueue任务队列

如果Handler处理器有一些长时间的业务处理，可以交给`taskQueue异步处理。`如下：

```java
ctx.channel().eventLoop().execute(() -> {
            try {
                Thread.sleep(3000);
                System.out.println("长时间业务处理");
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        });
System.out.println("收到客户端" + ctx.channel().remoteAddress() + "发送的信息：" + byteBuf.toString(CharsetUtil.UTF_8));
```



## scheduleTaskQueue延时任务队列

延时任务队列和上面介绍的任务队列非常相似，只是多一个可延迟一定时间在执行的设置。代码：

```java
ctx.channel().eventLoop().schedule(new Runnable() {
    @Override
    public void run() {
        try {
            Thread.sleep(1000);
            System.out.println("长时间处理一个业务");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}, 5, TimeUnit.SECONDS); // 延迟的时间
```



## Future异步机制

在搭建HelloWorld工程的时候，我们看到这样一行代码：

```java
ChannelFuture channelFuture = bootstrap.connect("localhost", 9999).sync();
```

很多操作都返回这个`ChannelFuture`对象，究竟这个`ChannelFuture`对象是用什么来做的呢？

`ChannelFuture`提供操作完成时一种**异步通知**的方式。一般在`Socket`编程中，等待相应结果都时同步阻塞的，而Netty则不会造成阻塞，因为`ChannelFuture`时采取类似观察者模式的形式进行获取结果。

```java
channelFuture.addListener(new ChannelFutureListener() {
    // 使用匿名内部类，ChannelFutureListener接口
    // 重写operationComplete方法
    @Override
    public void operationComplete(ChannelFuture future) throws Exception {
        // 判断是否操作成功
        if (future.isSuccess()) {
            System.out.println("连接成功");
        } else {
            System.out.println("连接失败");
        }
    }
});
```

如果用官方话说是这样的：

![image-20210206220825760](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208195948.png)

简单翻译一下就是：Netty中的I/O操作都是异步的。调用就会立即返回，但这并不能保证在调用结束时已经完成。而是给你一个`ChannelFuture`实例进行查询I/O状态信息的查询。



##  * Bootstrap

Bootstrap和ServerBootStrap是Netty提供的一个创建客户端和服务端启动类的工厂类，使用这个工厂类非常便利地创建启动类，根据上面的例子，其实可以看出能大大地减少了开发的难度。这时类图：

![image-20210206221710076](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208195952.png)

可以看到都是继承于`AbstractBootstrap`抽象类，所以大致配置都相同。

使用Bootstrap创建启动器的步骤可以分为以下几步：

1. 设置EventLoopGroup线程组group(parentGroup, childGroup)
2. 设置channel通道类型
3. 设置option参数
4. 设置handler流水线
5. 进行端口绑定
6. 启动
7. 等待通道关闭
8. 优雅关闭`EventLoopGroup`

## group()

首先我们要知道，服务端要使用两个线程组：

* parentGroup用于监听客户端连接，专门负责与客户端创建连接，并把连接注册到workerGroup的Selector中。
* childGroup用于处理每一个连接发生的读写事件。

一般创建线程组直接使用new就完事了：

```java
EventLoopGroup parentGroup = new NioEventLoopGroup();
EventLoopGroup childGroup = new NioEventLoopGroup();
```

但既然是线程组，那线程默认是多少呢？深入源码：

```java
public NioEventLoopGroup(int nThreads, ThreadFactory threadFactory,
    final SelectorProvider selectorProvider, final SelectStrategyFactory selectStrategyFactory) {
    super(nThreads, threadFactory, selectorProvider, selectStrategyFactory, RejectedExecutionHandlers.reject());
}
```

这是默认`NioEventLoopGroup`要走的构造方法。还有很多构造方法，但是，最后都有个super，也就是`NioEventLoopGroup`要继承的类`MultithreadEventLoopGroup`。然我们再来看看这个的源码：

```java
private static final int DEFAULT_EVENT_LOOP_THREADS;

static {
    DEFAULT_EVENT_LOOP_THREADS = Math.max(1, SystemPropertyUtil.getInt(
            "io.netty.eventLoopThreads", NettyRuntime.availableProcessors() * 2));

    if (logger.isDebugEnabled()) {
        logger.debug("-Dio.netty.eventLoopThreads: {}", DEFAULT_EVENT_LOOP_THREADS);
    }
}

/**
 * @see MultithreadEventExecutorGroup#MultithreadEventExecutorGroup(int, Executor, Object...)
 */
protected MultithreadEventLoopGroup(int nThreads, Executor executor, Object... args) {
    super(nThreads == 0 ? DEFAULT_EVENT_LOOP_THREADS : nThreads, executor, args);
}
```

构造函数都有个常量`DEFAULT_EVENT_LOOP_THREADS`, 默认是可用线程 * 2。也可以通过传参进行改变。



## channel()

这个方法用于设置通道类型，当建立连接后，会根据这个设置创建对应的Channel实例。

![image-20210206224234944](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208195957.png)

![image-20210206224248275](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208200000.png)

通道类型有以下：

**NioSocketChannel：**异步非阻塞的客户端TCP Socket连接。

**NioServerSocketChannel：**异步非阻塞的服务端TCP Socket连接。

> 常用的就是这两个通道类型，因为都是异步非阻塞。所以是首选

**OioSocketChannel:** 同步阻塞的客户端TCP Socket连接。

**OioServerSocketChannel:** 同步阻塞的服务器端TCP Socket连接。

这里看到Idea中都有弃用标志，所以当了解了~~

![image-20210206224634155](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208200002.png)

## option()与childOption()

先说下这两个的区别。

* option() 设置的是服务端用于接收进来的连接，也就是parentGroup线程。
* childOption() 是提供给副官到接收到的信息，也就是childGroup线程。

我们再来看看参数设置有那些：

SocketChannel参数，也就是childOption()常用的参数：

>SO_RCVBUF	Socket参数，TCP数据接收缓冲区大小。
>
>TCP_NODELAY	TCP参数，立即发送数据，默认值为True;
>
>SO_KEEPALIVE	Socket参数，连接保活，默认值为false。启用该功能时，TCP会主动探测空闲连接的有效性

ServerSocketChannel参数，也就是option()常用参数：

> SO_BACKLOG 	Socket参数，服务端接收连接的队列长度，如果队列已满，客户端连接将被拒绝，默认值，Windows为200，其他为128。

篇幅限制可自行百度。



## 设置流水线(重点)

ChannelPipeline时Netty处理请求的责任链，ChannelHandler则是具体处理请求的处理器。实际上每一个channel都有一个处理器的流水线。

在Bootstrap中childHandler()方法需要初始化通道，实例化一个ChannelInitializer, 这时候需要重写initChannel()初始化通道的方法，装配流水线就是在这个地方进行。

处理器Handler主要分为两种：

> ChannelInboundHandlerAdapter(入站处理器)，ChannelOutboundHandler(出站处理器)

入站指的是数据从底层Java NIO Channel到Netty的Channel。

出站指的是通过Netty的Channel来操作底层的Java NIO Channel。

**ChannelInboundHandlerAdapter处理器常用的事件有**：

1. 注册事件 fireChannelRegistered。
2. 连接建立事件 fireChannelActive。
3. 读事件和读完成事件 fireChannelRead、fireChannelReadComplete。
4. 异常通知事件 fireExceptionCaught。
5. 用户自定义事件 fireUserEventTriggered。
6. Channel 可写状态变化事件 fireChannelWritabilityChanged。
7. 连接关闭事件 fireChannelInactive。

**ChannelOutboundHandler处理器常用的事件有**：

1. 端口绑定 bind。
2. 连接服务端 connect。
3. 写事件 write。
4. 刷新时间 flush。
5. 读事件 read。
6. 主动断开连接 disconnect。
7. 关闭 channel 事件 close。



## bind()

提供用于服务端或者客户端绑定服务器地址和端口号，默认是异步启动。如果加上sync()方法则是同步。



## 优雅的关闭EventLoopGroup

```java
parentGroup.shutdownGracefully();
childGroup.shutdownGracefully();
```

会关闭所有的child Channel。关闭之后，释放掉底层的资源。





Channel是什么？看下官方文档说明：

> A nexus to a network socket or a component which is capable of I/O operations such as read, write, connect, and bind

一种连接到网络套接字或能进行读，写，连接和绑定等I/O操作的组件。

channel为用户提供：

1. 通道当前的状态（例如它是打开？还是已连接？）
2. channel的配置参数（例如接收缓冲区的大小）
3. channel支持的IO操作（例如读、写、连接和绑定），以及处理与channel相关联的所有IO事件和请求的ChannelPipeline。

## 获取Channel的状态

```java
boolean isOpen(); //如果通道打开，则返回true
boolean isRegistered();//如果通道注册到EventLoop，则返回true
boolean isActive();//如果通道处于活动状态并且已连接，则返回true
boolean isWritable();//当且仅当I/O线程将立即执行请求的写入操作时，返回true。
```

以上就是获取channel的四种状态的方法。



## 获取Channel的配置参数

获取单条配置信息，使用getOption()，代码演示：

```java
ChannelConfig config = channel.config();
Map<ChannelOption<?>, Object> options = config.getOptions();
for (Map.Entry<ChannelOption<?>, Object> entry : options.entrySet()) {
 System.out.println(entry.getKey() + " : " + entry.getValue());
}
/**
SO_REUSEADDR : false
WRITE_BUFFER_LOW_WATER_MARK : 32768
WRITE_BUFFER_WATER_MARK : WriteBufferWaterMark(low: 32768, high: 65536)
SO_BACKLOG : 128
以下省略...
*/
```



## channel支持的IO操作

**写操作**，这里演示从客户端写消息发送到服务端：

```java
ctx.writeAndFlush(Unpooled.copiedBuffer("Hello Netty Server", CharsetUtil.UTF_8));
```



**连接操作**，代码演示：

```java
ChannelFuture connect = channelFuture.channel().connect(new InetSocketAddress("127.0.0.1", 6666));//一般使用启动器，这种方式不常用
```



**通过channel获取ChannenlPipeline，**并做相关的处理

```java
ChannelPipeline pipeline = ctx.channel().pipeline();
pipeline.addLast(new MyServerHandler());
```



## Selector

在NioEventLoop中，有一个成员变量selector，这时nio包的Selector。

Netty中的Selector也和NIO的Selector是一样的，就是用于事件监听，管理注册到Selector中的channel，实现多路复用器。



## PiPeline与ChannelPipeline

在前面介绍Channel时，我们直到可以在channel中装配ChannelHandler流水线处理器，那一个channel不可能只有一个channelHandler处理器，肯定有很多的，既然时很多channelHnadler在一个流水线工作，肯定时有顺序的。

于是pipeline就出现了，pipeline相当于处理器的容器。初始化channel时，把channelHnadler按顺序转在pipeline中，就可以实现按顺序执行channelHandler了。

![40](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208200011.png)

在一个Channel中，只有一个ChannelPipeline。该pipeline在Channel被创建的时候创建。ChannelPipeline包含一个ChannelHander形式的列表，且所有ChannelHnadler都会注册到ChannelPipeline中。



## ChannelHandlerContext

在Netty中，Handler处理器时我们定义的，上面讲过通过集成入站处理器或者出战处理器实现。这时如果我们想在Handler中获取pipeline对象，或者channel对象，怎么获取呢？

于是Netty设计了这个ChannelHandlerContext上下文对象，就可以拿到channel，pipeline等对象，就可以进行读写等操作。

![image-20210207211426585](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208200013.png)

通过类图，ChannelHandlerContext是一个接口，下面有三个实现类。

实际上ChannelHandlerContext在pipeline中是一个链表的形式。看一段源码就明白：

```java
//ChannelPipeline实现类DefaultChannelPipeline的构造器方法
protected DefaultChannelPipeline(Channel channel) {
 this.channel = ObjectUtil.checkNotNull(channel, "channel");
 succeededFuture = new SucceededChannelFuture(channel, null);
 voidPromise =  new VoidChannelPromise(channel, true);
 //设置头结点head，尾结点tail
 tail = new TailContext(this);
 head = new HeadContext(this);
 
 head.next = tail;
 tail.prev = head;
}
```

下面我们用张图来表示会更加清晰一点：

![0](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208200016.png)



## EventLoopGroup

我们先看一下EventLoopGroup的类图：

![image-20210207212155889](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210208200019.png)



其中包括了常用的实现类NioEventLoopGroup。OioEventLoopGroup在前面的例子中也有使用过。

从Netty的架构图中，我们直到服务器是需要两个线程组进行配合工作的，而这个线程组的接口就是EventLoopGroup。

每个EventLoopGroup里包括一个或多个EventLoop，每个EventLoop中维护一个Selector实例。

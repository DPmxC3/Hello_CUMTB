# 1. Reactor概览

## 什么是Reactor模式

reactor模式一般翻译成**反应器模式**， 也有人称为**分发者模式**。`是将客户端请求提交到一个或者多个服务处理程序的涉及模式`。工作原理是由**一个线程接收所有的请求**，然后**派发这些请求到相关的工作线程中**。



## 为什么使用Reactor模式

在Java中，没有NIO出现之前都是使用socket编程。**socket的接收请求时阻塞的**，需要处理完一个请求才能处理下一个请求，所以在**面对高并发请求时，性能就会很差。**

那使用多线程就好了啊。如图：

![640](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210207174001.png)

接收到一个请求，就创建一个线程处理，这样就不会阻塞了。实际上这样的确是可以在提升性能上起到作用。**但是当请求很多的时候，就会创建大量的线程，维护线程需要资源的消耗，线程之间的切换也需要消耗内存**。而且系统创建线程的数量也是很有限，所以高并发时，会直接把系统拖垮。

基于以上的问题，提出了Reactor模式。

基于Java，Doug Lea(Java并发包作者)提出了三种形式，**单Reactor单线程， 单Reactor多线程和多Reactor多线程**。



# 2. Reactor模式的演进过程

先说下三种角色：

> **Reactor**: 负责响应事件，将事件分发到绑定了对应事件的Handler，如果是连接事件，则分发到Acceptor。
>
> **Handler**: 事件处理器。负责执行对应事件对应的业务逻辑。
>
> **Acceptor**：绑定了connect事件，当客户端发起connect请求时，Reactor会将accept事件分发给Acceptor处理

## 单Reactor单线程

![646](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210207173720.png)

**工作流程**

> 只有一个`select`循环接收请求，客户端注册进来由`Reactor`接收注册事件，然后再由reactor分发(dispatch)出去，由下面的(Handler)出处理。

**通俗的讲**

一个餐厅里只有一个既是前台也是服务员的人，负责接待客人，也负责把客人点的菜下达给厨师。

**单Reactor单线程的特点**

单线程的问题实际上是很明显的。只要其中一个Handler方法阻塞了，就会导致所有的client的Handler都被阻塞，也会导致注册事件无法处理，无法接收新的请求。所以这种模式用的比较少，因为不能充分利用到多核的资源。

这种模式仅仅只能处理Handler比较快速完成的场景。



## 单Reactor多线程

![640 (3)](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210207173717.png)

**工作流程**

> 在多线程Reactor中，**注册接收事件都是由Reactor来做,** 其他的计算，编解码由一个线程池来做。从图中可以看出工作线程是多线程的，监听注册事件的Reactor还是单线程。

**通俗解释**

相当于餐厅里面有一个前台，多个服务员。前台只负责接待客人，服务员只负责服务客人。

**单Reactor多线程的特点**

对比**单线程Reactor模型**，多线程Reactosr模式在Handler读写处理时，交给工作线程池处理，不会导致Reactor无法执行，因为Reactor分发和Handler处理是分开的，能充分利用资源。从而提升应用的性能。

缺点：Reactor只在主线程中运行，承担所有事件的监听和相应，如果短时间的高并发场景下，依然会造成性能的瓶颈。

## 多Reactor多线程

 ![649](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210207173714.png)

**工作流程**

1. mainReactor负责监听客户端请求，专门处理新连接的建立，讲建立好的连接注册到subReactor。
2. subReactor讲分配的连接加入到队列进行监听，当有新的事件发生时，会调用连接相对应的Handler进行处理。

**通俗的讲**

相当于餐厅里面有多个前台和多个服务员，前台只负责接待客人，服务员只负责服务客人。

**多Reactor多线程的特点**

mainReactor主要是用来处理客户端请求连接建立的操作。subReactor主要做和建立起来的连接做数据交互和事件业务处理操作，每个subReactor一个线程来处理。

这样的模型使得每个模块更加专一，耦合度更低，能支持更高的并发量。许多框架也使用这种模式，比如Netty就采用了这种模式。



# 3. 在netty中的应用

Netty可谓是框架中的精品，精品中的极品。要用一张图或一段话概括不太可能，所以下面我们仅仅分析一下Netty框架的构架模型。下篇文章详细探讨Netty。

![60](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210207173708.png)



**这个架构实际上和多Reactor多线程模型很像**

1. BossGroup相当于mainReactor，负责建立连接并把连接注册到WorkGroup中。WrokGroup负责处理连接对应的读写事件。
2. BossGroup和WorkGroup是两个线程池，里面有多个NioEventGroup(实际上是线程)，默认BossGroup和WorkGroup里的线程数是cpu核数的两倍(源码中是如此)
3. 每一个NioEventGroup都是一个无线循环，负责监听相对应的事件。
4. Pipeline(通道)里面包含多个ChannelHnadler(业务处理)，按顺序执行。



先给Netty打个基础，下篇文章写Netty的学习笔记

参考文章：

https://www.cnblogs.com/doit8791/p/7461479.html

https://blog.csdn.net/yehongzhi1994/article/details/107016621


# 1. 定义

java.nio全称java non-blocking IO, 是指`JDK1.4`及以上版本里提供的新api为所有的原始类型(boolean类型除外)提供**缓存支持的数据容器**，使用它可以提 供**非阻塞式**的高伸缩性网络

# 2. 为什么使用NIO

上面说到，是在JDK1.4以上才提供NIO，那在之前就是**BIO(阻塞式IO)**，也就是我们常用的IO流。

在使用BIO的时候，我们的主线程会进入到阻塞状态，这就非常影响程序的性能，**不能充分利用机器资源。**但是这样就会有人提出疑问， 那使用多线成不久行了吗？但是在高并发情况下，会创建很多线程，线程会占用内存，线程之间的切换也会浪费资源开销。

而NIO**只有在连接/通道真正有读写事件**发生时(事件驱动)，才会进行读写，就大大减少系统的开销。不必为每一个连接都创建一个线程，也不必去维护多个线程

## NIO三大核心

| NIO的核心 | 对应的类或接口 | 应用          | 作用     |
| :-------- | :------------- | :------------ | :------- |
| 缓冲区    | Buffer         | 文件IO/网络IO | 存储数据 |
| 通道      | Channel        | 文件IO/网络IO | 运输     |
| 选择器    | Selector       | 网络IO        | 控制器   |



`Buffer`是一个内存块。在`NIO`中，所有的数据都是用`Buffer`处理，有读写两种模式。所以NIO和传统的IO的区别就体现在这里。传统IO是面向`Stream`流，`NIO`而是面向缓冲区(`Buffer`)。

同时，Buffer有7种类型：ByteBuffer, ShortBuffer, IntBuffer, CharBuffer, LongBuffer, FloatBuffer, DoubleBuffer

我们一般用的类型就是`ByteBuffer`，将数据转成字节进行处理。实质上是一个byte[]数组。看看源码：

```java
public abstract class ByteBuffer
    extends Buffer
    implements Comparable<ByteBuffer>
{
    final byte[] hb;                  // Non-null only for heap buffers

    // Creates a new buffer with the given mark, position, limit, capacity,
    // backing array, and array offset
    //
    ByteBuffer(int mark, int pos, int lim, int cap,   // package-private
                 byte[] hb, int offset)
    {
        super(mark, pos, lim, cap);
        this.hb = hb;
        this.offset = offset;
    }
}
```



# 3. 创建Buffer的方式

主要分为两种：JVM堆内内存块Buffer，堆外内存块Buffer。

分别创建**堆内内存块**(非直接缓冲区)和**堆外内存块**(直接缓冲区)的方法是：

```java
public class test1 {
    public static void main(String[] args) {
        // 创建堆内内存块
        ByteBuffer allocate = ByteBuffer.allocate(1024);

        String msg = "哈哈哈";
        // 包装一个byte[]数组获得一个Buffer，实际类型是HeapByteBuffer
        ByteBuffer wrap = ByteBuffer.wrap(msg.getBytes());

        // 创建堆外内存块DirectByteBuffer
        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(1024);
    }
}
```

## 创建堆内和堆外内存的区别

`HeapByteBufer`所创建的字节缓冲区就是在JVM堆中的，即JVM内部所维护的字节数组。

`DirectByteBuffer`是直接操作操作系统本地代码创建的内存缓冲数组

`DirectByteBuffer`的使用场景：

1. java程序与本地磁盘、socket传输数据
2. 大文件对象，可以使用。不会受到堆内存大小的限制。
3. 不需要频繁创建，生命周期较长的情况，能重复使用的情况。

`HeapByteBuffer`的使用场景：

除了以上的场景外，其他情况还是建议使用`HeapByteBuffer`，没有达到一定的量级，实际上使用`DirectByteBuffer`是体现不出优势的。

# 4. Buffer初体验

接下来，使用`ByteBuffer`做一个小例子：

```java
public class test2 {
    public static void main(String[] args) {
        String msg = "一顿操作猛如虎，一看战绩0杠5";
        ByteBuffer allocate = ByteBuffer.allocate(1024); 
        byte[] bytes = msg.getBytes();
        allocate.put(bytes); // 写入数据到Buffer中
        allocate.flip(); // 切换成读的模式
        byte[] tempByte = new byte[bytes.length]; // 用于存放数据的数组
        int i = 0;
        while (allocate.hasRemaining()) {
            byte b = allocate.get();
            tempByte[i] = b;
            i++;
        }
        System.out.println(new String(tempByte));
    }
}
```

这个`flip()`方法是很重要的。就是切换到读模式。上面已经提到**缓存区是双向的，既可以往缓冲区写入数据，也可以从缓冲区读取数据**。但是不能同时进行，需要切换。那么这个切换的本质是什么呢？

## 三个重要的参数

先看看源码，从上到下：

```java
public static ByteBuffer allocate(int capacity) {
    if (capacity < 0)
        throw new IllegalArgumentException();
    return new HeapByteBuffer(capacity, capacity);
}

HeapByteBuffer(int cap, int lim) {            // package-private
    super(-1, 0, lim, cap, new byte[cap], 0);
    /*
        hb = new byte[cap];
        offset = 0;
        */
}

ByteBuffer(int mark, int pos, int lim, int cap,   // package-private
           byte[] hb, int offset)
{
    super(mark, pos, lim, cap);
    this.hb = hb;
    this.offset = offset;
}

Buffer(int mark, int pos, int lim, int cap) {       // package-private
    if (cap < 0)
        throw new IllegalArgumentException("Negative capacity: " + cap);
    this.capacity = cap;
    limit(lim);
    position(pos);
    if (mark >= 0) {
        if (mark > pos)
            throw new IllegalArgumentException("mark > position: ("
                                               + mark + " > " + pos + ")");
        this.mark = mark;
    }
}

```

绕了一大圈终于来到这里了。下面看看这三个参数吧。默认的话是`limit`和`capacity`是相同的

```java
// 位置，默认是从第一个开始
private int position = 0;
// 限制，不能读取或者写入的位置索引
private int limit;
// 容量， 缓冲区所包含的元素数量
private int capacity;
```

那么我们以上面的例子，一句一句代码的分析：

```java
ByteBuffer allocate = ByteBuffer.allocate(1024);
```

当创建一个ByteBuffer时是这样的：

![image-20210204221104511](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210205001020.png)

当向里面put值得时候可以看到`position`的值已经上升了。

![image-20210204222616288](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210205001018.png)

重要的来了，这里可以看到，当执行了`flip()`函数，这里的`limit`和`position`发生了变化。切换到了只读模式

![image-20210204222704914](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210205001014.png)

让我们来看看`flip()`的源码：

```java
public final Buffer flip() {
    limit = position;
    position = 0;
    mark = -1;
    return this;
}
```

将`position`清空，将`limit`设置为原有的`position`, 这样就能转换为只读模式了

顺带看看`hasRemaining()`的源码吧：

```java
public final boolean hasRemaining() {
    return position < limit;
}
```

这就一目了然，就是通过position和limit的值来控制读写，但是这里值得注意的是`capacity`一直也没有发生变化



# 5. 管道

首先我们看一下`Channel`有那些子类:

![Channel](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210205001008.png)

常用的`Channel`有这四种：

> FileChannel，读写文件中的数据。
> ServerSockectChannel，监听新进来的TCP连接，像Web服务器那样。对每一个新进来的连接都会创建一个SocketChannel。
> SocketChannel，通过TCP读写网络中的数据。
> DatagramChannel，通过UDP读写网络中的数据。

**Channel本身并不存储数据，只是负责数据的运输**。必须要和`Buffer`一起使用。

## FileChannel

FileChannel的获取方式，下面举个文件复制拷贝的例子进行说明：

![image-20210204231421410](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210205001005.png)

先准备一个"1.txt"放在项目的根目录下，然后编写一个main方法:

```java
import java.io.*;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

public class test1 {
    public static void main(String[] args) throws Exception {
        // 获取文件输入流
        File file = new File("1.txt");
        FileInputStream fileInputStream = new FileInputStream(file);
        // 从文件输入流获取通道
        FileChannel inputChannel = fileInputStream.getChannel();

        // 获取文件输出流
        FileOutputStream fileOutputStream = new FileOutputStream(new File("2.txt"));
        // 从文件输出流获取通道
        FileChannel outputChannel = fileOutputStream.getChannel();

        // 创建一个byteBuffer，小文件所以就直接一次读取，不分多次循环了
        ByteBuffer allocate = ByteBuffer.allocate((int) file.length());

        // 读到缓冲区
        inputChannel.read(allocate);
        // 切换成读模式
        allocate.flip();
        // 把数据从缓冲区写入到输出流通道
        outputChannel.write(allocate);

        // 关闭通道和流
        inputChannel.close();
        outputChannel.close();
        fileOutputStream.close();
        fileInputStream.close();
    }
}
```

执行后我们就获得一个"2.txt"。执行成功

![image-20210204231559595](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210205001000.png)

上面的例子，可以用一张示意图表示，是这样的：

![640](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210205000958.png)



## SocketChannel

接下来学习获取`SocketChannel`的方式。

先来看看样例

```java
import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;

public class test2 {
    public static void main(String[] args) throws Exception {
        // 获取ServerSocketChannel，相当于服务端的服务通道
        ServerSocketChannel serverSocketChannel = ServerSocketChannel.open();
        InetSocketAddress address = new InetSocketAddress("127.0.0.1", 9999);

        // 绑定地址，端口号
        serverSocketChannel.bind(address);

        // 创建一个缓冲区
        ByteBuffer allocate = ByteBuffer.allocate(1024);
        while (true) {
            // 获取客户端的SocketChannel
            SocketChannel socketChannel = serverSocketChannel.accept();
            // 客户端读入内容
            while (socketChannel.read(allocate) != -1) {
                // 输出内容
                System.out.println(new String(allocate.array()));
                // 清空，实际就是将position和limit归位
                allocate.clear();
            }
        }
    }
}
```

为了测试main()方法，我们通过`telnet`命令来进行测试：

![image-20210204235046325](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210205000954.png)

然后再输入框中输入`zouyishan`就会有这个效果：

![image-20210204235134880](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210205000951.png)

通过上面的例子我们可以知道，通过`ServerSocketChannel.open()`方法可以获取服务器的通道，然后绑定一个地址端口号，接着`accept()`方法可获得一个`SocketChannel`通道，也就是客户端的连接通道。

最后配合使用`Buffer`进行读写即可。

这就是一个简单的例子，实际上上面的例子是阻塞式的。要做到非阻塞式还需要使用选择器。



# 6. 选择器

`Selector`翻译成选择器，有些人会翻译成**多路复用器**，实际上指的是同一样东西。

只有网络IO才会使用选择器，文件IO是不需要使用的。

选择器可以说是NIO的核心组件，它可以监听通道的状态，来实现异步非阻塞的IO。换句话说，也就是事件驱动。`以此实现单线程管理多个Channel的目的`



![641](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210205000944.png)

## 核心API

| API方法名       | 作用                                            |
| :-------------- | :---------------------------------------------- |
| Selector.open() | 打开一个选择器。                                |
| select()        | 选择一组键，其相应的通道已为 I/O 操作准备就绪。 |
| selectedKeys()  | 返回此选择器的已选择键集。                      |

以上API回在后序的文章中说到，这篇文章先将基本API做一个了解~~



# 7. NIO快速入门

## 通道间的数据传输

这里主要介绍两个通道与通道之间数据传输的方式：

`transferTo()`: 把源通道的数据传输到目的的通道中。

示例：

```java
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

public class test1 {
    public static void main(String[] args) throws Exception {
        // 获取输入通道
        File file = new File("1.txt");
        FileInputStream fileInputStream = new FileInputStream(file);
        FileChannel fileInputStreamChannelchannel = fileInputStream.getChannel();

        // 获取输出通道
        FileOutputStream fileOutputStream = new FileOutputStream(new File("2.txt"));
        FileChannel fileOutputStreamChannel = fileOutputStream.getChannel();

        // 获取缓冲
        ByteBuffer allocate = ByteBuffer.allocate((int) file.length());

        // 将输入流通道的数据读取到输出流的通道
        fileInputStreamChannelchannel.transferTo(0, allocate.limit(), fileOutputStreamChannel);
        // 关闭流
        fileInputStreamChannelchannel.close();
        fileOutputStreamChannel.close();
        fileInputStream.close();
        fileOutputStream.close();

    }
}
```

`transferFrom`将输入流通道的数据读取到输出流通道

```java
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

public class test1 {
    public static void main(String[] args) throws Exception {
        // 获取输入通道
        File file = new File("1.txt");
        FileInputStream fileInputStream = new FileInputStream(file);
        FileChannel fileInputStreamChannelchannel = fileInputStream.getChannel();

        // 获取输出通道
        FileOutputStream fileOutputStream = new FileOutputStream(new File("2.txt"));
        FileChannel fileOutputStreamChannel = fileOutputStream.getChannel();

        // 获取缓冲
        ByteBuffer allocate = ByteBuffer.allocate((int) file.length());

        // 将输入流通道的数据读取到输出流的通道
//        fileInputStreamChannelchannel.transferTo(0, allocate.limit(), fileOutputStreamChannel);
        fileOutputStreamChannel.transferFrom(fileInputStreamChannelchannel, 0, allocate.limit());
        // 关闭流
        fileInputStreamChannelchannel.close();
        fileOutputStreamChannel.close();
        fileInputStream.close();
        fileOutputStream.close();

    }
}
```



## 分散读取和聚合写入

点进去源码看看：

```java
public abstract class FileChannel
    extends AbstractInterruptibleChannel
    implements SeekableByteChannel, GatheringByteChannel, ScatteringByteChannel
```

这里可以看到有实现`GatheringByteChannel`, 和`ScatteringByteChannel`。继续点进去看：

发现分别就是两个方法，`write`和`read`。也就是分散读取和聚合写入的操作。

```java
public interface GatheringByteChannel
    extends WritableByteChannel
{
    public long write(ByteBuffer[] srcs, int offset, int length)
        throws IOException;
    public long write(ByteBuffer[] srcs) throws IOException;
}

public interface ScatteringByteChannel
    extends ReadableByteChannel
{
    public long read(ByteBuffer[] dsts, int offset, int length)
        throws IOException;
    public long read(ByteBuffer[] dsts) throws IOException;
}
```

怎么使用，请看如下：

首先首页文件夹下有一个"1.txt"文件， 内容为：`qwertyuiopasdfghjklzxcvbnm`。

代码如下：

```java
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.Arrays;

public class test2 {
    public static void main(String[] args) throws Exception {
        // 获取输入流通道
        File file = new File("1.txt");
        FileInputStream fileInputStream = new FileInputStream(file);
        FileChannel fileInputStreamChannel = fileInputStream.getChannel();

        // 获取输出流通道
        FileOutputStream fileOutputStream = new FileOutputStream(new File("2.txt"));
        FileChannel fileOutputStreamChannel = fileOutputStream.getChannel();

        // 得到缓冲数组
        ByteBuffer allocate1 = ByteBuffer.allocate(5);
        ByteBuffer allocate2 = ByteBuffer.allocate(5);
        ByteBuffer allocate3 = ByteBuffer.allocate(5);

        ByteBuffer[] buffers = new ByteBuffer[]{allocate1, allocate2, allocate3};
        long read;
        long sumRead = 0;

        while ((read = fileInputStreamChannel.read(buffers)) != -1) {
            sumRead += read;
            // 分散读
            Arrays.stream(buffers)
                    .map(buffer -> "position = " + buffer.position() + ", limit = " + buffer.limit())
                    .forEach(System.out::println);
            // 切换读模式
            Arrays.stream(buffers).forEach(Buffer::flip);

            // 聚合写入
            fileOutputStreamChannel.write(buffers);
            Arrays.stream(buffers).forEach(Buffer::clear);
        }

        System.out.println("总长度：" + sumRead);

        // 关闭通道
        fileInputStream.close();
        fileInputStreamChannel.close();
        fileOutputStream.close();
        fileOutputStreamChannel.close();
    }
}
```

输出结果为：

```java
position = 5, limit = 5
position = 5, limit = 5
position = 5, limit = 5
position = 5, limit = 5
position = 5, limit = 5
position = 1, limit = 5
总长度：26
```

可见循环了两次，第一次循环时，三个缓冲区读取5个字节，总共读取15，也就是读满了。还剩下11个字节，于是第二次循环时，前两个缓冲区分配了5字节，最后一个缓冲区给他分配了1个字节，刚好读完。总共就是26个字节。

这就是分散读取，聚合写入的过程。

使用场景就是可以**使用一个缓冲区数组，自动地根据需要去分配缓冲区的大小。==可以减少内存消耗。==**网络IO也可以使用，这里就不演示了。



## 非直接/直接缓冲区

非直接缓冲区的创建方式：

```java
public static ByteBuffer allocate(int capacity) {
    if (capacity < 0)
        throw new IllegalArgumentException();
    return new HeapByteBuffer(capacity, capacity);
}
```

直接缓冲区的创建方式：

```java
public static ByteBuffer allocateDirect(int capacity) {
    return new DirectByteBuffer(capacity);
}
```

非直接/直接缓冲区的区别示意图：

![640 (1)](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210206134358.png)

![642](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210206134355.png)

从示意图我们可以发现，最大的不同在于直接缓冲区不需要再把文件内容copy到物理内存中。这就大大地提高了性能。其实再介绍Buffer时，我们就有接触到这个概念。直接缓冲区时堆外内存，再本地文件IO效率会更高一点。

# 8. 网络IO

其实NIO的主要用途是网络IO，再NIO之前Java要使用网络编程就只有`Socket`。而`Socket`是阻塞的，显然对于高并发的场景是不适用的。所以NIO的出现就是解决这个痛点。

主要思想是`把Channel通道注册到Selector中，通过Selecotr区监听Channel中的事件状态`，这样就不需要阻塞等待客户端的连接，从主动等待客户端的连接，变成 了通过事件驱动。没有监听的事件，服务器可以做自己的事情。

## 使用Selector的例子

接下来写一个服务端接收客户端的小例子：

服务端：

```java
package Nio.Selector;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.util.Iterator;
import java.util.Set;

public class NIOServer {
    public static void main(String[] args) throws Exception {
        // 打开ServerSocketChannel，绑定本机端口9999，然后设置为非阻塞
        ServerSocketChannel serverSocketChannel = ServerSocketChannel.open();
        InetSocketAddress inetSocketAddress = new InetSocketAddress("127.0.0.1", 9999);
        serverSocketChannel.bind(inetSocketAddress);
        serverSocketChannel.configureBlocking(false);

        // 打开选择器
        Selector selector = Selector.open();

        // serverSocketChannel注册到选择器中，监听连接事件
        serverSocketChannel.register(selector, SelectionKey.OP_ACCEPT);

        // 循环等待客户端连接
        while (true) {
            if (selector.select(3000) == 0) {
                System.out.println("服务器等待3秒，没有连接");
                continue;
            }
            // 如果有事件 selector.select(3000) > 0 的情况，获取事件
            // 这里的事件我理解的是通过监听服务端，可以监听到很多连接，也就有了很多不同Client的事件
            Set<SelectionKey> selectionKeys = selector.selectedKeys();

            // 获取迭代器遍历
            Iterator<SelectionKey> iterator = selectionKeys.iterator();

            while (iterator.hasNext()) {
                // 获取事件，也就是selectionKey
                SelectionKey selectionKey = iterator.next();

                // 连接事件
                if (selectionKey.isAcceptable()) {
                    // 获取客户端通道，这时客户端和服务端已经建立好了连接
                    SocketChannel accept = serverSocketChannel.accept();
                    System.out.println("连接成功");

                    // 设置为非阻塞
                    accept.configureBlocking(false);

                    // 把socketChannel注册到selector中，监听读事件， 并绑定一个缓冲区
                    accept.register(selector, SelectionKey.OP_READ, ByteBuffer.allocate(1024));
                }

                // 读事件
                if (selectionKey.isReadable()) {
                    // 获取客户端通道
                    SocketChannel socketChannel = (SocketChannel)selectionKey.channel();
                    // 获取关联的ByteBuffer
                    ByteBuffer buffer = (ByteBuffer) selectionKey.attachment();
                    socketChannel.read(buffer);
                    System.out.println("from 客户端： " + new String(buffer.array()));
                }
                // 删除已经有的事件
                iterator.remove();
            }

        }
    }
}
```

客户端：

```java
package Nio.Selector;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;

public class NIOClient {
    public static void main(String[] args) throws Exception {
        SocketChannel socketChannel = SocketChannel.open();
        InetSocketAddress address = new InetSocketAddress("localhost", 9999);
        socketChannel.configureBlocking(false);
        // 连接服务器
        boolean connect = socketChannel.connect(address);
        if (!connect) {
            // 等待连接，判断是否连接
            while (!socketChannel.finishConnect()) {
                System.out.println("连接服务器中...............");
            }
        }

        String msg = "hello server";
        ByteBuffer wrap = ByteBuffer.wrap(msg.getBytes());
        // 将数据写入到通道之中
        socketChannel.write(wrap);
        // 防止程序停止
        System.in.read();
    }
}
```



## SelectionKey

在`SelectionKey`类中有四个常量表示四种事件，来看看源码：

```java
// 读事件
public static final int OP_READ = 1 << 0;
// 写事件
public static final int OP_WRITE = 1 << 2;
// 连接操作，Client端支持的一种操作
public static final int OP_CONNECT = 1 << 3;
// 连接可接受操作，仅ServerSocketChannel支持
public static final int OP_ACCEPT = 1 << 4;
```

附加的对象（可选），把通道注册到选择器中时可以附加一个对象。

```java
public final SelectionKey register(Selector sel, int ops, Object att)
```

从`selectionKey`中获取对象可以使用`attachment()`方法

```java
public final Object attachment() {
    return attachment;
}
```



# 9.NIO实现多人聊天室

实战例子，用NIO实现一个多人运动版本的聊天室。

服务端代码：

```java
package Nio.Group;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.util.Iterator;
import java.util.Set;

public class GroupChatServer {
    private Selector selector;

    private ServerSocketChannel serverSocketChannel;

    public static final int Port = 9999;
    public GroupChatServer() throws Exception {
        this.selector = Selector.open();
        this.serverSocketChannel = ServerSocketChannel.open();
        // 绑定端口
        this.serverSocketChannel.bind(new InetSocketAddress("localhost", Port));
        // 设置为非阻塞
        serverSocketChannel.configureBlocking(false);
        // 把通道注册到选择器中
        serverSocketChannel.register(selector, SelectionKey.OP_ACCEPT);
    }

    public void listen() throws Exception {
        while (true) {
            // 获取事件的总数
            int count = selector.select(2000);
            if (count > 0) {
                // 获取事件的集合
                Set<SelectionKey> selectionKeys = selector.selectedKeys();
                // 获取迭代器，感觉这个和分散读很像，呸，感觉就是分散读
                Iterator<SelectionKey> iterator = selectionKeys.iterator();
                while (iterator.hasNext()) {
                    // 一个单独的事件
                    SelectionKey key = iterator.next();
                    // 是否可以获取连接
                    if (key.isAcceptable()) {
                        SocketChannel socketChannel = serverSocketChannel.accept();
                        // 设置为非阻塞
                        socketChannel.configureBlocking(false);
                        // 注册到选择器之中
                        socketChannel.register(selector, SelectionKey.OP_READ);
                        System.out.println("连接成功");
                        System.out.println(socketChannel.getRemoteAddress() + "上线了~");
                    }

                    if (key.isReadable()) {
                        readData(key);
                    }
                    iterator.remove();
                }
            } else {
                System.out.println("等着上线........");
            }
        }
    }

    private void readData(SelectionKey selectionKey) {
        SocketChannel socketChannel = null;
        try {
            // 从selectionKey中获取channel
            socketChannel = (SocketChannel) selectionKey.channel();
            ByteBuffer byteBuffer = ByteBuffer.allocate(1024);
            int count = socketChannel.read(byteBuffer);
            if (count > 0) {
                String msg = new String(byteBuffer.array());
                System.out.println(socketChannel.getRemoteAddress() + "from 客户端: " + msg);
                notifyAllClient(msg, socketChannel);
            }
        } catch (IOException e) {
            try {
                System.out.println(socketChannel.getRemoteAddress() + "离线了......");
                // 取消注册
                selectionKey.cancel();
                // 关闭流
                socketChannel.close();
            } catch (IOException ioException) {
                ioException.printStackTrace();
            }
            e.printStackTrace();
        }
    }

    private void notifyAllClient(String msg, SocketChannel noNotifyChannel) throws IOException {
        System.out.println("服务器转发信息~");
        // 这里不能用selector.selectedKeys(), 这里是要给无论在线还是离线的人都发送消息。
        // 用这个就是只是给无论离线还是在线的人都发送
        for (SelectionKey selectionKey : selector.keys()) {
            Channel channel = selectionKey.channel();
            if (channel instanceof SocketChannel && channel != noNotifyChannel) {
                // 转成SocketChannel类型
                SocketChannel socketChannel = (SocketChannel) channel;
                // 创建一个缓冲区
                ByteBuffer wrap = ByteBuffer.wrap(msg.getBytes());
                // 将这个缓冲区的内容发送的所有的客户机
                socketChannel.write(wrap);
            }
        }

    }

    public static void main(String[] args) throws Exception {
        GroupChatServer groupChatServer = new GroupChatServer();
        // 启动服务器进行监听
        groupChatServer.listen();
    }
}
```

客户端代码：

```java
package Nio.Group;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.util.Iterator;
import java.util.Scanner;

public class GroupChatClient {
    private Selector selector;

    private SocketChannel socketChannel;

    private String userName;

    public GroupChatClient() throws IOException {
        this.selector = Selector.open();
        socketChannel = SocketChannel.open(new InetSocketAddress("localhost", 9999));
        socketChannel.configureBlocking(false);
        socketChannel.register(selector, SelectionKey.OP_READ);
        userName = socketChannel.getLocalAddress().toString().substring(1);
        System.out.println(userName + "is ok~");
    }
    // 发送消息
    private void sendMsg(String msg) throws IOException {
        msg = userName + "说：" + msg;
        socketChannel.write(ByteBuffer.wrap(msg.getBytes()));
    }

    private void readMsg() throws IOException {

        int count = selector.select();
        if (count > 0) {
            Iterator<SelectionKey> iterator = selector.selectedKeys().iterator();
            while (iterator.hasNext()) {
                SelectionKey selectionKey = iterator.next();
                if (selectionKey.isReadable()) {
                    SocketChannel socketChannel = (SocketChannel) selectionKey.channel();
                    ByteBuffer byteBuffer = ByteBuffer.allocate(1024);
                    socketChannel.read(byteBuffer);
                    System.out.println(new String(byteBuffer.array()));
                }
                iterator.remove();
            }
        }
    }

    public static void main(String[] args) throws IOException {
        GroupChatClient chatClient = new GroupChatClient();
        new Thread(() -> {
            while (true) {
                try {
                    chatClient.readMsg();
                    Thread.sleep(3000);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();

        Scanner scanner = new Scanner(System.in);
        while (scanner.hasNextLine()) {
            String msg = scanner.nextLine();
            chatClient.sendMsg(msg);
        }
    }

}
```

所有完整代码可参考：https://github.com/zouyishan/JavaNio/tree/master

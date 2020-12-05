# 字节流

我们就讲讲一些实用的IO流，总共包括15个可以分为字节流和字符流。详细如下图：

  ![image-20201205155908899](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201205155911.png)

emmm 还有一个就是`PrintStream`就是我们每次输出的时候是`System.out.println`。这个`System.out`就是`PrintStream`类型啊。然后`PrintStream`重载了`println`这个方法。



关于继承关系呢。我想就读者自己看源码吧，网上分析了很多。但是看的再多，也不如用`Ctrl` + 左键点开源码看来的舒服啊！



`FileInputStream`就是读取一个文件的字节流。详细操作如下：注意`FileInputStream`里面的路径是我自己电脑路径下的。将C盘的一个图片复制到D盘。

```java
package zou.IO;

import java.io.FileInputStream;
import java.io.FileOutputStream;

public class testInput {
    public static void main(String[] args) throws Exception {
        InputStream fileInputStream = new FileInputStream("C:\\Users\\86131\\Desktop\\myimage\\bg.jpg");
        OutputStream fileOutputStream = new FileOutputStream("D:\\1.jpg");
        byte[] bytes = new byte[fileInputStream.available()];
        fileInputStream.read(bytes);
        fileOutputStream.write(bytes);
        fileInputStream.close();
        fileOutputStream.close();
    }
}
```





`BufferedInputStream`就是加个缓冲区域。就让读取速度变快，那我们看看为什么。

一开始学习处理流会疑问为什么速度会加快呢？好比一个10KB的文件。使用最基本的字节流读写，只要读一次10KB到内存。存一次10KB到目标文件就行了。但是使用缓冲不是要读1次10KB到缓冲，再从缓冲写一次10KB到CPU，再从CPU写10KB到缓冲，再从缓冲10KB到目标文件吗？明显过程长的！

其实不然！如下：

首先为什么要设置缓冲？

如果不设置缓冲的话，CPU取一个存一个。设置缓冲区的话：就好比是把基础流用DMA方式包装起来如下：

CPU通常会使用 DMA 方式去执行 I\O流 操作。CPU 将这个工作交给DMA控制器来做，自己腾出时间做其他的事，当DMA完成工作时，DMA会主动告诉CPU“操作完成”。这时，CPU接管后续工作。在此，CPU 是被动的。DMA是专门 做 I＼O 与 内存 数据交换的，不仅自身效率高，也节约了CPU时间，CPU在DMA开始和结束时做了一些设置罢了。 缓冲区就是内存里的一块区域，把数据先存内存里，然后一次性写入，类似数据库的批量操作，显然效率比较高。还节省CPU的使用。

总结：使用缓冲处理流包装就是一堆一堆的干活，还能不用CPU多次处理数据转换，只是设置一下数据转换成功后的文件。

   不使用缓冲处理流包装就是CPU傻傻的一个字节一个字节循环来干活存储写入文件中，相比可见效率明显变慢

这是有缓冲的，记住关闭流。

```java
package zou.IO;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;

public class testInputhuancun {
    public static void main(String[] args) throws Exception{
        InputStream inputStream = new BufferedInputStream(new FileInputStream("C:\\Users\\86131\\Desktop\\myimage\\6039_3.jpg"));
        OutputStream outputStream = new BufferedOutputStream(new FileOutputStream("D:\\1.jpg"));
        byte[] bytes = new byte[1024];
        int len = inputStream.read(bytes);
        while (len != -1) {
            outputStream.write(bytes, 0, len);
            len = inputStream.read(bytes);
        }
        outputStream.flush();
        inputStream.close();
        outputStream.close();
    }
}
```



# 字符流

`FileReader`其实字符流的操作和字节流差不多。只是内容变成了文本。

```java
package zou.IO.zifuliu;

import java.io.FileReader;
import java.io.FileWriter;
import java.io.Reader;
import java.io.Writer;

public class testReader {
    public static void main(String[] args) throws Exception {
        Reader fileReader = new FileReader("C:\\Users\\86131\\Desktop\\test.txt");
        Writer fileWriter = new FileWriter("D:\\11.txt");
        char[] chars = new char[1024];
        int read = fileReader.read(chars);
        while (read != -1) {
            fileWriter.write(chars, 0, read);
            read = fileReader.read(chars);
        }
        fileReader.close();
        fileWriter.close();
    }
}
```



`BufferedReader`也和字节流同理：

```java
package zou.IO.zifuliu;

import java.io.*;

public class testReaderBuffered {
    public static void main(String[] args) throws Exception {
        Reader reader = new BufferedReader(new FileReader("C:\\Users\\86131\\Desktop\\test.txt"));
        BufferedWriter writer = new BufferedWriter(new FileWriter("D:\\1.txt"));
        char[] chars = new char[1024];
        int read = reader.read(chars);
        while (read != -1) {
            writer.write(chars, 0, read);
            read = reader.read(chars);
        }
        reader.close();
        writer.close();
    }
}
```





# 用户怎么输入呢

1. 这里我们就要用到转换流了，`InputStreamReader`这是将里面的字节流转换为字符流。好了 到这我们就可以等待输入字符串，然后我们这里输出的Java程序。

```java
package zou.IO.zifuliu;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class testgetinput {
    public static void main(String[] args) throws Exception {
        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        String s = reader.readLine();
        System.out.println(s);
    }
}
```



2. 不通过转换流直接通过new一个String来也行。不过这个byte的值是个未知数。所以不推荐。

```java
package zou.IO.zifuliu;

public class teststand {
    public static void main(String[] args) throws Exception {
        byte[] bytes = new byte[1024];
        int read = System.in.read(bytes);
        String s = new String(bytes, 0, read);
        System.out.println(s);
    }
}
```



3. 还有一个是最标准的东西了，就用这个了,很简单。只用传一个用户输入的字节流`System.in`就可以愉快的输入了。

```java
package zou.IO.zifuliu;

import java.util.Scanner;

public class standerinput {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        String i = scanner.nextLine();
        System.out.println(i);
    }
}
```



# 关于`System.in`和`System.out`

好了 这里在聊聊`System.out`这个`out`是在System类中一个用`final static`修饰的`PrintStream`类型的常量在源码里面：

```java
/**
     * The "standard" output stream. This stream is already
     * open and ready to accept output data. Typically this stream
     * corresponds to display output or another output destination
     * specified by the host environment or user.
     * <p>
     * For simple stand-alone Java applications, a typical way to write
     * a line of output data is:
     * <blockquote><pre>
     *     System.out.println(data)
     * </pre></blockquote>
     * <p>
     * See the <code>println</code> methods in class <code>PrintStream</code>.
     *
     * @see     java.io.PrintStream#println()
     * @see     java.io.PrintStream#println(boolean)
     * @see     java.io.PrintStream#println(char)
     * @see     java.io.PrintStream#println(char[])
     * @see     java.io.PrintStream#println(double)
     * @see     java.io.PrintStream#println(float)
     * @see     java.io.PrintStream#println(int)
     * @see     java.io.PrintStream#println(long)
     * @see     java.io.PrintStream#println(java.lang.Object)
     * @see     java.io.PrintStream#println(java.lang.String)
     */
    public final static PrintStream out = null;
```

又点进去`PrintStream`的源码。我么就可以看到这个了

![image-20201201162538149](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201201163636.png)



那我么再来看看`System.in`可以看到，官方的解释就是标准输入。当没有缺省源的时候就是读取控制台的输入。

```java
/**
     * The "standard" input stream. This stream is already
     * open and ready to supply input data. Typically this stream
     * corresponds to keyboard input or another input source specified by
     * the host environment or user.
     */
    public final static InputStream in = null;
```

好了，这就是基本的I/O流了~~


# Tomcat的设计架构

我们思考以下：Tomcat是个web容器，那么它要解决说明问题？

1. 一个web容器可能需要部署两个应用程序，**不同的应用程序可能会依赖同一个第三方类库的不同版本。**不能要求同一个类库在同一个服务器只有一份，因此**保证每个应用程序的类库都是独立的，保证相互隔离**
2. 部署在同一个web容器中相同的类库相同的版本可以共享。这与第一点正好相反，但也很常见，如果服务器有10个应用程序，那么要有10份相同的类库加载进虚拟机内存，方法区很吃力。
3. 服务器需要尽可能地保证自身的安全不受部署的web应用程序影响。很多主流的Javaweb服务器自生也是Java实现。因此服务器本身也有类库依赖问题，基于安全问题，服务器所使用的类库应该与应用程序的类库相互独立。
4. 支持JSP的服务器要支持HotSwap功能。jsp文件最终也是要编译成class文件才能在虚拟机中运行。

由于存在上述问题，在部署Web应用时，单独的一个ClassPath就不能满足需求了，**所以各种Web服务器都不约而同地提供了好几个有着不同含义的ClassPath路径供用户存放第三方类库，具备不同的访问范围和服务对象，通常每一个目录都会有一个相应的自定义类加载器取加载防止在里面的Java类库。**那就让我们开始分析以下Tomcat吧！



# Tomcat如何实现自己独特的类加载机制

Tomcat团队已经给我们设计好了，看看它们的设计图：

![类加载器](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210226003443.jpg)

我们看到，前面3个类加载和默认的一致，CommonClassLoader，CatalinaClassLoader，SharedClassLoader和WebappClassLoader则是Tomcat自己定义的类加载器，它们分别加载`/common/*`, `/server/*`,`/shared/*`（在tomcat6之后已经合并到根目录下的lib目录下）和`/WebApp/WEB-INF/*`中的Java类库。**其中WebApp类加载器和Jsp类加载器通常会存在很多个实例，每一个Web应用程序对应一个WebApp类加载器，每一个JSP文件对应一个Jsp类加载器。**

* **commonLoader：**Tomcat最基本的类加载器，加载路径中的class可以被Tomcat容器本身以及各个Webapp访问
* **catalinaLoader：**Tomcat容器私有的类加载器，加载路径中的class对于Webapp不可见
* **sharedLoader：**各个Webapp共享的类加载器，加载路径中的class对于所有Webapp可见，但是对于Tomcat容器不可见
* **WebappClassLoader：**各个Webapp私有的类加载器，加载路径中的class只对当前Webapp可见

从图中的委派关系中可以看出：

> CommonClassLoader能加载的类都可以被Catalina ClassLoader和SharedClassLoader使用，从而实现了公有类库的共用，而CatalinaClassLoader和Shared ClassLoader自己能加载的类则与对方相互隔离。这就解决了第二点和第三点
>
> WebAppClassLoader可以使用SharedClassLoader加载到的类，但各个WebAppClassLoader实例之间相互隔离。这就解决了第一点
>
> 而JasperLoader的加载范围仅仅是这个JSP文件所编译出来的那一个.Class文件，它出现的目的就是为了被丢弃：当Web容器检测到JSP文件被修改时，会替换掉目前的JasperLoader的实例，并通过再建立一个新的Jsp类加载器来实现JSP文件的HotSwap功能。这就解决了第四点

这样一个Tomcat独特的类加载机制完美符合一个web容器！



# 一些拓展

## tomcat违背了双亲委派模型吗？

违背了，之前说过：

> 双亲委派模型要求处理顶层的启动类加载器之外，其余的类加载器都应当由自己的父类加载器加载

显然，tomcat为了实现隔离性，没有遵守这个约定，每个webappClassLoader加载自己的目录下的class文件，不会传递给父类加载器。

## 如果CommonClassLoader想加载WebApp ClassLoader中的类怎么办

这个问题很好回答，这是典型的破坏双亲委派，使用上下文加载器即可，可以让父类加载器请求子类加载器取完成类加载的动作。

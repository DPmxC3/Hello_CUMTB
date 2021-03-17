# Spring MVC的执行流程

## 第一站 DispatcherServlet

从请求离开浏览器之后，第一站到达的就是DispatcherServlet，看名字就是一个Servlet,Servlet可以拦截并处理HTTP请求，DispatcherServlet会拦截所有的请求，并且将这些请求发送给Spring MVC控制器。

```xml
<servlet>
    <servlet-name>dispatcher</servlet-name>
    <servlet-class>org.springframework.web.servlet.DispatcherServlet</servlet-class>
    <load-on-startup>1</load-on-startup>
</servlet>
<servlet-mapping>
    <servlet-name>dispatcher</servlet-name>
    <!-- 拦截所有的请求 -->
    <url-pattern>/</url-pattern>
</servlet-mapping>
```

* DispatcherServlet的任务就是拦截请求发送给Spring MVC控制器



## 第二站 处理器映射(HandlerMapping)

问题：经典的应用程序中可能有很多个控制器，这些请求到底应该发给哪一个控制器呢？

所以DispatcherServlet会查询一个或多个处理器映射来确定请求的下一站在哪里，处理器映射会根据请求所携带的URL信息来进行决策，例如上面的例子中，我们通过配置`simpleUrlHandlerMapping`或者`BeanNameUrlHandlerMapping`来将/hello地址交给helloController处理：

```xml
<bean id="simpleUrlHandlerMapping"
      class="org.springframework.web.servlet.handler.SimpleUrlHandlerMapping">
    <property name="mappings">
        <props>
            <!-- /hello 路径的请求交给 id 为 helloController 的控制器处理-->
            <prop key="/hello">helloController</prop>
        </props>
    </property>
</bean>
<bean id="helloController" class="controller.HelloController"></bean>
```



## 第三站 控制器

一旦选择了合适的控制器，DispatcherServlet会将请求发送给选中的控制器，到了控制器，请求会卸下其负载(用户提交的请求)等待处理器处理完这些信息：

```java
public ModelAndView handleRequest(javax.servlet.http.HttpServletRequest httpServletRequest, javax.servlet.http.HttpServletResponse httpServletResponse) throws Exception {
    // 处理逻辑
    ....
}
```



## 第四站 返回DispatcherServlet

当控制器在完成逻辑处理后，通常会产生一些信息，这些信息就是需要返回给用户并在浏览器上显示的信息，它们被称为`模型(Model)`。仅仅返回原始的信息是不够的。这些信息需要以用户友好的方式进行格式化，一般会是HTML，所以，信息需要发送给一个**视图**,通常会是JSP。

控制器所做的最后一件事是将模型数据打包，并且表示出用于渲染输出的视图名**(逻辑视图名)**。他接下来会将请求连同模型和视图名发送回DispatcherServlet。

```java
public ModelAndView handleRequest(javax.servlet.http.HttpServletRequest httpServletRequest, javax.servlet.http.HttpServletResponse httpServletResponse) throws Exception {
    // 处理逻辑
    ....
    // 返回给 DispatcherServlet
    return mav;
}
```

## 第五站 视图解析器

这样一来，控制器就不会和特定的视图相耦合，传递给DispatcherServlet的视图名并不直接表示某个特定的JSP。**相反，它传递的仅仅是一个逻辑名称，这个名称将会用来查找产生结果的真正视图**。

DispatcherServlet将会使用视图解析器(view resolver)来将逻辑视图名匹配为一个特定的视图实现，他可能是也可能不是JSP



## 第六站 视图

既然DispatcherServlet已经知道由哪个视图渲染结果了，那请求的任务基本上是完成了。

它的最后一站是视图的实现，在这里它交付模型数据，请求的任务也就完成了。视图使用模型数据渲染出结果，这个输出结果会通过响应对象传递给客户端。

```jsp
<%@ page language="java" contentType="text/html; charset=UTF-8"
         pageEncoding="UTF-8" isELIgnored="false"%>

<h1>${message}</h1>
```



# 注解配置Spring MVC

上面是一个典型的通过XML配置的方式创建了第一个Spring MVC程序，我们来看看基于注解应该怎么完成上述程序的配置：

## 第一步 为HelloController添加注解

```java
package controller;

import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.servlet.ModelAndView;

@Controller
public class HelloController{

    @RequestMapping("/hello")
    public ModelAndView handleRequest(javax.servlet.http.HttpServletRequest httpServletRequest, javax.servlet.http.HttpServletResponse httpServletResponse) throws Exception {
        ModelAndView mav = new ModelAndView("index.jsp");
        mav.addObject("message", "Hello Spring MVC");
        return mav;
    }
}
```

**解释一下：**

@Controller注解：很明显这个注解是用来声明控制器的。

@RequestMapping注解：很显然，这就表示路径`/hello`会映射到该方法上



## 第二步：取消之前的XML注释

在`dispatcher-servlet.xml`文件中注释掉之前的配置，然后增加一句组件扫描:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xmlns:context="http://www.springframework.org/schema/context"
       xsi:schemaLocation="http://www.springframework.org/schema/beans http://www.springframework.org/schema/beans/spring-beans.xsd http://www.springframework.org/schema/context http://www.springframework.org/schema/context/spring-context.xsd">

    <!--<bean id="simpleUrlHandlerMapping"-->
                                        <!--class="org.springframework.web.servlet.handler.SimpleUrlHandlerMapping">-->
    <!--<property name="mappings">-->
            <!--<props>-->
                <!--&lt;!&ndash; /hello 路径的请求交给 id 为 helloController 的控制器处理&ndash;&gt;-->
                <!--<prop key="/hello">helloController</prop>-->
            <!--</props>-->
        <!--</property>-->
    <!--</bean>-->
    <!--<bean id="helloController" class="controller.HelloController"></bean>-->

    <!-- 扫描controller下的组件 -->
    <context:component-scan base-package="controller"/>
</beans>
```

## 配置视图解析器

还记得我们Spring MVC的请求流程吗？视图解析器负责定位视图，它接受一个由DispatcherServlet传递过来的逻辑视图名来匹配一个特定的视图。

**问题：**

我们一些页面不希望用户直接能访问到，例如有重要数据的页面，例如有模型数据支撑的页面。我们可以在`web`根目录下放置一个`test.jsp`模拟一个重要数据的页面，我们什么都不用做，重新启动服务器，网页中输入`localhost/test.jsp`就能直接访问，这样会造成数据泄露。**另外，我们可以直接输入`localhost/index.jsp`，根据我们上面的程序，这是一个空白的页面，因为并没有获取到${message}参数就直接访问了**

**解决方案：**

我们将我们的JSP文件配置在`WEB-INF`文件夹中的`page`文件下，`WEB-INF`是Java Web中默认的安全目录，是不允许用户直接访问的。也就是说你通过`localhost/WEB-INF/`这样的方式是永远访问不到的!

但是我们需要将这告诉给视图解析器，我们在`dispatcher-servlet.xml`文件中做如下配置：

```xml
<bean id="viewResolver"
      class="org.springframework.web.servlet.view.InternalResourceViewResolver">
    <property name="prefix" value="/WEB-INF/page/" />
    <property name="suffix" value=".jsp" />
</bean>
```

这里配置了一个 Spring MVC 内置的一个视图解析器，该解析器是遵循着一种约定：会**在视图名上添加前缀和后缀，进而确定一个 Web 应用中视图资源的物理路径的。**



## 使用Model对象

在Spring MVC中，我们通常都是使用这样的方式来绑定数据：

```java
@RequestMapping("/value")
public String handleRequest(Model model) {
    model.addAttribute("message", "yes");
    return "test1";
}
```

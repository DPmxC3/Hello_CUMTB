# 分派

本章要求你能理解类加载的过程这些前置知识，不然看不懂。

[静态分派](#静态分派)

[动态分派](#动态分派)

[虚拟机动态分派的实现](#虚拟机动态分派的实现)
&nbsp;


首先要知道的字节码指令：

* **invokestatic**： 用于调用静态方法
* **invokespecial**： 用于调用实例构造器< init >()方法，私有方法和父类中的方法
* **invokevirtual**：用于调用所有的虚方法
* **invokeinterface**： 用于调用接口方法， 会在运行时在确定一个实现改接口的对象
* **invokedynamic**：现在运行时动态解析出调用点限定符所引用方法，然后再执行该方法 。前面四条调用指令，分派逻辑都固化在Java虚拟机内部，而invokedynamic指令的分派逻辑时由用户设定的引导方法来决定的。



## 静态分派

首先看代码：

```java
package zou.reflect;

class human{}

class man extends human {}

class woman extends human {}
public class test1 {
    public void say (human guy) {
        System.out.println("hey guy");
    }

    public void say (man guy) {
        System.out.println("hey boy");
    }

    public void say (woman guy) {
        System.out.println("hey girl");
    }

    public static void main(String[] args) {
        human woman = new woman();
        human man = new man();
        human human = new human();
        test1 test1 = new test1();
        test1.say(human);
        test1.say(man);
        test1.say(woman);
    }
}
```

> 运行结果：
>
> hey guy
> hey guy
> hey guy

应该能看出一些

```java
human woman = new woman();
```

这里我们把`human`称之为`静态类型`,后面的`woman`我们称之为`实际类型`。静态类型和实际类型在程序中都可能会发生变化，**区别是，最终的静态类型是在编译器可知的。**然而，**实际类型只有在运行期才可以确定**

例如对象`woman`的实际类型是可变的，编译期间完全是`薛定谔`的。到底是`man`还是`woman`，必须等到程序运行到这行的时候才能确定。而这个`human`类型是静态类型。那实际类型是`woman`和`man`那为什么输出的还是`human`类型的重载呢？



解答谜题的时候到了==



**虚拟机（或者准确地说是编译器）在重载时是通过参数的静态类型而不是实际类型作为判定依据的。由于静态类型在编译期可知，所以在编译阶段，Javac编译器就根据参数的静态类型决定了会使用哪个重载版本，因此选择了sayHello(Human)作为调用目标，并把这个方法的符号引用写到 `main()`方法里的两条`invokevirtual`指令的参数中。**



所有依赖`静态类型`来决定方法执行版本的分派动作，都称为`静态分派`。静态分派的最典型应用表现就是`重载!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!`。因此确定静态分派的动作实际上不是由虚拟机来执行的。所以这个过程只是`解析`不能称为`分派`



Javac编译器虽然能确定方法重载的版本，但是很多情况下这个重载版本并不是`唯一`的，往往只能确定一个`相对更适合的`版本。例如：

```java
package zou.fenpai;

import java.io.Serializable;

public class test1 {
    public static void sayHello(Object arg) {
        System.out.println("hello Object");
    }
    public static void sayHello(int arg) {
        System.out.println("hello int");
    }
    public static void sayHello(long arg) {
        System.out.println("hello long");
    }
    public static void sayHello(Character arg) {
        System.out.println("hello Character");
    }
    public static void sayHello(char arg) {
        System.out.println("hello char");
    }
    public static void sayHello(String args) {
        System.out.println("hello String");
    }
    public static void sayHello(char... arg) {
        System.out.println("hello char ...");
    }
    public static void sayHello(Serializable arg) {
        System.out.println("hello Serializable");
    }
    public static void main(String[] args) {
        sayHello('a');
    }
}
```

> 上面毫无疑问会输出：hello char
>
> 但是注释了char类型的方法呢？
>
> 程序就会输出：hello int

这时会发生一个自动类型转换，`a`除了代表字符，还代表97这个ascii码。

如果在注释掉int方法，就会继续进行自动装箱成为别的类型。

**所以论证了虚拟机只能确定一个`相对更适合的`版本。 同时也更加论证了：编译期间选择静态分派目标的过程，这个过程也是Java语言实现方法重载的本质！！！！！**





## 动态分派

先请您看看这个代码

```java
package zou.fenpai;

public class test2 {
    static class human{
        public void say() {
            System.out.println("hey guy");
        }
    }
    
    static class man extends human {
        public void say() {
            System.out.println("hey boy");
        }
    }
    
    static class woman extends human {
        public void say() {
            System.out.println("hey girl");
        }
    }

    public static void main(String[] args) {
        human woman = new woman();
        human man = new man();
        human human = new human();
        woman.say();
        man.say();
        human.say();
    }
}
```

> 运行结果：
>
> hey girl
> hey boy
> hey guy

这个结果就完全不出您的所料了吧！没错 这就是重写一个方法。这就是所能理解的多态了吧。但是，我们深究一下原理是上面，为什么`静态类型`都一样，`动态类型`不一样，偏偏调用的是`动态类型`呢？



这时，Java层面已经不能解释这个问题了，然我们看看程序的字节码吧！

这个字节码还算简单：

 ![image-20201202202428526](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201202214949.png)

可以看到主要就是24行开始到33行。看到注释都是调用的`human.say()`方法。理应是`human`方法啊。玄机就在这个`invokevirtual`指令上。让我们来看看这个`invokevirtual`到底干了什么事。



1. 找到操作数栈顶的第一个元素所指向的对象的实际类型，记作C。

2. 如果在类型C中找到与常量中的描述符和简单名称都相符的方法，则进行访问权限校验，如果通过则返回这个方法的直接引用，查找过程结束；不通过则返回`java.lang.IllegalAccessError`异常。

3. 否则，按照继承关系从下往上依次对C的各个父类进行第二步的搜索和验证过程。

4. 如果始终没有找到合适的方法，则抛出`java.lang.AbstractMethodError`异常



正是由于`invokevirtual`指令第一步就是找`实际类型`, **还会根据方法接收者的实际类型选择版本，这就是Java语言中方法重写的本质**



同时，也可以轻易发现Java的多态就是这个`invokevirtual`方法的作用，然后在上面我们就给出，`invokevirtual`方法只是用于方法，不适用于字段，所以字段并没有多态性。



## 虚拟机动态分派的实现

在Java层面上我想对于分派已经有所理解了，那虚拟机呢？



动态分派是执行非常频繁的动作，而且动态分派的方法版本选择过程需要运行时在接收者类型的 方法元数据中搜索合适的目标方法，因此，Java虚拟机实现基于执行性能的考虑，真正运行时一般不会如此频繁地去反复搜索类型元数据。面对这种情况，一种基础而且常见的优化手段是为类型在方法区中建立一个`虚方法表`（Virtual Method Table，也称为vtable，与此对应的，在`invokeinterface`执行时也 会用到接口方法表——Interface Method Table，简称itable），使用虚方法表索引来代替元数据查找以 提高性能。

![image-20201202214219714](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201202214946.png)

虚方法表中存放着各个方法的实际入口地址。**如果某个方法在子类中没有被重写**，那子类的虚方法表中的地址入口和父类相同方法的地址入口一致，都指向父类的实现入口。**如果子类中重写了这个方法**，子类虚方法表中的地址也会被替换成为指向子类实现版本的入口地址。



查虚方法表是分派调用的一种优化手段，**由于Java对象里面的方法默认（即不 使用final修饰）就是虚方法**，虚拟机除了使用虚方法表之外，为了进一步提高性能，还会使用类型继承关系分析（Class Hierarchy Analysis，CHA）、守护内联（Guarded Inlining）、内联缓存（Inline Cache）等多种非稳定的激进优化来争取更大的性能空间。
&nbsp;
&nbsp;
&nbsp;
&nbsp;
&nbsp;

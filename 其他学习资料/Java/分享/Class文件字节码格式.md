# Class文件字节码格式

废话不多说 直接开始

**Class文件总体结构：**

![总体结构](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031000954.png)

约定：

* 无符号数属于基本类型数据结构，以u1,u2,u4,u8分别表示1,2,4,8字节
* 表是由多个无符号数或者其他表作为数据项构成的符合数据类型，为了便于区分，所有表的命名都习惯性以"_info"结尾。所以实际上整个class文件可以看成是一个一个表组成的。

### 魔数与Class文件版本

class文件头四个字节被称为魔数。作用是确定这个文件是否为一个能被虚拟机接受的class文件。使用魔数而不是文件拓展名来识别是因为文件拓展名可以随意改动。同时魔数取得很有浪漫气息 `0xCAFEBABE`(咖啡宝贝)。

然后第五六个字节为class文件得版本号。第七八个字节为主版本号。因为感觉不怎么重要所以不重点讨论。

 ![魔数](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031002719.png)

### 常量池

常量池开始 也就是九十个字节放得是常量池容量得计数值。这个计数值是从1开始的，设计者将0项常量空出来的目的是。如果后面某些指向常量池的索引值的数据在特定情况下需要表达"不引用任何一个常量池项目"的含义

然后常量池中每一项常量都是一个表，最初常量表中共有11种结构各不相同的表结构数据，后来为了更好地支持动态语言调用，增加了4种动态语言相关的常量，为了支持Java模块化系统，又加入了CONSTANT_Module_info和CONSTANT_Package_info两个常量，截至到JDK13，常量表种分别有17种不同类型的常量

 ![常量池](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031003321.png)

然后之前说了，每个常量项都是一个表。所以存的数据就是根据常量项的表来存的。

举例一个项：

 ![常量](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031004406.png)

然后看看对应的第项字节码。完全符合吧~~

 ![常量表结构](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031003918.png)



所以说。常量池存的数据都是一个一个的表。我们要是记住各个数据项的格式，就能完美的看懂字节码了！！！！17个总表太长了。就不展示了，自行百度就可以。



好了 再让我们看看一个程序的字节码吧== 此字节码是通过javap工具的-verbose来生成。 具体代码是这样的:

```java
package org.fenixsoft.clazz;
public class TestClass {    
    private int m;    
    public int inc() {
        return m + 1;    
    }
}
```

![程序的字节码](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031004723.png)

其中有部分常量不是源自于Java源代码。比如"I","V"这些都是编译器自动产生的。



### 访问标志

在常量池结束之后，紧接着的2个字节代表访问标志（access_flags），这个标志用于识别一些类或 者接口层次的访问信息，包括：这个Class是类还是接口；是否定义为public类型；是否定义为abstract 类型；如果是类的话，是否被声明为final；等等。具体的标志位以及标志的含义见表：

![访问标志](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031005147.png)

比如看看我们字节码里面怎么写的：

![字节码](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031005227.png)

这里写的是`0021`对应我们就可以得出。他的标志是: 0x0001 | 0x0020 = 0x0021(异或运算)，access_flag的标志就是`ACC_SUPER` 和 `ACC_PUBLIC`



### 类索引，父类索引与接口索引集合

类索引(this_class)和父类索引(super_class)都是一个u2类型的数据，而接口索引集合 (interfaces)是一组u2类型的数据的集合，Class文件中由这三项数据来确定该类型的继承关系。类索 引用于确定这个类的全限定名，父类索引用于确定这个类的父类的全限定名。由于Java语言不允许多 重继承，所以父类索引只有一个，除了`java.lang.Object`之外，所有的Java类都有父类，因此除了 `java.lang.Object`外，所有Java类的父类索引都不为0。接口索引集合就用来描述这个类实现了哪些接 口，这些被实现的接口将按implements关键字(如果这个Class文件表示的是一个接口，则应当是 extends关键字)后的接口顺序从左到右排列在接口索引集合中。



类索引、父类索引和接口索引集合都按顺序排列在访问标志之后，类索引和父类索引用两个u2类 型的索引值表示，它们各自指向一个类型为`CONSTANT_Class_info`的类描述符常量，通过 `CONSTANT_Class_info`类型的常量中的索引值可以找到定义在`CONSTANT_Utf8_info`类型的常量中的 全限定名字符串。

**可以对照上面的字节码的图来看**

![类信息](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031010111.png)



同时接口。如果计数为0的话，后面就可以不屑，如果有的话就按照两个字节两个字节的读取常量池中的索引就行！

![类对应字节码](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031010338.png)



### 字段表集合

字段表(field_info)用于描述接口或者类中声明的变量。Java语言中的“字段”（Field）包括类级变 量以及实例级变量，但不包括在方法内部声明的局部变量。字段可以包括的修饰符有字段的作用域(public、private、protected修饰 符)、是实例变量还是类变量(static修饰符)、可变性(final)、并发可见性(volatile修饰符，是否 强制从主内存读写)、可否被序列化(transient修饰符)、字段数据类型(基本类型、对象、数组)、 字段名称。**上述表示都是可以用布尔值表示有没有修饰的！！！！所以奇怪的表又有了**



**跟随access_flags标志的是两项索引值：name_index和descriptor_index。它们都是对常量池项的引用，分别代表着字段的简单名称以及字段和方法的描述符。** 字段和方法描述和前面的全限定名就不赘述了。。请自行百度。。。



对于数组类型，每一维度将使用一个前置的“[”字符来描述，如一个定义为“java.lang.String[][]”类型 的二维数组将被记录成“[[Ljava/lang/String；”，一个整型数组“int[]”将被记录成“[I”。
用描述符来描述方法时，按照先参数列表、后返回值的顺序描述，参数列表按照参数的严格顺序 放在一组小括号“()”之内。如方法void inc()的描述符为“()V”，方法java.lang.String toString()的描述符 为“()Ljava/lang/String；”，方法int indexOf(char[]source，int sourceOffset，int sourceCount，char[]target， int targetOffset，int targetCount，int fromIndex)的描述符为“([CII[CIII)I”。



**字段访问标志**

![字段访问](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031010851.png)



**描述符标识字符含义**

![image-20201031011013953](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031011017.png)



然后上个字节码吧:

![image-20201031011921144](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031011923.png)

通过查表和上面常量池中对应的值很容易得出这段描述的就是 private int m

字段表所包含的固定数据项目到descriptor_index为止就全部结束了，不过在descrip-tor_index之后 跟随着一个属性表集合，用于存储一些额外的信息，字段表可以在属性表中附加描述零至多项的额外 信息。例如用`final`修饰的变量就会有一项为ConstantValue的属性，指向常量123。这个在进行类加载中就会在准备阶段直接给赋值了。



字段表集合中不会列出从父类或者父接口中继承而来的字段，但有可能出现原本Java代码之中不 存在的字段，譬如在内部类中为了保持对外部类的访问性，编译器就会自动添加指向外部类实例的字 段。



### 方法表集合

Class文件存储 格式中对方法的描述与对字段的描述采用了几乎完全一致的方式，方法表的结构如同字段表一样，依 次包括访问标志(access_flags)、名称索引(name_index)、描述符索引(descriptor_index)、属性表 集合(attributes)几项



**又是惯例性展示方法访问标志**

![image-20201031012549031](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031012550.png)



第一个u2类型的数据（即计数器容量）的值为0x0002，代表集合中有两个方 法，这两个方法为编译器添加的实例构造器<init>和源码中定义的方法inc()。第一个方法的访问标志值 为0x0001，也就是只有ACC_PUBLIC标志为真，名称索引值为0x0007，查代码清单6-2的常量池得方法 名为“<init>”，描述符索引值为0x0008，对应常量为“()V”，属性表计数器attributes_count的值为 0x0001，表示此方法的属性表集合有1项属性，属性名称的索引值为0x0009，对应常量为“Code”，说明 此属性是方法的字节码描述。

![image-20201031012720677](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031012722.png)



**这就是编译器自己添加的方法！！！！有 < init >() 和 < clint >()**

> 哎 太难了 总感觉要写的简练点。但是真正就是有这么多。没办法

在Java语言中，要重载（Overload）一个方法，除了要与原方法具有相同的简单名称之外，还要求 必须拥有一个与原方法不同的特征签名[2]。特征签名是指一个方法中各个参数在常量池中的字段符号 引用的集合，也正是因为返回值不会包含在特征签名之中，所以Java语言里面是无法仅仅依靠返回值 的不同来对一个已有方法进行重载的。但是在Class文件格式之中，特征签名的范围明显要更大一些， 只要描述符不是完全一致的两个方法就可以共存。也就是说，如果两个方法有相同的名称和特征签 名，但返回值不同，那么也是可以合法共存于同一个Class文件中的。

`也就是说有Java层面和字节码层面的特征签名`



### 属性表集合

属性表(attribute_info)在前面的讲解之中已经出现过数次，Class文件、字段表、方法表都可以 携带自己的属性表集合，以描述某些场景专有的信息。与Class文件中其他的数据项目要求严格的顺序、长度和内容不同，属性表集合的限制稍微宽松一 些，不再要求各个属性表具有严格顺序，并且《Java虚拟机规范》允许只要不与已有属性名重复，任 何人实现的编译器都可以向属性表中写入自己定义的属性信息，Java虚拟机运行时会忽略掉它不认识 的属性。到如今。定义的属性已经有29种了。所以还是自行百度~~ 咋门把所有都讲完不太现实。就聊聊`code`属性

#### Code属性

Code属性表结构:

![image-20201031013755023](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201031013756.png)



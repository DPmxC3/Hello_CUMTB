# 简介

## 新增加的内容

jdk8增加了很多东西，大致分为以下几种吧(可能不全)

* Lambda表达式
* 函数式接口
* 方法引用和构造器调用
* Stream API
* 接口中的默认方法和静态方法
* 新时间日期API

## HashMap新机制

在jdk1.8中对hashMap等map集合的数据结构优化。hashMap数据结构的优化原来的hashMap采用的数据结构是哈希表（数组+链表），hashMap默认大小是16，一个0-15索引的数组，如何往里面存储元素，首先调用元素的hashcode方法，计算出哈希码值，经过哈希算法算成数组的索引值，如果对应的索引处没有元素，直接存放，如果有对象在，那么比较它们的equals方法比较内容如果内容一样，后一个value会将前一个value的值覆盖，如果不一样，在1.7的时候，后加的放在前面，形成一个链表，形成了碰撞，在某些情况下如果链表无限下去，那么效率极低，碰撞是避免不了的加载因子：0.75，数组扩容，达到总容量的75%，就进行扩容，但是无法避免碰撞的情况发生在1.8之后，在数组+链表+红黑树来实现hashmap，当碰撞的元素个数大于8时 & 总容量大于64，会有红黑树的引入除了添加之后，效率都比链表高，1.8之后链表新进元素加到末尾ConcurrentHashMap (锁分段机制)concurrentLevel,jdk1.8采用CAS算法(无锁算法，不再使用锁分段)，数组+链表中也引入了红黑树的使用



# Lambda表达式

## 为什么使用

* 在Java中，我们无法将函数作为参数传递给一个方法，也无法声明返回一个函数的方法，也无法声明返回一个函数的方法
* 在JavaScript中，函数参数是一个函数，返回值是另一个函数的情况是非常常见的；JavaScript是一门经典的函数式语言。

## Lambda表达式的作用

* lambda表达式为Java添加了缺失的函数式编程特性，使我们能将函数当作一等公民看待
* 在将函数作为一等公民的语言中，lambda表达式的类型是函数。但在Java中，lambda表达式是对象，它们必须依附于一类特别的对象类型：函数式接口(functional interface).

## 匿名内部类

**以前的书写**

```java
public static void main(String[] args) {
    Thread thread = new Thread(new Runnable() {
        @Override
        public void run() {
            for (int i = 0; i < 100; i++) {
                System.out.println(i);
            }
        }
    });
}
```

使用了lambda表达式

```java
public static void main(String[] args) {
    new  Thread(()->{
        for (int i = 0; i < 100; i++) {
            System.out.println(i);
        }
    });
}
```



## lambda表达式格式

### 简介

> (参数列表) -> {}
>
> (参数列表)：可有可无
>
> {}：方法体
>
> ->：没有实际意义起到连接作用



### 无参无返回值

```java
package zou.lambda;

import java.util.function.Function;

interface Hello {
    void sayHello();
}

public class test1 {
    public static void fun(Hello hello) {
        hello.sayHello();
    }
    public static void main(String[] args) {
        fun(() -> {
            System.out.println("Hello World");
        });
    }
}
```



### 一个参数无返回值

```java
package zou.lambda;

import java.util.function.Function;

interface Hello {
    void sayHello(String name);
}

public class test1 {
    public static void fun(Hello hello, String name) {
        hello.sayHello(name);
    }
    public static void main(String[] args) {
        fun((String str) -> {
            System.out.println(str + "Hello");
        }, "zouyishan");
    }
}
```





### 无参数有返回值

```java
package zou.lambda;

import java.util.function.Function;

interface Hello {
    int sayHello();
}

public class test1 {
    public static void fun(Hello hello) {
        System.out.println(hello.sayHello());
    }
    public static void main(String[] args) {
        fun(() -> {
            return 10;
        });
    }
}
```



其余多参数同理，这里不做说明



# 函数式接口

## Function

**接收一个参数，返回一个结果**

```java
package zou.function;

import java.util.function.Function;

public class test1 {
    public static void main(String[] args) {
        Function function = name -> name + "zouyishan";
        System.out.println(function.apply("zouyishan"));
    }
}
```



## Predicate

**输入输入参数，返回值为boolean**

```java
package zou.function;

import java.util.function.Predicate;

public class test2 {
    public static void main(String[] args) {
        Predicate predicate = name -> {
            if ("zouyishan".equals(name)) {
                return true;
            } else {
                return false;
            }
        };
        System.out.println(predicate.test("zouyishan"));
    }
}
```



## Consume

**接收一个输入参数，无返回值**

```java
package zou.function;

import java.util.function.Consumer;

public class test3 {
    public static void main(String[] args) {
        Consumer consumer = name -> System.out.println(name + " Hello World");
        consumer.accept("zouyishan");
    }
}
```



## Supplier

**不接收参数，返回一个结果**

```java
package zou.function;

import java.util.function.Supplier;

public class test4 {
    public static void main(String[] args) {
        Supplier supplier = () -> "zouyishan";
        System.out.println(supplier.get());
    }
}
```

其余的不做说明，可参考这个表：

| 序号 | 接口 & 描述                                                  |
| :--- | :----------------------------------------------------------- |
| 1    | **BiConsumer<T,U>**代表了一个接受两个输入参数的操作，并且不返回任何结果 |
| 2    | **BiFunction<T,U,R>**代表了一个接受两个输入参数的方法，并且返回一个结果 |
| 3    | **BinaryOperator<T>**代表了一个作用于于两个同类型操作符的操作，并且返回了操作符同类型的结果 |
| 4    | **BiPredicate<T,U>**代表了一个两个参数的boolean值方法        |
| 5    | **BooleanSupplier**代表了boolean值结果的提供方               |
| 6    | **Consumer<T>**代表了接受一个输入参数并且无返回的操作        |
| 7    | **DoubleBinaryOperator**代表了作用于两个double值操作符的操作，并且返回了一个double值的结果。 |
| 8    | **DoubleConsumer**代表一个接受double值参数的操作，并且不返回结果。 |
| 9    | **DoubleFunction<R>**代表接受一个double值参数的方法，并且返回结果 |
| 10   | **DoublePredicate**代表一个拥有double值参数的boolean值方法   |
| 11   | **DoubleSupplier**代表一个double值结构的提供方               |
| 12   | **DoubleToIntFunction**接受一个double类型输入，返回一个int类型结果。 |
| 13   | **DoubleToLongFunction**接受一个double类型输入，返回一个long类型结果 |
| 14   | **DoubleUnaryOperator**接受一个参数同为类型double,返回值类型也为double 。 |
| 15   | **Function<T,R>**接受一个输入参数，返回一个结果。            |
| 16   | **IntBinaryOperator**接受两个参数同为类型int,返回值类型也为int 。 |
| 17   | **IntConsumer**接受一个int类型的输入参数，无返回值 。        |
| 18   | **IntFunction<R>**接受一个int类型输入参数，返回一个结果 。   |
| 19   | **IntPredicate**：接受一个int输入参数，返回一个布尔值的结果。 |
| 20   | **IntSupplier**无参数，返回一个int类型结果。                 |
| 21   | **IntToDoubleFunction**接受一个int类型输入，返回一个double类型结果 。 |
| 22   | **IntToLongFunction**接受一个int类型输入，返回一个long类型结果。 |
| 23   | **IntUnaryOperator**接受一个参数同为类型int,返回值类型也为int 。 |
| 24   | **LongBinaryOperator**接受两个参数同为类型long,返回值类型也为long。 |
| 25   | **LongConsumer**接受一个long类型的输入参数，无返回值。       |
| 26   | **LongFunction<R>**接受一个long类型输入参数，返回一个结果。  |
| 27   | **LongPredicate**R接受一个long输入参数，返回一个布尔值类型结果。 |
| 28   | **LongSupplier**无参数，返回一个结果long类型的值。           |
| 29   | **LongToDoubleFunction**接受一个long类型输入，返回一个double类型结果。 |
| 30   | **LongToIntFunction**接受一个long类型输入，返回一个int类型结果。 |
| 31   | **LongUnaryOperator**接受一个参数同为类型long,返回值类型也为long。 |
| 32   | **ObjDoubleConsumer<T>**接受一个object类型和一个double类型的输入参数，无返回值。 |
| 33   | **ObjIntConsumer<T>**接受一个object类型和一个int类型的输入参数，无返回值。 |
| 34   | **ObjLongConsumer<T>**接受一个object类型和一个long类型的输入参数，无返回值。 |
| 35   | **Predicate<T>**接受一个输入参数，返回一个布尔值结果。       |
| 36   | **Supplier<T>**无参数，返回一个结果。                        |
| 37   | **ToDoubleBiFunction<T,U>**接受两个输入参数，返回一个double类型结果 |
| 38   | **ToDoubleFunction<T>**接受一个输入参数，返回一个double类型结果 |
| 39   | **ToIntBiFunction<T,U>**接受两个输入参数，返回一个int类型结果。 |
| 40   | **ToIntFunction<T>**接受一个输入参数，返回一个int类型结果。  |
| 41   | **ToLongBiFunction<T,U>**接受两个输入参数，返回一个long类型结果。 |
| 42   | **ToLongFunction<T>**接受一个输入参数，返回一个long类型结果。 |
| 43   | **UnaryOperator<T>**接受一个参数为类型T,返回值类型也为T。    |



# 方法引用



```java
package zou.quote;

import java.util.UUID;
import java.util.function.Supplier;

public class test1 {
    private static UUID hh() {
        return UUID.randomUUID();
    }
    public static void main(String[] args) {
        Supplier supplier = test1::hh;
        System.out.println(supplier.get());
    }
}
```





> 请注意其中的双冒号`::`，这被称为`"方法引用"`，是一种新的语法。
>
> * 方法引用的格式
>   * 符号表示：`::`
>   * 符号说明：双冒号为方法引用的运算符，而它所在的表达式被称为**方法引用。**
>   * 应用场景：如果Lambda所要实现的方案，已经有其他方法存在相同方案，那么则可以使用方法引用

## 引用种类

> 方法引用在JDK8中使用方式相当灵活，有以下几种形式

| name                        | message                        |
| --------------------------- | ------------------------------ |
| instanceName::methodName    | 对象::方法名                   |
| ClassName::staticMethodName | 类名::静态方法                 |
| ClassName::methodName       | 类名::普通方法                 |
| ClassName::new              | 类名::new 调用的构造器         |
| TypeName[]::new             | String[]::new 调用数组的构造器 |



## 对象名::引用成员方法

最常见的一种，如果一个类中已经存在了一个成员方法，则可以通过对象名引用成员方法，代码：

```java
@Test
public void test1() {
    UUID uuid = UUID.randomUUID();
    Supplier<String> supplier = uuid::toString;
    System.out.println(supplier.get());
}
```



## 类名::引用静态方法

```java
@Test
public void test02() {
    Supplier<Long> supp = () -> {
        return System.currentTimeMillis();
    };
    System.out.println(supp.get());
    // 类名::静态方法
    Supplier<Long> supp2 = System::currentTimeMillis;
    System.out.println(supp2.get());
}
```



后面同理，不做演示...........



## 小结

方法引用是对Lambda表达式符合特定情况下的一种缩写，它使得我们的lambda表达式更加精简，也可以理解为Lambda表达式的缩写形式，不过要注意的是方法引用只能"引用"已经存在的方法！


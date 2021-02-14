# Stream流(一)

## 简介

* Stream与IO无任何关系
* Java8中的Stream是对集合(Collection)对象功能的增强，它**专注于对集合对象进行各种非常便利，高效的聚合操作，或者大批量数据操作**
* Stream API借助于同样新出现的Lambda表达式，极大的提高编程效率和程序可读性，同时它提供串行和并行两种汇聚操作，**并发模式能够利用多核处理器优势，使用fork/join并行方式来拆分任务和加速处理过程**
* 通常编写并行代码很难而且容易出错，但是使用Stream API无需编写一行多线程代码，就可以很方便地写出高性能的并发代码。
* Java8中首次出现的java.util.stream是一个函数式语言 + 多核时代总和影响的产物



## 集合处理数据的弊端

当我们需要对集合中的元素进行操作的时候，处理必须的添加，删除，获取外，最典型的就是集合遍历。我们来体验集合操作数据的弊端，需求如下：



> 一个ArrayList集合中存储以下数据：张无忌，周芷若，赵敏，张强，张三丰
>
> 需求：
>
>    1. 拿到所有姓张的
>    2. 拿到名字长度为3个字的
>    3. 打印这些数据



代码：

```java
public class ListTest {
    public static void main(String[] args) {
        // 一个ArrayList集合中存储有以下数据:张无忌,周芷若,赵敏,张强,张三丰
        ArrayList<String> list = new ArrayList<>();
        Collections.addAll(list, "张无忌", "周芷若", "赵敏", "张强", "张三丰");
        // 1.拿到所有姓张的
        ArrayList<String> zhangList = new ArrayList<>(); // {"张无忌", "张强", "张三丰"}
        for (String name : list) {
            if (name.startsWith("张")) {
                zhangList.add(name);
            }
        }
        // 2.拿到名字长度为3个字的
        ArrayList<String> threeList = new ArrayList<>();// {"张无忌", "张三丰"}
        for (String name : zhangList) {
            if (name.length() == 3) {
                threeList.add(name);
            }
        }
        // 3.打印这些数据
        for (String name : threeList) {
            System.out.println(name);
        }
    }
}
```



**弊端**

这段代码中含有三个循环，每一个作用不同：

1. 首先筛选所有姓张的人；
2. 筛选名字有三个字的人；
3. 最后打印输出

每当我们需要对集合的元素进行操作的时候，总是需要进行循环，这是理所当然的吗？**不是。**循环时做事情的方式，而不是目的。每个需求都要循环一次，还要弄一个新的集合来装数据，如果希望再次遍历，只能再使用另一个循环从头开始。那Stream能给我们带来怎么样更优雅的写法呢？

## Stream初体验

```java
package zou.stream;

import java.util.ArrayList;
import java.util.List;

public class test1 {
    public static void main(String[] args) {
        List<String> list = new ArrayList<>();
        list.add("张一");
        list.add("黄琴");
        list.add("邹沂珊");
        list.add("张22");
        list.add("张三");
        list.add("张三丰");

        list.stream()
                .filter(s -> s.startsWith("张"))
                .filter(s -> s.length() == 3)
                .forEach(System.out::println);
    }
}
/**output
 * 张22
 * 张三丰
 */
```



## 获取Stream流

### 1. Collection获取流

首先，`java.util.Collection`接口中加入了`default`方法和`stream`用来获取流，所以其所有实现类均可获取流：

看看源码：

```java
 public interface Collection<E> extends Iterable<E> {   
	default Stream<E> stream() {
        return StreamSupport.stream(spliterator(), false);
    }
}
```

这里可以看到，Collection为我们提供了直接获取Stream的方法，只要是继承了这个接口的方法都能通过此方法获取。

```java
public static void main(String[] args) {
    List<String> list = new ArrayList<>();
    Stream<String> stream1 = list.stream();
    Set<String> set = new HashSet<>();
    Stream<String> stream2 = set.stream();
    Vector<String> vector = new Vector<>();
    Stream<String> stream3 = vector.stream();
    // Map获取流
    Map<String, String> map = new HashMap<>();
    // ...
    Stream<String> keyStream = map.keySet().stream();
    Stream<String> valueStream = map.values().stream();
    Stream<Map.Entry<String, String>> entryStream = map.entrySet().stream();
}
```

### 2. Stream中的静态方法of获取流

由于数组对象不可能添加默认方法，所以Stream接口中提供了静态方法of，使用很简单：

> 备注：of方法的参数其实是一个可变参数，所以支持数组。

```java
public static void main(String[] args) {
        // Stream中的静态方法:           static Stream of(T... values)
        Stream<String> stream6 = Stream.of("aa", "bb", "cc");
        String[] arr = {"aa", "bb", "cc"};
        Stream<String> stream7 = Stream.of(arr);
        Integer[] arr2 = {11, 22, 33};
        Stream<Integer> stream8 = Stream.of(arr2);
        // 注意:基本数据类型的数组不行,如下，它会将整个数组看成一个数据，不是里面的数据
        int[] arr3 = {11, 22, 33};
        Stream<int[]> stream9 = Stream.of(arr3);
}
```

### 3. 其他创建方法

**Stream里提供了一个generate()方法可以创建流**，把Supplier实例传递给Stream.generate()方法会生成Stream。由于这种方法创建的流是无线的，再管道中，`必须利用limit之类的操作限制Stream大小。`

生成10个随机数

```java
package zou.stream;

import java.util.Random;
import java.util.stream.IntStream;

public class test2 {
    public static void main(String[] args) {
        IntStream.generate(new Random()::nextInt).limit(10)
                .forEach(System.out::println);
    }
}
```





## 常用方法，注意事项

**常用方法**

Stream流模型的操作很丰富，这里介绍一些常用的API。这些方法分为两种：

`终结方法：`返回值类型不再是Stream类型的方法，不在支持链式调用，终结方法包括`count`和`forEach`方法

`非终结方法：`返回值类型任然是Stream类型的方法，支持链式调用。(处理终结方法外，其余方法均为非终结方法。)

| 方法名  | 方法作用   | 返回值类型 | 方法种类 |
| ------- | ---------- | ---------- | -------- |
| count   | 统计个数   | long       | 终结     |
| forEach | 逐一处理   | void       | 终结     |
| filter  | 过滤       | Stream     | 函数拼接 |
| limit   | 取用前几个 | Stream     | 函数拼接 |
| skip    | 跳过前几个 | Stream     | 函数拼接 |
| map     | 映射       | Stream     | 函数拼接 |
| concat  | 组合       | Stream     | 函数拼接 |

**Stream注意事项**

1. Stream只能操作一次
2. Stream方法返回的是新的流
3. Stream不调用终结方法，中间的操作不会执行

### forEach方法

`forEach`用来遍历流中的数据

```java
void forEach(Consumer<? super T> action);
```

该方法接受一个Consumer接口函数，会将每一个流元素交给该函数进行处理。例如：

```java
@Test
public  void  testForEach(){
    List<Integer> list = new ArrayList<>();
    list.add(Math.round(2));
    list.add(Math.round(2));
    list.add(Math.round(2));
    list.add(Math.round(2));
    list.add(Math.round(2));
    list.stream().forEach(item->{
        System.out.println(item);
    });
    //简写
    list.stream().forEach(item->System.out.println(item));
    //再次简写
    list.stream().forEach(System.out::println);
}
```

### count方法

Stream流提供`count`方法来统计其中的元素个数：

```java
long count();
```

该方法返回一个long值代表元素个数。基本使用：

```java
@Test
public  void  testCount(){
    List<Integer> list = new ArrayList<>();
    list.add(Math.round(2));
    list.add(Math.round(2));
    list.add(Math.round(2));
    list.add(Math.round(2));
    list.add(Math.round(2));
    System.out.println(list.stream().count());//5
}
```



### filter方法

`filter`用于过滤数据，返回符合过滤条件的数据，可以通过filter方法将一个流转换成另一个子集流。方法声明：

```java
Stream<T> filter(Predicate<? super T> preidcate)
```

该接口接受一个`Predicate`函数时接口参数(可以是一个Lambda或方法引用)作为筛选条件。

Stream流中的filter方法基本使用代码如下：

```java
@Test
public  void  testFilter(){
    List<Integer> list = new ArrayList<>();
    list.add(20);
    list.add(30);
    list.add(1);
    list.add(5);
    list.add(15);
    list.stream().filter(item->item>=10).forEach(System.out::println);//筛选出大于等于10的数值
    /**
        打印结果：
                20
                30
                15
        **/
}
```



### limit方法

`limit`方法可以对流进行截取，只取用前n个。方法签名：

```java
Stream<T> limit(long maxSize);
```

参数是一个long型，如果集合当前长度大于参数则进行截取。否则不进行操作。基本使用：

```java
@Test
public  void  testLimit(){
    List<Integer> list = new ArrayList<>();
    list.add(20);
    list.add(30);
    list.add(1);
    list.add(5);
    list.add(15);
    list.stream().limit(2).forEach(System.out::println);
    /*打印结果：
                    20  
                    30
        */
}
```



### skip方法

如果希望跳过前几个元素，可以使用skip方法获取一个截取之后的新流：

```java
Stream<T> skip(long n)
```

如果流的当前长度大于n，则跳过前n个；否则将会得到一个长度为0的空流。基本使用：

```java
@Test
public void testSkip() {
    List<Integer> list = new ArrayList<>();
    list.add(20);
    list.add(30);
    list.add(1);
    list.add(5);
    list.add(15);
    list.stream().skip(2).forEach(System.out::println);
    /**
        打印结果：
                1
                5
                15
        **/
}
```



# Stream流(二)

## sorted方法

如果需要将数据排序，可以使用sorted方法。方法签名：

```java
Stream<T> sorted();
Stream<T> sorted(Comparator<? super T> comparator);
```

**基本使用**

Stream流中的sorted方法基本使用的代码如下：

```java
@Test
public void testSorted() {
    List<Integer> list = new ArrayList<>();
    list.add(20);
    list.add(30);
    list.add(1);
    list.add(5);
    list.add(15);
    list.stream().sorted().forEach(System.out::println);//按照自然顺序排序
    System.out.println("=====================");
    list.stream().sorted((o1, o2) -> o2 - o1)
        .forEach(System.out::println);//自定义比较器
    /**
         * 打印结果
         1
        ................
         30
         =====================
         30
         ................
         1
         */
}
```



## distinct方法

如果需要去除重复数据，可以使用distinct方法。方法签名：

```java
Stream<T> distinct();
```

**基本使用**

Stream流中的distinct方法基本使用的代码如下：

```java
@Test
public void testDistinct() {
    List<Integer> list = new ArrayList<>();
    list.add(20);
    list.add(20);
    list.add(10);
    list.add(10);
    list.add(5);
    list.stream().distinct()
        .forEach(System.out::println);
    /**
         * 打印结果
         20
         10
         5
         */
}
```



## match方法

如果需要判断数据是否匹配指定的条件，可以使用Match相关方法。方法签名：

```java
boolean allMatch(Predicate<? super T> predicate);// 元素是否全部满足条件
boolean anyMatch(Predicate<? super T> predicate);// 元素是否任意有一个满足条件
boolean noneMatch(Predicate<? super T> predicate);// 元素是否全部不满足条件
```

```java
package zou.stream;

import java.util.stream.Stream;

public class test3 {
    public static void main(String[] args) {
        System.out.println(Stream.of(1, 3, 543, 524, 665, 72, 4312, 34).allMatch(a -> a > 10));
        System.out.println(Stream.of(1, 3, 543, 524, 665, 72, 4312, 34).noneMatch(a -> a > 10));
        System.out.println(Stream.of(1, 3, 543, 524, 665, 72, 4312, 34).anyMatch(a -> a > 10));
    }
}
/**
 * false
 * false
 * true
*/
```



## find方法

如果需要找到某些数据，可以使用find相关方法。方法签名：

```java
Optional<T> findFirst();
Optional<T> findAny();
```

这两个方法一模一样，无区别

**基本使用**

Stream流中的find相关方法基本使用代码如下：

```java
@Test
public void testFind() {
    Optional<Integer> first = Stream.of(5, 3, 6, 1).findFirst();
    System.out.println("first = " + first.get());
    Optional<Integer> any = Stream.of(5, 5, 6, 1).findAny();
    System.out.println("any = " + any.get());
}
/**
打印结果：
        first = 5
        any = 5
**/
```



## max和min方法

需要获取`最大`和`最小`值，可以使用`max`和`min`两种方法，方法签名：

```java
Optional<T> max(Comparator<? super T> comparator); 
Optional<T> min(Comparator<? super T> comparator);
```

这两个方法都可以获取最大值最小值，主要区别在于我们定义比较器，通过我们的比较器排序后获取到第一个值

**基本使用**

Stream流中的`max`和`min`相关方法基本使用代码如下：

```java
@Test
public void testMax_Min() {
    Optional<Integer> max1 = Stream.of(5, 3, 6, 1).max((o1, o2) -> o1 - o2);
    Optional<Integer> max2 = Stream.of(5, 3, 6, 1).min((o1, o2) -> o2 - o1);
    System.out.println("max1 = " + max1.get());
    System.out.println("max2 = " + max2.get());
    Optional<Integer> min1 = Stream.of(5, 3, 6, 1).max((o1, o2) -> o2 - o1);
    Optional<Integer> min2 = Stream.of(5, 3, 6, 1).min((o1, o2) -> o1 - o2);
    System.out.println("min1 = " + min1.get());
    System.out.println("min2 = " + min2.get());
    /**
        max1 = 6
        max2 = 6
        min1 = 1
        min2 = 1
        **/
}
```



## reduce方法

如果需要将所有数据归纳得到一个数据，可以使用reduce方法。方法签名：

```java
T reduce(T identity, BinaryOperator<T> accumulator);//参数数据具体处理方式
T reduce(T identity, BinaryOperator<T> accumulator);//第一个参数是默认值，第二个值是数据具体处理方式
```



## concat方法

如果有两个流，希望合并成为一个流，那么可以使用Stream接口的静态方法concat：

```java
@Test
public void testContact() {
    Stream<String> streamA = Stream.of("张三","李四");
    Stream<String> streamB = Stream.of("王麻子");
    Stream<String> result = Stream.concat(streamA, streamB);
    result.forEach(System.out::println);
}
/**
    打印结果
    **/
```



# Stream流(三)

## 转数组

### 转换成Object数组

```java
@Test
public  void  test2(){
    Stream<String> stream = Stream.of("上单","中单","射手","辅助","打野");
    Object[] objects = stream.toArray();
    Arrays.asList(objects).forEach(item-> System.out.println(item));
}
```

### 转换成指定类型的数组

```java
@Test
public  void  test2(){
    Stream<String> stream = Stream.of("上单","中单","射手","辅助","打野");
    String[] objects = stream.toArray(String[]::new);
    Arrays.asList(objects).forEach(item-> System.out.println(item));
}
```



## 转List集合

### 转成List集合

```java
@Test
public void test3() {
    Stream<String> stream = Stream.of("上单", "中单", "射手", "辅助", "打野");
    List<String> collect = stream.collect(Collectors.toList());
    collect.forEach(System.out::println);
}
```

### 转成指定list类型

```java
@Test
public void test4() {
    Stream<String> stream = Stream.of("上单", "中单", "射手", "辅助", "打野");
    ArrayList<String> collect1 = stream.collect(Collectors.toCollection(ArrayList::new));
    LinkedList<String> collect2 = collect1.stream().collect(Collectors.toCollection(LinkedList::new));
    collect1.forEach(System.out::println);
    collect2.forEach(System.out::println);
}
```



## 转set集合

### 转成set集合

```java
@Test
public void test5() {
    Stream<String> stream = Stream.of("上单", "中单", "射手", "辅助", "打野");
    Set<String> collect = stream.collect(Collectors.toSet());
    collect.forEach(System.out::println);
}
```

### 转成指定set类型

```java
@Test
public void test6() {
    Stream<String> stream = Stream.of("上单", "中单", "射手", "辅助", "打野");
    TreeSet<String> collect1 = stream.collect(Collectors.toCollection(TreeSet::new));
    HashSet<String> collect2 = collect1.stream().collect(Collectors.toCollection(HashSet::new));
    collect1.forEach(System.out::println);
    collect2.forEach(System.out::println);
}
```



## 流中聚合运算

当我们使用Stream流处理数据后，可以像数据库的聚合函数一样对某个字段进行操作。比如获取最大值，最小值，总和，平均值，统计数量。

**实体类**

```java
package zou.pojo;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@AllArgsConstructor
@NoArgsConstructor
public class Student {
    private String name;
    private int age;
    private int score;
}
```

代码：

```java
@Test
public void calculate() {
    //获取分数最大值的user----->第一种
    Student student1 = getStream().collect(Collectors.maxBy((o1, o2) -> o1.getScore() - o2.getScore())).get();
    System.out.println(student1);
    //获取分数最大值的user----->第二种
    Student student2 = getStream().collect(Collectors.minBy((o1, o2) -> o2.getScore() - o1.getScore())).get();
    System.out.println(student2);
    //获取分数最小值的user----->第一种
    Student student3 = getStream().collect(Collectors.maxBy((o1, o2) -> o2.getScore() - o1.getScore())).get();
    System.out.println(student3);
    //获取分数最小值的user----->第二种
    Student student4 = getStream().collect(Collectors.minBy((o1, o2) -> o1.getScore() - o2.getScore())).get();
    System.out.println(student4);
    //求和
    Integer collect = getStream().collect(Collectors.summingInt(t -> t.getScore()));
    System.out.println("总分数：" + collect);
    // 平均值
    Double collect1 = getStream().collect(Collectors.averagingInt(t -> t.getScore()));
    System.out.println("平均数：" + collect1);
    Long collect2 = getStream().collect(Collectors.counting());
    System.out.println("总个数：" + collect2);
} // 数据进行聚合运算
public Stream<Student> getStream() {
    return Stream.of(
        new Student("小明", 58, 50),
        new Student("小红", 56, 0),
        new Student("小刚", 56, 60),
        new Student("笑笑", 52, 100));
}
```



## 数据进行分组

```java
@Test
public void test12() {
    Map<Integer, List<Student>> collect1 = getListStream().collect(Collectors.groupingBy(s -> s.getAge()));
    collect1.forEach((k, v) -> {
        System.out.println(k + "======" + v);
    });
    Map<String, List<Student>> collect2 = getListStream().collect(Collectors.groupingBy(s ->
                                                                                        s.getScore() >= 60 ? "及格" : "不及格"
                                                                                       ));
    collect2.forEach((k, v) -> {
        System.out.println(k + "======" + v);
    });
    /**打印结果
 50======[Student(name=张三, age=50, score=100)]
 100======[Student(name=jack, age=100, score=100), Student(name=mark, age=100, score=100)]
 20======[Student(name=小红, age=20, score=0)]
 70======[Student(name=王五, age=70, score=100)]
 40======[Student(name=admin, age=40, score=100)]
 90======[Student(name=王麻子, age=90, score=100)]
 10======[Student(name=小明, age=10, score=50)]
 60======[Student(name=李四, age=60, score=100)]
 30======[Student(name=小刚, age=30, score=60)]
 不及格======[Student(name=小明, age=10, score=50), Student(name=小红, age=20, score=0)]
 及格======[Student(name=小刚, age=30, score=60), Student(name=admin, age=40, score=100), Student(name=张三, age=50, score=100), Student(name=李四, age=60, score=100), Student(name=王五, age=70, score=100), Student(name=王麻子, age=90, score=100), Student(name=jack, age=100, score=100), Student(name=mark, age=100, score=100)]
 */
}
public Stream<Student> getListStream() {
    return Stream.of(
        new Student("小明", 10, 50),
        new Student("小红", 20, 0),
        new Student("小刚", 30, 60),
        new Student("admin", 40, 100),
        new Student("张三", 50, 100),
        new Student("李四", 60, 100),
        new Student("王五", 70, 100),
        new Student("王麻子", 90, 100),
        new Student("jack", 100, 100),
        new Student("mark", 100, 100)
    );
```

## 分区

`Collectors.partitioningBy`会根据值是否为true，把集合分割为两个列表，一个true列表，一个false列表

```java
// 分区
@Test
public void testPartition() {
    Stream<Student> studentStream = Stream.of(
            new Student("赵丽颖", 52, 95),
            new Student("杨颖", 56, 88),
            new Student("迪丽热巴", 56, 99),
            new Student("柳岩", 52, 77));
    // partitioningBy会根据值是否为true，把集合分割为两个列表，一个true列表，一个false列表。
    Map<Boolean, List<Student>> map = studentStream.collect(Collectors.partitioningBy(s -> s.getScore() > 90));
    map.forEach((k, v) -> {
        System.out.println(k + " == " + v);
    });
}
/**
打印结果：
false == [Student(name=杨颖, age=56, score=88), Student(name=柳岩, age=52, score=77)]
true == [Student(name=赵丽颖, age=52, score=95), Student(name=迪丽热巴, age=56, score=99)]
**/
```



## 小结

收集Stream流中的结果

- 到集合中: Collectors.toList()/Collectors.toSet()/Collectors.toCollection()
- 到数组中: toArray()/toArray(int[]::new)
- 聚合计算:
  - Collectors.maxBy/Collectors.minBy/Collectors.counting/Collectors.summingInt/Collectors.averagingInt
- 分组: Collectors.groupingBy
- 分区: Collectors.partitionBy
- 拼接: Collectors.joinging

# 并行Stream流

### 串行的straem流

我们使用的stream流就是串行，简单的说就是再一条线程上执行的流

```java
@Test
public void  test14(){
    Stream.of(1, 2, 3, 4, 5, 5, 6).filter(t -> {
        System.out.println(Thread.currentThread().getName() + "::" + t);
        return t > 1;
    }).count();
    /**
    打印结果：
     main::1
     main::2
     main::3
     main::4
     main::5
     main::5
     main::6
     */
}
```



### 并行的Stream流

它通过默认的ForkJoinPool，可一提高多线程任务的速度。



### 获取并行流

1. 直接获取并行流：parallelStream()
2. 将串行流转换成并行流：parallel()

```java
@Test
public void testgetParallelStream() {
    ArrayList<Integer> list = new ArrayList<>();
    // 直接获取并行的流
    Stream<Integer> stream1 = list.parallelStream();
    // 将串行流转成并行流
    Stream<Integer> stream2 = list.stream().parallel();
}
```



### 并行操作代码

```java
@Test
public void test0Parallel() {
    // 将流转成并发流,Stream处理的时候将才去
    long count = Stream.of(4, 5, 3, 9, 1, 2, 6).parallel()
        .filter(s -> {
            System.out.println(Thread.currentThread() + ", s = " + s);
            return true;
        }).count();
    System.out.println("count = " + count);
}
/*
Thread[main,5,main], s = 1
Thread[ForkJoinPool.commonPool-worker-6,5,main], s = 9
Thread[ForkJoinPool.commonPool-worker-13,5,main], s = 3
Thread[ForkJoinPool.commonPool-worker-4,5,main], s = 2
Thread[ForkJoinPool.commonPool-worker-2,5,main], s = 6
Thread[ForkJoinPool.commonPool-worker-9,5,main], s = 5
Thread[ForkJoinPool.commonPool-worker-11,5,main], s = 4
count = 7
*/
```



### Stream流的效率对比

使用for循环，串行Stream流，并行Stream流来对5亿数字求和。看消耗时间

```java
public class Demo {
    private static long times = 50000000000L;
    private long start;
    @Before
    public void init() {
        start = System.currentTimeMillis();
    }
    // 测试效率,parallelStream 120
    @Test
    public void parallelStream() {
        System.out.println("serialStream");
        LongStream.rangeClosed(0, times).parallel().reduce(0, Long::sum);
    }
    // 测试效率,普通Stream 342
    @Test
    public void serialStream() {
        System.out.println("serialStream");
        LongStream.rangeClosed(0, times).reduce(0, Long::sum);
    }
    // 测试效率,正常for循环 421
    @Test
    public void forAdd() {
        System.out.println("forAdd");
        long result = 0L;
        for (long i = 1L; i < times; i++) {
            result += i;
        }
    }
    @After
    public void destory() {
        long end = System.currentTimeMillis();
        System.out.println("消耗时间: " + (end - start));
    }
}
```

我们可以看到parallelStream的效率是最高的。

Stream并行处理的过程会分而治之，也就是将一个大任务切分成多个小任务，这表示每个任务都是一个操作。

### 线程安全问题

```java
// parallelStream线程安全问题
@Test
public void parallelStreamNotice() {
    ArrayList<Integer> list = new ArrayList<>();
    /*IntStream.rangeClosed(1, 1000)
            .parallel()
            .forEach(i -> {
                list.add(i);
            });
    System.out.println("list = " + list.size());*/
    // 解决parallelStream线程安全问题方案一: 使用同步代码块
    /*Object obj = new Object();
    IntStream.rangeClosed(1, 1000)
            .parallel()
            .forEach(i -> {
                synchronized (obj) {
                    list.add(i);
                }
            });*/
    // 解决parallelStream线程安全问题方案二: 使用线程安全的集合
    // Vector<Integer> v = new Vector();
    /*List<Integer> synchronizedList = Collections.synchronizedList(list);
    IntStream.rangeClosed(1, 1000)
            .parallel()
            .forEach(i -> {
                synchronizedList.add(i);
            });
    System.out.println("list = " + synchronizedList.size());*/
    // 解决parallelStream线程安全问题方案三: 调用Stream流的collect/toArray
    List<Integer> collect = IntStream.rangeClosed(1, 1000)
            .parallel()
            .boxed()
            .collect(Collectors.toList());
    System.out.println("collect.size = " + collect.size());
}
```

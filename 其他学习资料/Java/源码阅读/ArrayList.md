[先来看看字段值](#先来看看字段值)

[看看构造方法](#看看构造方法)

[看一下常用方法](#看一下常用方法)

[看看get和set方法吧](#看看get和set方法吧)

[看看add方法和其中的扩容机制吧！](#看看add方法和其中的扩容机制吧！)

[看看remove的实现](#看看remove的实现)

# 先来看看字段值

```java
// 默认的初始容量为10
private static final int DEFAULT_CAPACITY = 10;

// 用于空实例的共享空数组实例。
private static final Object[] EMPTY_ELEMENTDATA = {};

// 用于默认大小的空实例的共享空数组实例。我们将其与EMPTY_ELEMENTDATA区分开来，以便知道添加第一个元素时要增加多少
private static final Object[] DEFAULTCAPACITY_EMPTY_ELEMENTDATA = {};

// 存储数组列表元素的数组缓冲区。ArrayList的容量就是这个数组缓冲区的长度。当添加第一个元素时，任何带有elementData == DEFAULTCAPACITY_EMPTY_ELEMENTDATA的空数组列表将扩展为DEFAULT_CAPACITY。
transient Object[] elementData; // 非私有，以简化嵌套类访问

// ArrayList的长度
private int size;
```



# 看看构造方法

```java
public ArrayList(int initialCapacity) {
        if (initialCapacity > 0) {
            // 大于0的话就直接new空间
            this.elementData = new Object[initialCapacity];
        } else if (initialCapacity == 0) {
            // 是0的话给个空数组
            this.elementData = EMPTY_ELEMENTDATA;
        } else {
            // 小于0抛出异常
            throw new IllegalArgumentException("Illegal Capacity: "+
                                               initialCapacity);
        }
    }

    public ArrayList() {
        // 注意这里并没有new对象，相较于单例模式是属于懒汉式。
        this.elementData = DEFAULTCAPACITY_EMPTY_ELEMENTDATA;
    }

    public ArrayList(Collection<? extends E> c) { // 
        elementData = c.toArray();
        if ((size = elementData.length) != 0) {
            // c.toArray might (incorrectly) not return Object[] (see 6260652)
            if (elementData.getClass() != Object[].class)
                elementData = Arrays.copyOf(elementData, size, Object[].class);
        } else {
            // replace with empty array.
            this.elementData = EMPTY_ELEMENTDATA;
        }
    }
```



# 看一下常用方法

```java
// 返回长度
public int size() {
    return size;
}

// 返回这个ArrayList是否为空
public boolean isEmpty() {
    return size == 0;
}

// 查看是否包含o元素，用到下面的indexOF方法
public boolean contains(Object o) {
    return indexOf(o) >= 0;
}

// 通过遍历寻找元素o
public int indexOf(Object o) {
    if (o == null) {
        for (int i = 0; i < size; i++)
            if (elementData[i]==null)
                return i;
    } else {
        for (int i = 0; i < size; i++)
            if (o.equals(elementData[i]))
                return i;
    }
    return -1;
}

// 调用的是Arrays工具类的方法
public Object[] toArray() {
        return Arrays.copyOf(elementData, size);
    }

```



# 看看get和set方法吧

```java
// 找到指定索引的元素，再次之前要调用rangeCheck方法。检查是否越界
public E get(int index) {
    rangeCheck(index);
    return elementData(index);
}
// 检查是否越界，越界就抛出outOfBoundsMsg异常。
private void rangeCheck(int index) {
        if (index >= size)
            throw new IndexOutOfBoundsException(outOfBoundsMsg(index));
    }

// 同样，先检查是否越界，然后直接复制就行了。注意这里返回值是oldValue
public E set(int index, E element) {
        rangeCheck(index);
        E oldValue = elementData(index);
        elementData[index] = element;
        return oldValue;
    }
```





# 看看add方法和其中的扩容机制吧！

```java
public boolean add(E e) {
    ensureCapacityInternal(size + 1);  // Increments modCount!!
    elementData[size++] = e;
    return true;
}
private void ensureCapacityInternal(int minCapacity) {
    ensureExplicitCapacity(calculateCapacity(elementData, minCapacity));
}
// 获取默认的容量和传入参数的较大值
private static int calculateCapacity(Object[] elementData, int minCapacity) {
    if (elementData == DEFAULTCAPACITY_EMPTY_ELEMENTDATA) {
        return Math.max(DEFAULT_CAPACITY, minCapacity);
    }
    return minCapacity;
}

// 判断是否需要扩容
private void ensureExplicitCapacity(int minCapacity) {
    modCount++;
    if (minCapacity - elementData.length > 0) // 满足这个条件就进行扩容
    	grow(minCapacity);
}

private void grow(int minCapacity) {
    // overflow-conscious code
    int oldCapacity = elementData.length;
    // 可以看成新容量是旧容量的1.5倍
    int newCapacity = oldCapacity + (oldCapacity >> 1);
    //然后检查新容量是否大于最小需要容量，若还是小于最小需要容量，那么就把最小需要容量当作数组的新容量
    if (newCapacity - minCapacity < 0)
        newCapacity = minCapacity;
    
    // 如果新容量大于 MAX_ARRAY_SIZE,进入(执行) `hugeCapacity()` 方法来比较 minCapacity 和 MAX_ARRAY_SIZE，
    //如果minCapacity大于最大容量，则新容量则为`Integer.MAX_VALUE`，否则，新容量大小则为 MAX_ARRAY_SIZE 即为 `Integer.MAX_VALUE - 8`。
    if (newCapacity - MAX_ARRAY_SIZE > 0)
        newCapacity = hugeCapacity(minCapacity);
    // minCapacity is usually close to size, so this is a win:
    elementData = Arrays.copyOf(elementData, newCapacity);
}

//如果minCapacity大于最大容量，则新容量则为`Integer.MAX_VALUE`，否则，新容量大小则为 MAX_ARRAY_SIZE 即为 `Integer.MAX_VALUE - 8`。
private static int hugeCapacity(int minCapacity) {
    if (minCapacity < 0) // overflow
        throw new OutOfMemoryError();
    return (minCapacity > MAX_ARRAY_SIZE) ?
        Integer.MAX_VALUE :
    MAX_ARRAY_SIZE;
}
```



## 看一下带下标的add方法

```java
// 某个位置添加元素
public void add(int index, E element) {
    // 检查上下溢出
    rangeCheckForAdd(index);
	// 检查是否需要扩容
    ensureCapacityInternal(size + 1);  // Increments modCount!!
    
    // elementData:源数组;index:源数组中的起始位置;elementData：目标数组；index + 1：目标数组中的起始位置； size - index：要复制的数组元素的数量；
    System.arraycopy(elementData, index, elementData, index + 1,
                     size - index);
    elementData[index] = element;
    size++;
}

// 检查index是否发生上下溢出
private void rangeCheckForAdd(int index) {
    if (index > size || index < 0)
        throw new IndexOutOfBoundsException(outOfBoundsMsg(index));
}
```



# 看看remove的实现

```java
public boolean remove(Object o) {
    if (o == null) {
        // 遍历寻找
        for (int index = 0; index < size; index++)
            if (elementData[index] == null) {
                fastRemove(index);
                return true;
            }
    } else {
        // 遍历寻找
        for (int index = 0; index < size; index++)
            if (o.equals(elementData[index])) {
                fastRemove(index);
                return true;
            }
    }
    return false;
}

// 通过System.arraycopy实现快速移除，因为此方法为native方法！所以性能较快
private void fastRemove(int index) {
    modCount++;
    int numMoved = size - index - 1;
    if (numMoved > 0)
        System.arraycopy(elementData, index+1, elementData, index,
                         numMoved);
    elementData[--size] = null; // 让GC去回收内存
}

public void clear() {
    modCount++;
    // 全部设置为null，让GC线程去处理垃圾
    for (int i = 0; i < size; i++)
        elementData[i] = null;
    size = 0;
}
```


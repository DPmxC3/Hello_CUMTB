[简介](#简介)

[内部结构](#内部结构)

[构造方法](#构造方法)

[链表节点的增删](#链表节点的增删)

[获取节点的值和位置](#获取节点的值和位置)

&nbsp;

# 简介

LinkedList是一个实现了List接口和Deque接口的双端链表。 LinkedList底层的链表结构使它支持高效的插入和删除操作，另外它实现了Deque接口，使得LinkedList类也具有队列的特性; LinkedList不是线程安全的。

```java
public class LinkedList<E>
    extends AbstractSequentialList<E>
    implements List<E>, Deque<E>, Cloneable, java.io.Serializable
```

# 内部结构

如图：

![image-20201227191035502](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201227200743.png)

其构造类如下：

```java
private static class Node<E> {
    E item;
    Node<E> next;
    Node<E> prev;

    Node(Node<E> prev, E element, Node<E> next) {
        this.item = element;
        this.next = next;
        this.prev = prev;
    }
}
```

这个类就代表双端链表的节点Node。这个类有三个属性，分别是前驱节点，本节点的值，后继结点。



# 构造方法



```java
public LinkedList() {
}

// 在后面连接集合c
public LinkedList(Collection<? extends E> c) {
    this();
    addAll(c);
}
// 将集合c连接到size后面。
public boolean addAll(Collection<? extends E> c) {
    return addAll(size, c);
}


public boolean addAll(int index, Collection<? extends E> c) {
    // 检测index是否越界
    checkPositionIndex(index);
    
 	// toArray()方法把集合的数据都存到对象数组中
    Object[] a = c.toArray();
    int numNew = a.length;
    if (numNew == 0)
        return false;
	
    // 得到插入的前驱和后继结点
    Node<E> pred, succ;
    // 如果插入位置为尾部，前驱结点为last，后继节点为null
    if (index == size) {
        succ = null;
        pred = last;
    } else { // 否则调用node()方法得到后继节点，在得到前驱节点
        succ = node(index);
        pred = succ.prev;
    }

    // 遍历数据插入
    for (Object o : a) {
        @SuppressWarnings("unchecked") E e = (E) o;
        // 创建新节点
        Node<E> newNode = new Node<>(pred, e, null);
        // 如果插入位置是链表头部
        if (pred == null)
            first = newNode;
        else
            pred.next = newNode;
        pred = newNode;
    }
	
    // 如果插入位置在尾部，就将last节点指向pred，这时的pred节点是最新创建的节点
    if (succ == null) {
        last = pred;
    } else { // 将其和后半部分连接起来
        pred.next = succ;
        succ.prev = pred;
    }
	
    // 这里的numNew为a对象数组的长度
    size += numNew;
    modCount++;
    return true;
}

// 为上文检查index范围的代码
private void checkElementIndex(int index) {
    if (!isElementIndex(index))
        throw new IndexOutOfBoundsException(outOfBoundsMsg(index));
}
private boolean isElementIndex(int index) {
    return index >= 0 && index < size;
}

// 这里为查找index所在位置的后继节点的代码：
// 细细一看原来这里还用了二分的思想，又快了点
Node<E> node(int index) {
    if (index < (size >> 1)) {
        Node<E> x = first;
        for (int i = 0; i < index; i++)
            x = x.next;
        return x;
    } else {
        Node<E> x = last;
        for (int i = size - 1; i > index; i--)
            x = x.prev;
        return x;
    }
}
```



# 链表节点的增删



```java
// 在头部添加节点
public void addFirst(E e) {
    linkFirst(e);
}
// 在链表头插入节点
private void linkFirst(E e) {
    final Node<E> f = first;
    final Node<E> newNode = new Node<>(null, e, f);
    first = newNode;
    if (f == null)
        last = newNode;
    else
        f.prev = newNode;
    size++;
    modCount++;
}



// 在尾部添加节点
public void addLast(E e) {
    linkLast(e);
}
// 在链表的尾部插入节点
void linkLast(E e) {
    final Node<E> l = last;
    final Node<E> newNode = new Node<>(l, e, null);
    last = newNode;
    if (l == null)
        first = newNode;
    else
        l.next = newNode;
    size++;
    modCount++;
}



// 在index处添加节点
public void add(int index, E element) {
    // 检查index的范围
    checkPositionIndex(index);
    if (index == size)
        linkLast(element);
    else // node为查找index位置的后驱节点
        linkBefore(element, node(index));
}
// 在节点succ前面插入节点
void linkBefore(E e, Node<E> succ) {
    // assert succ != null;
    final Node<E> pred = succ.prev;
    final Node<E> newNode = new Node<>(pred, e, succ);
    succ.prev = newNode;
    if (pred == null)
        first = newNode;
    else
        pred.next = newNode;
    size++;
    modCount++;
}



// 默认的remove为删除头节点
public E remove() {
    return removeFirst();
}
// 这里的pop也是删除头节点
public E pop() {
    return removeFirst();
}
public E removeFirst() {
    final Node<E> f = first;
    if (f == null)
        throw new NoSuchElementException();
    return unlinkFirst(f);
}


public E removeLast() {
    final Node<E> l = last;
    if (l == null)
        throw new NoSuchElementException();
    return unlinkLast(l);
}
// f为头节点，目的是删除头节点
private E unlinkFirst(Node<E> f) {
    // assert f == first && f != null;
    final E element = f.item;
    final Node<E> next = f.next;
    f.item = null;
    f.next = null; // help GC
    first = next;
    if (next == null)
        last = null;
    else
        next.prev = null;
    size--;
    modCount++;
    return element;
}

// 删除尾节点，l是尾节点
private E unlinkLast(Node<E> l) {
    // assert l == last && l != null;
    final E element = l.item;
    final Node<E> prev = l.prev;
    l.item = null;
    l.prev = null; // help GC
    last = prev;
    if (prev == null)
        first = null;
    else
        prev.next = null;
    size--;
    modCount++;
    return element;
}



// 删除对象为o的节点；
public boolean remove(Object o) {
    if (o == null) {
        for (Node<E> x = first; x != null; x = x.next) {
            if (x.item == null) {
                unlink(x);
                return true;
            }
        }
    } else {
        for (Node<E> x = first; x != null; x = x.next) {
            if (o.equals(x.item)) {
                unlink(x);
                return true;
            }
        }
    }
    return false;
}
// 删除节点x
E unlink(Node<E> x) {
    // assert x != null;
    final E element = x.item;
    final Node<E> next = x.next;
    final Node<E> prev = x.prev;

    if (prev == null) {
        first = next;
    } else {
        prev.next = next;
        x.prev = null;
    }

    if (next == null) {
        last = prev;
    } else {
        next.prev = prev;
        x.next = null;
    }

    x.item = null;
    size--;
    modCount++;
    return element;
}
```

## clear方法

```java
// 全部丢给GC做，有点东西 √
public void clear() {
    // Clearing all of the links between nodes is "unnecessary", but:
    // - helps a generational GC if the discarded nodes inhabit
    //   more than one generation
    // - is sure to free memory even if there is a reachable Iterator
    for (Node<E> x = first; x != null; ) {
        Node<E> next = x.next;
        x.item = null;
        x.next = null;
        x.prev = null;
        x = next;
    }
    first = last = null;
    size = 0;
    modCount++;
}
```

# 获取节点的值和位置



```java
// 通过索引获取值，这个node方法前面讲过
public E get(int index) {
    // 检查index的范围
    checkElementIndex(index);
    return node(index).item;
}



// 通过传入对象，查询所对应的索引
public int indexOf(Object o) {
    int index = 0;
    if (o == null) {
        // 从前到后遍历
        for (Node<E> x = first; x != null; x = x.next) {
            if (x.item == null)
                return index;
            index++;
        }
    } else {
        // 从前到后遍历，对比的只是item这个值。
        for (Node<E> x = first; x != null; x = x.next) {
            if (o.equals(x.item))
                return index;
            index++;
        }
    }
    return -1;
}



// 和IndexOf不同的区别是从尾节点到头节点
public int lastIndexOf(Object o) {
    int index = size;
    if (o == null) {
        for (Node<E> x = last; x != null; x = x.prev) {
            index--;
            if (x.item == null)
                return index;
        }
    } else {
        for (Node<E> x = last; x != null; x = x.prev) {
            index--;
            if (o.equals(x.item))
                return index;
        }
    }
    return -1;
}
```
&nbsp;
&nbsp;

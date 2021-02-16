# 从fork开始说起

不了解fork源码可以看看:http://www.zouyishan.com/read/49a3583a9d484f929818554621b11b46

从前面提到的fork开始说起：fork() -> sys_fork -> copy_process。看看copy_process中的一段源码，在`kernel/fork.c`：

```c
int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
		long ebx,long ecx,long edx,
		long fs,long es,long ds,
		long eip,long cs,long eflags,long esp,long ss)
{
    ................
   // 接下来复制进程页表。即在线性地址空间中设置新任务代码段和数据段描述符中的基址和限长，
   // 并复制页表。如果出错(返回值不是0)，则复位任务数组中相应项并释放为该新任务分配的用于
   // 任务结构的内存页。
    if (copy_mem(nr,p)) {
       task[nr] = NULL;
       free_page((long) p);
       return -EAGAIN;
    }
    ................
}
```



由此跳到相应的copy_mem中：

```c
int copy_mem(int nr,struct task_struct * p)
{
	.............
	new_data_base = new_code_base = nr * 0x4000000; // nr为任务号，
	p->start_code = new_code_base;
	set_base(p->ldt[1],new_code_base); // 做段表，所以进程切换的时候，段表也会相应的切换
	set_base(p->ldt[2],new_data_base);
    // 由此可以发现，每个进程为64MB。
	...............
	return 0;
}
```



所以内存在我们脑海中应该有这个图：

![image-20210216171253901](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210216203413.png)

* 每个进程的代码段，数据段都是一个段
* 每个进程占64MB虚拟地址空间，互不重叠

> b站老师说这样很弱智,太难了=_=



# 分配内存，建页表

```c
int copy_mem(int nr,struct task_struct * p)
{
	unsigned long old_data_base,new_data_base,data_limit;
	unsigned long old_code_base,new_code_base,code_limit;	

	if (copy_page_tables(old_data_base,new_data_base,data_limit)) { // 因为是fork出来的，父进程和子进程都指向相同的物理地址，所以只需要拷贝这两个虚拟地址
		printk("free_page_tables: from copy_mem\n");
		free_page_tables(new_data_base,data_limit);
		return -ENOMEM;
	}
}
```

先看看页表的结构：

![image-20210216181817015](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210216203417.png)

页目录号表示逻辑页的目录号，页号表示物理页的页号，然后offset就表示从开始的偏移。所以我们就要修改的是`页目录号`。

`copy_page_tables`源码在`mm/memory.c`中：

```c
int copy_page_tables(unsigned long from,unsigned long to,long size)
{
   ......
   // form是32位虚拟地址，
   from_dir = (unsigned long *) ((from>>20) & 0xffc); 
   to_dir = (unsigned long *) ((to>>20) & 0xffc);
   size = ((unsigned) (size+0x3fffff)) >> 22;
   ......
}
```

那按照上面所说，那么应该向右偏移22位才能得到页目录号啊。

**在前面讲过，一个项是4字节。所以实际应该偏移的是(from >> 22) * 4，对应的就是`from_dir = (unsigned long *) ((from>>20) & 0xffc);`,然后在通过&清空后面的位数**

## from_page_table与to_page_table

继续看`copy_page_tables`，这里的get_free_page用于是一段汇编代码，用于找到一块空闲的物理内存：

```c
for( ; size-- > 0 ; from_dir++,to_dir++) {
    to_page_table=get_free_page(); // 申请一个新的物理页用于映射
    *to_dir=((unsigned long)to_page_table)|7;
}

// mm/memory.c中
unsigned long get_free_page(void)
{
register unsigned long __res asm("ax");

__asm__("std ; repne ; scasb\n\t"   // 置方向位，al(0)与对应每个页面的(di)内容比较
	"jne 1f\n\t"                    // 如果没有等于0的字节，则跳转结束(返回0).
	"movb $1,1(%%edi)\n\t"          // 1 => [1+edi],将对应页面内存映像bit位置1.
	"sall $12,%%ecx\n\t"            // 页面数*4k = 相对页面其实地址
	"addl %2,%%ecx\n\t"             // 再加上低端内存地址，得页面实际物理起始地址
	"movl %%ecx,%%edx\n\t"          // 将页面实际其实地址->edx寄存器。
	"movl $1024,%%ecx\n\t"          // 寄存器ecx置计数值1024
	"leal 4092(%%edx),%%edi\n\t"    // 将4092+edx的位置->dei（该页面的末端地址）
	"rep ; stosl\n\t"               // 将edi所指内存清零(反方向，即将该页面清零)
	"movl %%edx,%%eax\n"            // 将页面起始地址->eax（返回值）
	"1:"
	:"=a" (__res)
	:"0" (0),"i" (LOW_MEM),"c" (PAGING_PAGES),
	"D" (mem_map+PAGING_PAGES-1)
	);
return __res;           // 返回空闲物理页面地址(若无空闲页面则返回0).
}
```

然后，我们申请到了物理内存以后，要做的就很简单，直接拷贝相应的数据就行,这样就实现**父进程指向的物理页，子进程页指向**：

![image-20210216194452855](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210216203423.png)

看看代码：

```c
for(;nr-->0;from_page_table++,to_page_table++){
    this_page = *from_page_table;
    this_page&=~2;//只读
    *to_page_table=this_page;
    *from_page_table=this_page;
    this_page -= LOW_MEM; this_page >>= 12;
    mem_mep[this_page]++;
}
```

父子进程都指向同一个物理页，所以子进程要成为只读的，所以有这个`this_page&=~2`也叫copy on write写时复制

到这时内存应该时这个样子：

![image-20210216200119065](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210216203413.png)



**`写时复制`**: 就是当一个进程想要改变其中的值的时候，新申请一个内存页，然后修改页表，建立一个新的映射。如图：

![image-20210216200239621](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210216203429.png)



到这里就实现了可以通过变量来使用内存了！




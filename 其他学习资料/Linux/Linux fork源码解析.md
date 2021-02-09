

# linux0.11源码分析-fork进程

操作系统首先在main.c中：

```c
if (!fork()) {
   init();
}
```

这个init用于启动shell，让一个shell让您执行。

fork函数先从当前任务表（task）里找到一个任务号（进程pid），如果可以找到，就会复制当前进程`current`结构体的数据(task_struct),然后复制进程页表项，将RW置位0，为以后写时复制做准备。子进程与父进程共享内存。然后处理信号。切换进程后，CPU会自动的加载每个 task_struct中的TSS数据，并且保存前一个进程的CPU状态到TSS中。进程fork后，就等着调度了。注意，子进程在初始化的时候往eax寄存器中存进去了0，eax用于函数返回值。也就是说子进程会返回0，而父进程会返回自己的pid。



在`init/main.c`中：

```c
// 下面_syscall0()是unistd.h中的内嵌宏代码。以嵌入汇编的形式调用Linux的系统调用中断
// 0x80.该中断是所有系统调用的入口。该条语句实际上是int fork()创建进程系统调用。可展
// 开看之就会立刻明白。syscall0名称中最后的0表示无参数，1表示1个参数。
static inline _syscall0(int,fork)
```

在`include/unistd.h`中:

```asm
#define __NR_fork	2

#define _syscall0(type,name) \
type name(void) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
   : "=a" (__res) \
   : "0" (__NR_##name)); \
if (__res >= 0) \
   return (type) __res; \
errno = -__res; \
return -1; \
}
```

宏展开后就是：

```c
int fork(void)
{
    long __res;
    __asm__ volatile ("int $0x80" \
        : "=a" (__res) \
        : "0" (__NR_fork)); \
    if (__res >= 0) \
        return (int) __res; \
    errno = -__res; \
    return -1; \
}
```

`"0" (__NR_fork));`表示沿用上一次的约束，就是把`__NR_fork`的值2放到`eax`寄存器中。`"=a"(__res)`表示的是`_res`与`eax`绑定。函数的返回结果会在`eax`里面，也就是在`_res`里。

使用`int 0x80`中断后就会调用`system_call`函数，然后`system_call`会根据传递进来的函数索引从系统调用表`sys_call_table`中找到对应的函数，从而执行。

&nbsp;

所以执行fork函数就会执行`system_call`函数，但是再这之前，还有些事情要做，就是保存现场。下面是操作系统执行系统调用前，在内核栈里保存的寄存器，这个压入的寄存器和iret中断返回指令出战的寄存器是对应的。其中ip指向的是调用系统该调用返回后的下一句代码。

![image-20210209141558792](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210209150422.png)



在`kernel/sched.c`在`sched.c`文件中设置了0x80号中断的处理程序。

```c
// 内核调度程序的初始化子程序
void sched_init(void)
{
   .............
   set_system_gate(0x80,&system_call);
}
```

`set_system_gate`是个宏，include/asm/system.h中定义为：

```asm
#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), \
	"o" (*(4+(char *) (gate_addr))), \
	"d" ((char *) (addr)),"a" (0x00080000))
	
#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)
```

填写IDT,将`system_call`函数地址写到`0x80`对应的中断描述符中,当发生中断`0x80`后,调用`system_call`函数

在`kernel/system_call.s`中：

```asm
# 错误的系统调用号
.align 2                # 内存4字节对齐
bad_sys_call:
	movl $-1,%eax       # eax 中置-1，退出中断
	iret
# 重新执行调度程序入口。调度程序schedule在sched.c中。
# 当调度程序schedule返回时就从ret_from_sys_call出继续执行。
.align 2
reschedule:
	pushl $ret_from_sys_call        # 将ret_from_sys_call返回地址压入堆栈
	jmp schedule
### int 0x80 - linux系统调用入口点(调用中断int 0x80,eax 中是调用号)
.align 2
system_call:
	cmpl $nr_system_calls-1,%eax    # 调用号如果超出范围的话就在eax中置-1并退出
	ja bad_sys_call
	push %ds                        # 保存原段寄存器值
	push %es
	push %fs
# 一个系统调用最多可带有3个参数，也可以不带参数。下面入栈的ebx、ecx和edx中放着系统
# 调用相应C语言函数的调用函数。这几个寄存器入栈的顺序是由GNU GCC规定的，
# ebx 中可存放第1个参数，ecx中存放第2个参数，edx中存放第3个参数。
# 系统调用语句可参见头文件include/unistd.h中的系统调用宏。
	pushl %edx
	pushl %ecx		# push %ebx,%ecx,%edx as parameters
	pushl %ebx		# to the system call
	movl $0x10,%edx		# set up ds,es to kernel space
	mov %dx,%ds
	mov %dx,%es
# fs指向局部数据段(局部描述符表中数据段描述符)，即指向执行本次系统调用的用户程序的数据段。
# 注意,在Linux 0.11 中内核给任务分配的代码和数据内存段是重叠的，他们的段基址和段限长相同。
	movl $0x17,%edx		# fs points to local data space
	mov %dx,%fs
# 下面这句操作数的含义是：调用地址=[_sys_call_table + %eax * 4]
# sys_call_table[]是一个指针数组，定义在include/linux/sys.h中，该指针数组中设置了所有72
# 个系统调用C处理函数地址。
	call sys_call_table(,%eax,4)        # 间接调用指定功能C函数
	pushl %eax                          # 把系统调用返回值入栈
# 下面几行查看当前任务的运行状态。如果不在就绪状态(state != 0)就去执行调度程序。如果该
# 任务在就绪状态，但其时间片已用完(counter = 0),则也去执行调度程序。例如当后台进程组中的
# 进程执行控制终端读写操作时，那么默认条件下该后台进程组所有进程会收到SIGTTIN或SIGTTOU
# 信号，导致进程组中所有进程处于停止状态。而当前进程则会立刻返回。
	movl current,%eax                   # 取当前任务(进程)数据结构地址→eax
	cmpl $0,state(%eax)		# state
	jne reschedule
	cmpl $0,counter(%eax)		# counter
	je reschedule

# 以下这段代码执行从系统调用C函数返回后，对信号进行识别处理。其他中断服务程序退出时也
# 将跳转到这里进行处理后才退出中断过程，例如后面的处理器出错中断int 16.
ret_from_sys_call:
# 首先判别当前任务是否是初始任务task0,如果是则不比对其进行信号量方面的处理，直接返回。
	movl current,%eax		# task[0] cannot have signals
	cmpl task,%eax
	je 3f                   # 向前(forward)跳转到标号3处退出中断处理
# 通过对原调用程序代码选择符的检查来判断调用程序是否是用户任务。如果不是则直接退出中断。
# 这是因为任务在内核态执行时不可抢占。否则对任务进行信号量的识别处理。这里比较选择符是否
# 为用户代码段的选择符0x000f(RPL=3,局部表，第一个段(代码段))来判断是否为用户任务。如果不是
# 则说明是某个中断服务程序跳转到上面的，于是跳转退出中断程序。如果原堆栈段选择符不为
# 0x17(即原堆栈不在用户段中)，也说明本次系统调用的调用者不是用户任务，则也退出。
	cmpw $0x0f,CS(%esp)		# was old code segment supervisor ?
	jne 3f
	cmpw $0x17,OLDSS(%esp)		# was stack segment = 0x17 ?
	jne 3f
# 下面这段代码用于处理当前任务中的信号。首先取当前任务结构中的信号位图(32位，每位代表1种
# 信号)，然后用任务结构中的信号阻塞(屏蔽)码，阻塞不允许的信号位，取得数值最小的信号值，
# 再把原信号位图中该信号对应的位复位(置0)，最后将该信号值作为参数之一调用do_signal().
# do_signal()在kernel/signal.c中，其参数包括13个入栈信息。
	movl signal(%eax),%ebx          # 取信号位图→ebx,每1位代表1种信号，共32个信号
	movl blocked(%eax),%ecx         # 取阻塞(屏蔽)信号位图→ecx
	notl %ecx                       # 每位取反
	andl %ebx,%ecx                  # 获得许可信号位图
	bsfl %ecx,%ecx                  # 从低位(位0)开始扫描位图，看是否有1的位，若有，则ecx保留该位的偏移值
	je 3f                           # 如果没有信号则向前跳转退出
	btrl %ecx,%ebx                  # 复位该信号(ebx含有原signal位图)
	movl %ebx,signal(%eax)          # 重新保存signal位图信息→current->signal.
	incl %ecx                       # 将信号调整为从1开始的数(1-32)
	pushl %ecx                      # 信号值入栈作为调用do_signal的参数之一
	call do_signal                  # 调用C函数信号处理程序(kernel/signal.c)
	popl %eax                       # 弹出入栈的信号值
3:	popl %eax                       # eax中含有上面入栈系统调用的返回值
	popl %ebx
	popl %ecx
	popl %edx
	pop %fs
	pop %es
	pop %ds
	iret
```

首先看到call _sys_call_table(,%eax,4)这一句。这时候内核栈是：

![image-20210209142442071](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210209150415.png)

因为是段内跳转，所以cs不需要入栈。ip指向`call _sys_call_table(,%eax,4)`下面一句代码。我们首先进入到call _sys_call_table(,%eax,4)里面去。在`includ/linux/sys.h`中：

```c
fn_ptr sys_call_table[] = { sys_setup, sys_exit, sys_fork, sys_read,
sys_write, sys_open, sys_close, sys_waitpid, sys_creat, sys_link,
sys_unlink, sys_execve, sys_chdir, sys_time, sys_mknod, sys_chmod,
sys_chown, sys_break, sys_stat, sys_lseek, sys_getpid, sys_mount,
sys_umount, sys_setuid, sys_getuid, sys_stime, sys_ptrace, sys_alarm,
sys_fstat, sys_pause, sys_utime, sys_stty, sys_gtty, sys_access,
sys_nice, sys_ftime, sys_sync, sys_kill, sys_rename, sys_mkdir,
sys_rmdir, sys_dup, sys_pipe, sys_times, sys_prof, sys_brk, sys_setgid,
sys_getgid, sys_signal, sys_geteuid, sys_getegid, sys_acct, sys_phys,
sys_lock, sys_ioctl, sys_fcntl, sys_mpx, sys_setpgid, sys_ulimit,
sys_uname, sys_umask, sys_chroot, sys_ustat, sys_dup2, sys_getppid,
sys_getpgrp, sys_setsid, sys_sigaction, sys_sgetmask, sys_ssetmask,
sys_setreuid,sys_setregid };
```

其实是一个数组，根据eax即系统函数的编号找到对应的函数执行。前面说过`__NR_fork`的值2放到`eax`寄存器中。所以这里我们找的是`sys_fork`。所以我们继续看`sys_fork`函数代码。

在`kernel/system_call.s`中。

```asm
.align 2
sys_fork:
   call find_empty_process		# 执行find_empty_process函数，返回一个进程id在eax里
   testl %eax,%eax             # 在eax中返回进程号pid。若返回负数则退出。
   js 1f						# 没有找到就返回
   push %gs						# 找到则压栈寄存器
   pushl %esi
   pushl %edi
   pushl %ebp
   pushl %eax					# 找到的进程id
   call copy_process			# 继续掉函数
   addl $20,%esp               # 出栈上面压进栈的五个寄存器，然后返回
1: ret
```

`sys_fork`先找`find_empty_process`函数找到一个可用的进程号。在`kernel/fork.c`中，代码如下：

```c
// 为新进程取得不重复的进程号last_pid.函数返回在任务数组中的任务号(数组项)。
int find_empty_process(void)
{
	int i;

    // 首先获取新的进程号。如果last_pid增1后超出进程号的整数表示范围，则重新从1开始
    // 使用pid号。然后在任务数组中搜索刚设置的pid号是否已经被任何任务使用。如果是则
    // 跳转到函数开始出重新获得一个pid号。接着在任务数组中为新任务寻找一个空闲项，并
    // 返回项号。last_pid是一个全局变量，不用返回。如果此时任务数组中64个项已经被全部
    // 占用，则返回出错码。
	repeat:
    	// 先找到一个可用的pid
		if ((++last_pid)<0) last_pid=1;
		for(i=0 ; i<NR_TASKS ; i++)
			if (task[i] && task[i]->pid == last_pid) goto repeat;
    // 在找一个pcb项，从1开始，0是init进程
	for(i=1 ; i<NR_TASKS ; i++)         // 任务0项被排除在外
		if (!task[i])
			return i;
	return -EAGAIN;
}
```

找到后把pid放在`eax`里,返回sys_fork。根据sys_fork的代码，我们看到继续压栈寄存器然后执行`copy_process`函数。这时候内核栈是：

![未命名文件](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210209150410.png)



然后看`copy_process`函数的代码。在`kernel/fork.c`中:

```c
int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
      long ebx,long ecx,long edx,
      long fs,long es,long ds,
      long eip,long cs,long eflags,long esp,long ss)
{
   struct task_struct *p;
   int i;
   struct file *f;

    // 首先为新任务数据结构分配内存。如果内存分配出错，则返回出错码并退出。
    // 然后将新任务结构指针放入任务数组的nr项中。其中nr为任务号，由前面
    // find_empty_process()返回。接着把当前进程任务结构内容复制到刚申请到
    // 的内存页面p开始处。
   p = (struct task_struct *) get_free_page();
   if (!p)
      return -EAGAIN;
   task[nr] = p;
   *p = *current; /* NOTE! this doesn't copy the supervisor stack */
    // 随后对复制来的进程结构内容进行一些修改，作为新进程的任务结构。先将
    // 进程的状态置为不可中断等待状态，以防止内核调度其执行。然后设置新进程
    // 的进程号pid和父进程号father，并初始化进程运行时间片值等于其priority值
    // 接着复位新进程的信号位图、报警定时值、会话(session)领导标志leader、进程
    // 及其子进程在内核和用户态运行时间统计值，还设置进程开始运行的系统时间start_time.
   p -> state = TASK_UNINTERRUPTIBLE;
   p -> pid = last_pid;              // 新进程号。也由find_empty_process()得到。
   p -> father = current->pid;       // 设置父进程
   p -> counter = p->priority;       // 运行时间片值
   p -> signal = 0;                  // 信号位图置0
   p->alarm = 0;                   // 报警定时值(滴答数)
   p->leader = 0;    /* process leadership doesn't inherit */
   p->utime = p->stime = 0;        // 用户态时间和和心态运行时间
   p->cutime = p->cstime = 0;      // 子进程用户态和和心态运行时间
   p->start_time = jiffies;        // 进程开始运行时间(当前时间滴答数)
    // 再修改任务状态段TSS数据，由于系统给任务结构p分配了1页新内存，所以(PAGE_SIZE+
    // (long)p)让esp0正好指向该页顶端。ss0:esp0用作程序在内核态执行时的栈。另外，
    // 每个任务在GDT表中都有两个段描述符，一个是任务的TSS段描述符，另一个是任务的LDT
    // 表描述符。下面语句就是把GDT中本任务LDT段描述符和选择符保存在本任务的TSS段中。
    // 当CPU执行切换任务时，会自动从TSS中把LDT段描述符的选择符加载到ldtr寄存器中。
   p->tss.back_link = 0;
   p->tss.esp0 = PAGE_SIZE + (long) p;     // 任务内核态栈指针。
   p->tss.ss0 = 0x10;                      // 内核态栈的段选择符(与内核数据段相同)
   p->tss.eip = eip;                       // 指令代码指针
   p->tss.eflags = eflags;                 // 标志寄存器
   p->tss.eax = 0;                         // 这是当fork()返回时新进程会返回0的原因所在
   p->tss.ecx = ecx;
   p->tss.edx = edx;
   p->tss.ebx = ebx;
   p->tss.esp = esp;
   p->tss.ebp = ebp;
   p->tss.esi = esi;
   p->tss.edi = edi;
   p->tss.es = es & 0xffff;                // 段寄存器仅16位有效
   p->tss.cs = cs & 0xffff;
   p->tss.ss = ss & 0xffff;
   p->tss.ds = ds & 0xffff;
   p->tss.fs = fs & 0xffff;
   p->tss.gs = gs & 0xffff;
   p->tss.ldt = _LDT(nr);                  // 任务局部表描述符的选择符(LDT描述符在GDT中)
   p->tss.trace_bitmap = 0x80000000;       // 高16位有效
    // 如果当前任务使用了协处理器，就保存其上下文。汇编指令clts用于清除控制寄存器CRO中
    // 的任务已交换(TS)标志。每当发生任务切换，CPU都会设置该标志。该标志用于管理数学协
    // 处理器：如果该标志置位，那么每个ESC指令都会被捕获(异常7)。如果协处理器存在标志MP
    // 也同时置位的话，那么WAIT指令也会捕获。因此，如果任务切换发生在一个ESC指令开始执行
    // 之后，则协处理器中的内容就可能需要在执行新的ESC指令之前保存起来。捕获处理句柄会
    // 保存协处理器的内容并复位TS标志。指令fnsave用于把协处理器的所有状态保存到目的操作数
    // 指定的内存区域中。
   if (last_task_used_math == current)
      __asm__("clts ; fnsave %0"::"m" (p->tss.i387));
    // 接下来复制进程页表。即在线性地址空间中设置新任务代码段和数据段描述符中的基址和限长，
    // 并复制页表。如果出错(返回值不是0)，则复位任务数组中相应项并释放为该新任务分配的用于
    // 任务结构的内存页。
   if (copy_mem(nr,p)) {
      task[nr] = NULL;
      free_page((long) p);
      return -EAGAIN;
   }
    // 如果父进程中有文件是打开的，则将对应文件的打开次数增1，因为这里创建的子进程会与父
    // 进程共享这些打开的文件。将当前进程(父进程)的pwd，root和executable引用次数均增1.
    // 与上面同样的道理，子进程也引用了这些i节点。
   for (i=0; i<NR_OPEN;i++)
      if ((f=p->filp[i]))
         f->f_count++;
   if (current->pwd)
      current->pwd->i_count++;
   if (current->root)
      current->root->i_count++;
   if (current->executable)
      current->executable->i_count++;
    // 随后GDT表中设置新任务TSS段和LDT段描述符项。这两个段的限长均被设置成104字节。
    // set_tss_desc()和set_ldt_desc()在system.h中定义。"gdt+(nr<<1)+FIRST_TSS_ENTRY"是
    // 任务nr的TSS描述符项在全局表中的地址。因为每个任务占用GDT表中2项，因此上式中
    // 要包括'(nr<<1)'.程序然后把新进程设置成就绪态。另外在任务切换时，任务寄存器tr由
    // CPU自动加载。最后返回新进程号。
   set_tss_desc(gdt+(nr<<1)+FIRST_TSS_ENTRY,&(p->tss));
   set_ldt_desc(gdt+(nr<<1)+FIRST_LDT_ENTRY,&(p->ldt));
   p->state = TASK_RUNNING;   /* do this last, just in case */
   return last_pid;
}
```

执行该函数新建立一个pcb结构，然后访问执行`addl $20, %esp`。出栈上面5个寄存器，并且把ip也出栈了，然后返回call _sys_call_table(,%eax,4)的下面一句代码。这时候内核栈是：

![image-20210209145708409](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20210209150358.png)

我们继续看`call _sys_call_table(,%eax,4)`下面的代码。主要判断是否需要重新调度进程。接下来进行信号的处理。新号另外分析，**这里假设没有信号**, 直接跳转到标签3。

```asm
3:	popl %eax                       # eax中含有上面入栈系统调用的返回值
	popl %ebx
	popl %ecx
	popl %edx
	pop %fs
	pop %es
	pop %ds
	iret
```

把剩下的一些寄存器出栈，pop eax即把系统调用的返回值存在eax里。eax的值是在下面的代码处得到的。

```asm
call _sys_call_table(,%eax,4)
pushl %eax
```

最后通过iret中断返回指令弹出五个寄存器，回到系统调用前的ip处执行。父进程返回值是eax，即子进程id。子进程的eax是0所以是返回值是0。




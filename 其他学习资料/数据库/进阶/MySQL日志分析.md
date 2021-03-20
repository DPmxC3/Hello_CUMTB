# undo log简述

日志在内存里也有缓存的，这里将其叫做log buffer。磁盘上的日志文件称为log file。log file一般是追加内容，可以认为是顺序写，顺序写的磁盘IO开销要小于随机写。

【概述】Undo log 是InnoDB MVCC事务特性的重要组成部分。当我们对记录做了变更操作时就会产生undo记录，undo记录默认被记录到系统表ibdata中，但是从MySQL 5.6以后 也可以使用独立的Undo 表空间。

【作用】其作用是保存记录的老版本数据，当一个旧的事务需要读取数据时，为了能读取到老版本的数据，需要顺着undo链找到满足其可见性的记录。当版本链很长时，通常可以认为是个比较耗时的耗时操作。因此可以用来回滚，崩溃恢复，MVCC。
大多数对数据的变更操作包括INSERT/DELETE/UPDATE，其中INSERT操作在事务提交前只对当前事务可见，因此产生的Undo日志可以在事务提交后直接删除，而对于UPDATE/DELETE则需要维护多版本信息，在InnoDB里，UPDATE和DELETE操作产生的Undo日志被归成一类，即update_undo。

【产生时机】事务开始之前，将当前的数据版本生成Undo log, Undo log也会产生redo log 来保证Undo log的可靠性。

【释放时机】当事务提交后，Undo log并不能立马被删除，而是放入待清理的链表，由purge 线程判断是否由其他事务在使用undo 段中表的上一个事务之前的版本信息，决定是否可以清理undo log的日志空间。

【存储结构】InnoDB采用回滚段的方式来维护Undo log的并发写入和持久化。

回滚段实际上是一种Undo 文件组织方式，Undo内部由多个回滚段组成，即 Rollback segment，一共有128个，保存在ibdata系统表空间中，分别从resg slot0 - resg slot127，每一个resg slot，也就是每一个回滚段，内部由1024个undo segment 组成。
为了便于管理和使用undo记录，在内存中维持了如下关键结构体对象：
1.所有回滚段都记录在 trx_sys->rseg_array，数组大小为128，分别对应不同的回滚段；
2.rseg_array 数组类型为trx_rseg_t，用于维护回滚段相关信息；
3.每个回滚段对象trx_rseg_t 还要管理undo log信息，对应结构体为trx_undo_t, 使用多个链表来维护trx_undo_t信息；
4.事务开启时，会专门给他指定一个回滚段，以后该事务用到的undo log页，就从该回滚段上分配；
5.事务提交后，需要purge的回滚段会被放到purge队列上（purge_sys->purge_queue)。

# Change Buffer简述

```txt
当需要更新一个数据页：
1. 如果数据页在内存 — 直接更新
2. 如果数据页不在内存，在不影响数据一致性的前提下，InnoDB会将这些更新操作缓存在change buffer中，这样就不需要从磁盘读入这个数据页了。在下次查询需要访问这个数据页时候，将数据页读入内存，然后执行change buffer中与这个页有关的操作。通过这种方式保证这个数据逻辑的正确性。
```

另外，虽然叫change buffer, 实际上此操作也是可以持久化的数据。将change buffer中的操作应用到原始数据页，得到最新结果的过程叫merge。除了访问这个数据页会触发merge 外，系统有后台线程会定期merge. 在db正常关闭的时候，也会执行merge。 — 如果能够将更新操作先记录在change buffer，减少读磁盘，更新语句的执行速度会得到明显的提升 。

**使用场景：**

Change buffer的主要目的就是将记录的变更动作缓存下来，所以在一个数据页做purge之前，change buffer 记录的变更越多（也就是这个页面上要更新的次数越多），收益就越大。

因此对于写多读少的业务，页面在写完以后马上被访问到的概率比较小，此时change buffer的使用效果最好。这种业务模型常见的是账单类，日志类的系统。

反过来，假设一个业务的更新模式就是写入之后马上会做查询，那么即使满足了条件，将先更新记录在change buffer,但之后由于马上要访问这个数据页，会立即出发purge过程。这样随机访问IO的次数不会减少,反而增加了change buffer的维护代价，所以对于这种业务模式来说，change buffer反而起到了副作用。

另外，只有普通索引才能使用到change buffer, 唯一索引不能用。因为唯一索引每次都要将数据页读入内存判断唯一性，所以没必要使用change buffer了。

# Redo log简述

```
保证事务的持久性。日志先行(WAL 先写日志，再写磁盘。)，即在持久化数据文件前，保证之前的redo 日志已经写在磁盘。记录的是新数据的备份。在事务提交前，只要将Redo Log持久化即可，不需要将数据持久化。当系统崩溃时，虽然数据没有持久化，但是RedoLog已经持久化。系统可以根据RedoLog的内容，将所有数据恢复到最新的状态。
具体来说，当有一条记录需要更新的时候，InnoDB引擎就会先把记录写到redo log里，并更新内存[具体操作参见change buffer]，这个时候更新就算完成了。
同时Innodb引擎会在适当的时候，将这个操作记录更新到磁盘里，而这个更新往往是在系统比较空闲的时候做。（redo log 类似MQ解耦，异步操作，把随机IO的写磁盘变成了顺序IO的写日志。）
```

WAL好处：
1.利用WAL技术，数据库将随机写换成了顺序写，大大提升了数据库性能。
2.保证crash safe : 有了redo log 可以保证即使数据库发生异常重启，之前提交的记录都不会丢失。

WAL坏处：
但是也会带来内存脏页问题，内存脏页会后台线程自动flush,也会由于数据页淘汰而触发flush. flush脏页的过程会占用资源，可能导致查询语句的响应时间长一些。

## Redo log特点

InnoDB的redo log是固定大小的，比如可以配置为一组4个文档，每个1GB，从头开始写，写到末尾就又回到开头循环写。redo log通过使用两个指针checkpoint&writepos来控制数据更新到数据文件速度。
另外，redo log是InnoDB引擎特有的日志。

## WAL/Redo log  VS change buffer

WAL /redo log 提升性能的核心机制即尽量减少随机写磁盘的IO消耗（转成顺序写）。而Change buffer 的提升性能的核心机制是节省更新语句中随机读磁盘的IO消耗 。

## 两段式提交2pc

2PC即Innodb对于事务的两阶段提交机制。当MySQL开启binlog的时候，会存在一个内部XA的问题：事务在存储引擎层（redo）commit的顺序和在binlog中提交的顺序不一致的问题。如果不使用两阶段提交，那么数据库的状态有可能用它的日志恢复出来的库的状态不一致。

事务的commit分为prepare和commit两个阶段：
1、prepare阶段：redo持久化到磁盘（redo group commit），并将回滚段置为prepared状态，此时binlog不做操作。

2、commit阶段：innodb释放锁，释放回滚段，设置提交状态，binlog持久化到磁盘，然后存储引擎层提交。
这两个阶段的意义在于，在prepare阶段，redo是没有写入redo log中的，也就是说只有当bin log提交完了，redo log才会记录上为commit，原因可以模拟一下当在redo log 或者 bin log两个有一个没有成功提交后。可以发现两种方法只要有一个在crash之前没有备份好，数据库上的数据都没法和备份的数据相匹配。这就是为啥搞个两段式提交的原因！！！



# bin log 简介

MySQL 的二进制日志 binlog 可以说是 MySQL 最重要的日志，它记录了所有的 `DDL` 和 `DML` 语句（除了数据查询语句select、show等），**以事件形式记录**，还包含语句所执行的消耗的时间，MySQL的二进制日志是事务安全型的。binlog 的主要目的是**复制和恢复**。

#### Binlog日志的两个最重要的使用场景

- **MySQL主从复制**：MySQL Replication在Master端开启binlog，Master把它的二进制日志传递给slaves来达到master-slave数据一致的目的
- **数据恢复**：通过使用 mysqlbinlog工具来使恢复数据





# 参数

## sync_binlog

这个参数直接影响MySQL的性能和完整性

### sync_binlog = 0

当事务提交之后，MySQL不做fsync之类的磁盘同步指令刷新binlog_cache中的信息到磁盘，而让Filesystem自行决定什么时候来做同步，或者cache满了之后才同步到磁盘。

### sync_binlog = N

sync_binlog=n，当每进行n次事务提交之后，MySQL将进行一次fsync之类的磁盘同步指令来将binlog_cache中的数据强制写入磁盘。

在MySQL中系统默认的设置是sync_binlog=0，也就是不做任何强制性的磁盘刷新指令，这时候的性能是最好的，但是风险也是最大的。因为一旦系统Crash，在binlog_cache中的所有binlog信息都会被丢失。而当设置为“1”的时候，是最安全但是性能损耗最大的设置。因为当设置为1的时候，即使系统Crash，也最多丢失binlog_cache中未完成的一个事务，对实际数据没有任何实质性影响。

## innodb_flush_log_at_trx_commit

**0:** 由mysql的main_thread每秒将存储引擎log buffer中的redo日志写入到log file，并调用文件系统的sync操作，将日志刷新到磁盘。

**1：**每次事务提交时，将存储引擎log buffer中的redo日志写入到log file，并调用文件系统的sync操作，将日志刷新到磁盘。

**2：**每次事务提交时，将存储引擎log buffer中的redo日志写入到log file，并由存储引擎的main_thread 每秒将日志刷新到磁盘。

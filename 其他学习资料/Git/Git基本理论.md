# 1. 版本控制

> 什么是版本控制 版本迭代 新的版本 版本管理器

* 实现跨区域多人协同开发
* 追踪和记载一个或者多个文件的历史记录
* 组织和保护你的源代码和文档
* 统计工作量
* 并行开发，提高开发效率
* 追踪记录整个软件的开发过程
* 减轻开发人员的负担，节省时间，同时降低人为错误

> 常用的版本控制工具

主流的有：

* Git
* SVN
* CVS
* VSS
* TFS
* Visual Studio Online

最广泛的还是Git



**分类**

## 1.1 本地版本控制

记录文件每次的更新，可以对每个版本做一个快照，或是记录补丁文件，适合个人用。



##  1.2 集中式版本控制 SVN

所有的版本数据都保存在服务器上，协同开发者从服务器上同步更新或上传自己的修改

![image-20200908172913583](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201112223041.png)

所有的版本数据都存在服务器上，用户的本地只有自己以前同步的版本，如果不联网，用户就看不到历史版本，也无法切换版本验证问题。或在不同分支工作。而且，所有数据都保存在单一的服务器上，有很大的风险这个服务器会损坏。这样会丢失所有数据，当然也可以定期备份。代表有SVN CVS VSS

## 1.3 分布式管理系统 Git

每个人都拥有全部的代码！

所有版本信息仓库全部同步到本地的每个用户，这样就可以在本地查看所有版本历史，可以离线在本地提交，只需要在连网时push到相应的服务器或其他用户哪里。由于每个用户哪里保存的都是所有的版本数据，只要有一个用户的设备没有问题就可以恢复所有的数据，但这增加了本地存储空间的占用

不会因为服务器损坏或者网络问题，造成不能工作的情况！

![image-20200908174443398](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201112223119.png)



## 1.4 Git与SVN的最主要区别

SVN时集中式版本控制系统，版本库时集中放在中央服务器上的，而工作的时候，用的都是自己的电脑，所以首先要从中央服务器得到最新的版本，然后工作，完成工作后需要把自己做完的活推到中央服务器。集中式版本控制系统时必须联网才能工作，对网络带宽要求比较高



Git时分布式版本控制系统，没有中央服务器，每个人的电脑就是一个完整的版本库，工作的时候不需要联网了，因为版本都在自己电脑上。协同的方法时这样的：比如说自己在电脑上改了文件A，其他人也在电脑上改了文件A，这是，你们俩之间只需要把各自的修改推送给对方，就可以相互看见对方的修改了。

Git时目前世界上最先进的分布式版本控制系统。



# 2. Git的历史

同生活中的许多伟大事物一样，Git 诞生于一个极富纷争大举创新的年代。

Linux 内核开源项目有着为数众多的参与者。 绝大多数的 Linux 内核维护工作都花在了提交补丁和保存归档的繁琐事务上（1991－2002年间）。 到 2002 年，整个项目组开始启用一个专有的分布式版本控制系统 BitKeeper 来管理和维护代码。

==Linus社区中存在很多大佬！研究破解BitKeeper！==

到了 2005 年，开发 BitKeeper 的商业公司同 Linux 内核开源社区的合作关系结束，他们收回了 Linux 内核社区免费使用 BitKeeper 的权力。 这就迫使 Linux 开源社区（特别是 Linux 的缔造者 Linus Torvalds）基于使用 BitKeeper 时的经验教训，==两周开发出自己的版本系统==。 他们对新的系统制订了若干目标：

- 速度
- 简单的设计
- 对非线性开发模式的强力支持（允许成千上万个并行开发的分支）
- 完全分布式
- 有能力高效管理类似 Linux 内核一样的超大规模项目（速度和数据量）

自诞生于 2005 年以来，Git 日臻成熟完善，在高度易用的同时，仍然保留着初期设定的目标。 它的速度飞快，极其适合管理大项目，有着令人难以置信的非线性分支管理系统（参见 [Git 分支](https://git-scm.com/book/zh/v2/ch00/ch03-git-branching)）。

**Git时目前世界上最先进得分布式系统**

**Git是免费，开源的，最初Git是为辅助Linus内核开发的，来替代BitKeeper**



# 3. Git的环境

这下载就自己下了 然后配个环境变量

```bash
git config --global --list # 全局查看自己的配置 相当于身份证 要有一个可以识别的东西
git config --system --list # 查看系统的配置
git config -l
```

![image-20200908182948620](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201112223148.png)





**Git相关的配置文件**

1. Git\etc\gitconfig: Git 安装目录下的gitconfig --system系统级
2. C:\User\Administrator\\.gitconfig 只适用于当前登录用户的配置 --global全局

这里可以直接编辑配置文件，通过命令设置后会响应到这里

我的.gitconfig里面的内容

```text
[filter "lfs"]
	clean = git-lfs clean -- %f
	smudge = git-lfs smudge -- %f
	process = git-lfs filter-process
	required = true
[user]
	name = zouyishan
	email = 2983748714@qq.com

```



**清空以后如何设置用户信息(使用之前必须要配置的)**

```bash
git config --global user.name "zouyishan"
git config --global user.email 2983748714@qq.com
```

到这基本Git的环境就配好了



# 4. Git基本理论

> 工作区域

Git本地有三个工作区域：工作目录(Working Directory), 暂存区(Stage/Index), 资源区(Repository或Git Directory)。如果在加上远程的git仓库(Remote Directory)就可以分为四个工作区域。文件在这四个区域之间的转换关系如下:

![image-20200908184611137](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201112223205.png)

如果要传东西到git上就只需要记住左边三个命令就行

```bash
git add files
git commit
git push
```

如果从远程拉到本地就只需要

```bash
git pull
git reset
git checkout
```

* Workspace: 工作区，就是你平时存放项目代码的地方
* Index/Stage: 暂存区，用于临时存放你的改动，事实上只是一个文件，保存即将提交到文件列表信息
* Repository: 仓库区(或本地仓库)，就是安全存放数据的位置，这里面有你提交到所有版本的数据。其中HEAD指向最新放入仓库的版本
* Remote: 远程仓库，托管代码的服务器，可以简单的认为是你项目组中的一台电脑用于远程数据交换






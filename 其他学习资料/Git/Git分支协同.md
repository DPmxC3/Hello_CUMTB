# Git分支协同

看本文需要会些基本的理论。可参照前面的git基本理论和git实操以后进行学习

[协同开发](#协同开发)



## 单项目的git

## git构架

先来看看git的基本构架:

![git](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201112223205.png)

**工作区(working directory)** :之就是你工作的区域。对于git而言，就是的本地工作目录。工作区的内容会包含提交到暂存区和版本库(当前提交点)的内容，同时也包含自己的修改内容。

**暂存区(stage area, 又称为索引区index)** :it中一个非常重要的概念。是我们把修改提交版本库前的一个过渡阶段。查看GIT自带帮助手册的时候，通常以index来表示暂存区。在工作目录下有一个.git的目录，里面有个index文件，存储着关于暂存区的内容。git add命令将工作区内容添加到暂存区。

**本地仓库(local repository)** :控制系统的仓库，存在于本地。当执行git commit命令后，会将暂存区内容提交到仓库之中。在工作区下面有.git的目录，这个目录下的内容不属于工作区，里面便是仓库的数据信息，暂存区相关内容也在其中。这里也可以使用merge或rebase将 **远程仓库副本** 合并到本地仓库。图中的只有merge，注意这里也可以使用rebase。

**远程版本库(remote repository)** :与本地仓库概念基本一致，不同之处在于一个存在远程，可用于远程协作，一个却是存在于本地。通过push/pull可实现本地与远程的交互；

**远程仓库副本** :可以理解为存在于本地的远程仓库缓存。如需更新，可通过git fetch/pull命令获取远程仓库内容。使用fech获取时，并未合并到本地仓库，此时可使用git merge实现远程仓库副本与本地仓库的合并。git pull 根据配置的不同，可为git fetch + git merge 或 git fetch + git rebase。rebase和merge的区别可以自己去网上找些资料了解下。



## 什么是分支

可以把分支想象成一个叉子

![image-20201113064116191](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114215020.png)

比如小明的试卷现在上面全是错误，你看不下去，把小明的试卷复印下来。然后自己对着复印的小明的试卷进行修改。修改完以后再拿给小明看，如果小明觉得对。改的还行的话就改成你改的那份试卷~

### 怎么创建分支

git 命令的话只是通过

```bash
git branch [branchname]
```

看如下命令:

```bash
git branch one # 创建one分支
git branch # 查看所有本地分支
```

![image-20201113064832327](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213537.png)

### 切换分支

切换分支也非常简单

```bash
git checkout [branch]
```

例如:

```bash
git checkout one
```

太过简单不做图片展示

同时也可以直接创建一个分支然后切换 命令是:

```bash
git checkout -b [barach]
```

### 合并分支

看一看 **fast-forward** 的合并:

![image-20201113071226986](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213541.png)



如果是 **non fast-forward** 呢，non fast-forward意思就是一个分支中已经有一部分代码，所以它不允许你直接把你的代码覆盖上去。

主要办法就是协商修改相应的差异，然后在add和commit就行。同时也可以用rebase不过这样有的记录会消失。不建议，就自己去了解

如果是仓库的话可以直接强推

```bash
git push -f origin master
```



### 删除分支

直接给命令吧

```bash
git branch -d one # 删除one分支
```



 到这里基本能维护自己的单独项目了。接下来就看看git的协同开发吧。





# 协同开发

首先：我们先对自己感兴趣的项目在GitHub上fork一下。这样在自己的本地仓库就会有一个一模一样的项目了。

然后对本地分支的修改已经不需要多讲了。前面的操作会了就肯定能对本地仓库进行操作。这里不赘述



## 如何让自己的远程分支同步远程主分支

进行远程协作不可避免的问题就是。我先fork了源项目。然后源项目更新了，然后我正好又在分支上更新了一些东西，准备合并到本地主分支。

显然这个解决办法就是先把本地主分支同步。在去合并本地分支。本地分支合并不赘述。讲讲 **如何让自己的远程分支同步远程主分支**。

首先先添加好远程主仓库的地址:

```bash
git remote add [name] [url] # 添加地址
git remote -v # 查看主仓库地址是否添加进去
```

对仓库进行fetch:

```bash
git fetch root # 更新head
```

合并到本地仓库:

```bash
git merge root/master
```

本地仓库push到fork的项目地址

```bash
git push origin master
# 当然也可以先把本地fork未更新的pull到本地仓库 
# git pull origin master
# git push origin master
```



**这是实际操作**

![image-20201114204759881](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213546.png)

![image-20201114204815618](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213549.png)



## 删除GitHub远程仓库中的某个文件和本地仓库的文件

从github上只能删除仓库，无法删除文件夹或文件，只能通过命令行解决

**远程仓库**:进入你的master分支下

```bash
git pull origin master # 将远程仓库里面的项目拉下来
dir # 看看有那些目录
git rm -r --cached target
git commit -m '删除了target'
git push origin master
```

![image-20201114205249763](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213551.png)

**本地仓库的文件** : 这里就可以直接使用Linux的命令来操作。不想一个一个点确认可以这样

```bash
yes | rm -r [文件名]
```



## 如何提交自己的更新PR

先说清楚。这是GitHub的东西。不是git的东西

我们先更新我们自己的远程仓库副本

1. 这里已经把我们远程仓库副本修改了。增加了Java和数据库文件![image-20201114211440122](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213556.png)



2. 然后点击页面的Pull requests![image-20201114211718025](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213558.png)
3. 在新的页面点击新建一个请求![image-20201114211808578](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213600.png)

4. 对比不同 新建修改![image-20201114212016661](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213603.png)

5. 填写信息正式提交![image-20201114212624226](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213605.png)

6. 这样就完成了。就静等项目管理员进行审核，项目管理员在Pull request上有列表，如果你能得到管理员的审核，你就对项目做出贡献了~![image-20201114212926792](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213607.png)

7. 如果成功的话，你就会收到merge成功的邮件了~![image-20201114213458698](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201114213609.png)

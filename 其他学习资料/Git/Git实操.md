# 5. Git项目搭建

> 本地仓库搭建

首先先建立一个git仓库 `git init` 注意这是隐藏文件 或者clone远程的 `git clone url`



# 6. Git文件操作

> 文件四种操作

版本控制就是对文件的版本控制，要对文本进行修改，提交等操作，首先要知道文件当前你在上面状态，不然可能会提交了现在还不想提交的文件，或者要提交的文件没提交上

* Untracked：未跟踪，此文件在文件夹中，但并没有加入到git库，不参与版本控制，通过`git add`状态变为`Staged`
* Unmodify：文件已经入库，未修改，即版本库中的文件快照内容与文件夹中完全一致，这种类型的文件有两种取出，如果它被修改，而变为`Modified`，如果使用`git rm`移出版本库，则成为`Untracked`文件
* Modified：文件已修改，仅仅是修改，并没有进行其他操作，这个问价你也有两个去处,通过`git add`可以进入暂存`staged`状态，使用`git checkout`则丢弃修改过，返回到`unmodify`状态，这个`git checkout`即从库中取出文件，覆盖当前修改！
* Stage：暂存状态 执行`git commit`则将修改同步到库中，这是库中的文件和本地文件又变为一致，文件为`Unmodify`状态，执行`git reset HEAD filename`取消暂存，文件状态为`Modified`



> 查看文件状态



上面说文件有4种状态，通过如下命令可以查看问价状态:

```bash
git add . # 添加到缓存区
git status [filename] # 查看指定文件状态
git status # 查看所有文件状态
git commit -m "注释内容" # 提交暂存区的内容
```



> 忽略文件

.gitignore

忽略文件种的空行或以井号(#)开始的行将会被忽略

```bash
# 为注释
*.txt # 忽略所有 .txt结尾文件，这样的话上传就不会被选中
!lib.txt # 但lib.txt除外
/temp # 仅忽略项目根目录下的文件，不包括其他目录
build/ # 忽略build/目录下的所有文件
doc/*.txt # 会忽略doc/notes.txt但不包括 doc/server/arch.txt
```



# 7. 使用Github

先生成公钥(如果没有公钥):

```bash
ssh-keygen -t rsa -C "[email]@[email].com" # 注意-key哪里没有空格
```

这时在你的.ssh文件中就会有生成两个文件`id_rsa`和`id_rsa.pub`第一个是私钥千万不要告诉别人。第二个是公钥可以随便告诉别人没关系。然后到GitHub首页点击settings -> SSH -> New SSH key然后把你的公钥复制上去就可以了。


**下面就是详细过程了**


```bash
ssh -T git@github.com # 检测公钥是否正确 如果正确返回
#Hi zouyishan! You've successfully authenticated, but GitHub does not provide shell access.

git clone [url] # 从远程服务器上把clone到本地
# 如果开始没有勾选创建readme文件则要
git init
touch README.md
git add README.md
git commit -m 'first_commit'
git remote add origin [url]
git push origin master
===============================================

git init # 初始化
git add . # 添加到临时仓库
git commit -m 'name' # 添加到本地仓库
git remote add origin [url] # 这个url是你的仓库的地址
# 这里可以pull一下 执行如下命令
# git pull origin master
git push origin master # 如果push不上去的话可以推荐加个-f强推
# git push -f origin master
```



**git pull 和 git fetch + git merge的区别**

```bash
# 先fetch从远程获取最新版本到本地，不会自动合并
git fetch origin master
git merge origin/master
# 从远程获取并merge到本地
git pull origin master
```

从实际上看`git fetch`和`git merge`安全一点 因为可以查看更新情况。然后决定是否合并。具体如下:

```bash
git fetch origin master:tmp
git diff tmp
git merge tmp
```

# 8. Git分支


```bash
git branch # 列出所有本地分支
git branch -r # 列出所有远程分支
git branch dev # 新建一个分支
git checkout -b [branch] # 切换分支
git merge [branch] # 合并指定到当前分支
git branch -d [branch-name] # 删除分支
git push origin --delete [branch-name] # 删除远程分支
git branch -dr [remote/branch] # 删除远程分支
```


首先要用git就肯定要下载git吧~

下载地址：https://git-for-windows.github.io/

然后走流程安装，最重要的就是 这两个要勾上，其他的就一直点确定 然后finish就行

![image-20201121190855645](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201121192757.png)



这个弄好了先创建一个空的文件夹用于测试

然后在空文件下点击鼠标右键

![image-20201121191230731](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201121192804.png)

然后新建一个文件

![image-20201121191352358](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201121192807.png)

输入git init初始化仓库

![image-20201121191505301](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201121192809.png)



再然后就开始通过你GitHub的账户信息进行配置



首先要先配置你的账号 在命令行输入一下命令：

```
git config --global user.name "zouyishan"
git config --global user.email 2983748714@qq.com
```

如果要看自己是否配置成功可以通过命令：

```bash
git config --global --list # 查看配置信息
```

我这里就配置好了~~

![image-20201121192218420](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201121192811.png)

同时 Git相关的配置文件也可以在本机找到：

```
Git\etc\gitconfig: Git 安装目录下的gitconfig —system系统级
C:\User\Administrator\.gitconfig 只适用于当前登录用户的配置 —global全局
```

如果直接更改这些配置文件，相应的用户信息也可以映射



然后到这里你就配好了git的环境了~ 
然后如果对git的构架感兴趣可以看看[这个](https://github.com/Alexiosvon/Hello_CUMTB/blob/master/%E5%85%B6%E4%BB%96%E5%AD%A6%E4%B9%A0%E8%B5%84%E6%96%99/Git/Git%E5%9F%BA%E6%9C%AC%E7%90%86%E8%AE%BA.md)和[这个](https://github.com/Alexiosvon/Hello_CUMTB/blob/master/%E5%85%B6%E4%BB%96%E5%AD%A6%E4%B9%A0%E8%B5%84%E6%96%99/Git/Git%E5%AE%9E%E6%93%8D.md)。

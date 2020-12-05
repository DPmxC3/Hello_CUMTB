# 配置网络

首先如果你魔改了你的网络配置，最好先恢复默认的。

![image-20201205001653017](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201205153935.png)

然后进入到虚拟机中。选择`NAT模式`, 此时ping是ping不通的。

先编辑一下网络配置。记住`Linux中一切皆文件`

```
vim /etc/sysconfig/network-scripts/ifcfg-ens33
```

这里我们选用自动分配`dhcp`如果自己设置静态IP的要自行配置改成`static`然后还要填写`IPADDR`等信息。

然后最重要的就是改成`ONBOOT=yes` 保存退出即可。

 ![image-20201205153016563](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201205153928.png)

然后重启网络配置:

```shel
service network restart
```

然后ping baidu.com 就能上网了！



# 将yum换成清华的源

首先备份一下`/etc/yum.repos.d/CentOS-Base.repo`

```bash
mv /etc/yum.repos.d/CentOS-Base.repo /etc/yum.repos.d/CentOS-Base.repo.backup
```

**将新的源文件上传到/etc/yum.repos.d 目录或在改目录下新建一个文件 命名为 CentOS-Base.repo, 这是CentOS7的版本，其他版本可以在[清华镜像站上找找](https://mirrors.tuna.tsinghua.edu.cn/help/centos/)**

```
# CentOS-Base.repo
#
# The mirror system uses the connecting IP address of the client and the
# update status of each mirror to pick mirrors that are updated to and
# geographically close to the client.  You should use this for CentOS updates
# unless you are manually picking other mirrors.
#
# If the mirrorlist= does not work for you, as a fall back you can try the
# remarked out baseurl= line instead.
#
#


[base]
name=CentOS-$releasever - Base
baseurl=https://mirrors.tuna.tsinghua.edu.cn/centos/$releasever/os/$basearch/
#mirrorlist=http://mirrorlist.centos.org/?release=$releasever&arch=$basearch&repo=os
enabled=1
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-7

#released updates
[updates]
name=CentOS-$releasever - Updates
baseurl=https://mirrors.tuna.tsinghua.edu.cn/centos/$releasever/updates/$basearch/
#mirrorlist=http://mirrorlist.centos.org/?release=$releasever&arch=$basearch&repo=updates
enabled=1
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-7



#additional packages that may be useful
[extras]
name=CentOS-$releasever - Extras
baseurl=https://mirrors.tuna.tsinghua.edu.cn/centos/$releasever/extras/$basearch/
#mirrorlist=http://mirrorlist.centos.org/?release=$releasever&arch=$basearch&repo=extras
enabled=1
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-7



#additional packages that extend functionality of existing packages
[centosplus]
name=CentOS-$releasever - Plus
baseurl=https://mirrors.tuna.tsinghua.edu.cn/centos/$releasever/centosplus/$basearch/
#mirrorlist=http://mirrorlist.centos.org/?release=$releasever&arch=$basearch&repo=centosplus
gpgcheck=1
enabled=0
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-7
```

然后执行命令:

```
yum clean all
yum makecache
```

 ![image-20201205140227097](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201205153921.png)

OK yum源就配置好了，享受4 5MB/S的下载速度吧！



# 下载GCC

按照上面来做，这时候肯定能上网了。然后就输入



我先卸载一遍gcc

![image-20201205002715983](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201205153916.png)



默认确定安装gcc

![image-20201205002931532](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201205153912.png)

成功了

![image-20201205002957015](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201205153907.png)

同理安装好`g++`和`gdb`工具。

```shell
yum install -y g++
yum install -y gdb
```

在安装的时候可能会报这个错：`Couldn't open file /etc/pki/rpm-gpg/RPM-GPG-KEY-7`。

然后输入以下指令就行：

```
rpm --import /etc/pki/rpm-gpg/RPM*
yum makecache
```



输入`gcc -v`和`gcc --version`有这些东西就代表你安装好了：

![image-20201205003322786](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201205153901.png)



然后就好好享受在`Linux`上coding的快乐吧

![img](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201205153846.png)



![img](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201205153855.png)

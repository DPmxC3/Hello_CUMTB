此部分为同目录下Linux的补充，同样不会涉及很深的知识，都是些基本的操作。

## 防火墙

实际上防火墙也是一个命令，`iptables`先让我们来看看防火墙怎么使用吧

首先在CentOS7中是没有默认安装防火墙的，先来安装

```bash
yum install iptables-services
```

然后启动一下防火墙吧！

```
systemctl enable iptables
systemctl start iptables
```

![image-20201211234655488](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201212010018.png)

查看防火墙状态

```bash
service iptables status
```

![image-20201211234802790](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201212010024.png)

关闭防火墙

```bash
systemctl stop iptables
```

![image-20201211235023647](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201212010028.png)



Linux防火墙（即Netfilter/iptables IP过滤系统）在企业中的应用非常广泛，主要应用在：

- 对于IDC机房的服务器，可以使用Linux防火墙来代替硬件防火墙，由于IDC机房的机器一般是没有硬件防火墙的，因此开源免费的iptables是一个性价比不错的选择。
- 在各大云计算平台下，都有“安全组”的存在，其原理与iptables极为类似，但是底层具体是如何实现的我们就不得而知了。
- 利于iptables来作为企业的NAT路由器，从而代替传统路由器提供企业内部员工上网只用，在节约成本和进行有效控制上，Linux防火墙确实有它独有的优势。
- 结合Squid作为企业内部上网的透明代理。传统的代理需要在浏览器里配置代理服务器信息，而iptables结合Squid的透明代理则可以把客户端的请求重定向到代理服务器的端口，让客户感知不到代理的存在，当然，客户端也无法做任何设置。
- 用于外网IP向内网IP映射。我们可以假设有一家ISP提供园区Internet接入服务，为了方便管理，该iSP分配给园区用户的IP地址都是内网IP，但是部分用户要求建立自己的web服务器对外发布信息，这时可以在防火墙的外部网卡上绑定多个合法IP地址，然后通过IP映射使发给其中一个IP地址的包转发至内部用户的web服务器上，这样内部的web服务器也就可以对外提供服务了，这种形式的NAT一般称为DNAT。
- 防止轻量级的DOS攻击，比如ping攻击以及SYN洪水攻击，我们利用iptables来做相关安全策略还是很有效果的。

### 关于端口

一，查看那些端口被打开

```bash
lsof -i tcp:80
```

二，关闭端口

```bash
iptables -A OUTPUT -p tcp --dport 端口号 -j DROP
```

三，开启端口

```bash
iptables -A INPUT -p tcp --dport 端口号 -j DROP
```

四，保存设置

```bash
service iptables save
```



## 关于下载东西

**wget**

那就要用到wget了！wget就是在网上找资源来下载啊

```bash
wget https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201211204541.png
```

![image-20201212002024379](https://zouyishan.oss-cn-beijing.aliyuncs.com/images/20201212010035.png)



**rpm**

rpm，我的理解是一个安装工具，但是由于一个软件可能要有很多依赖包

看看其中的参数就行==

- -a 　查询所有套件。
- -b<完成阶段><套件档>+或-t <完成阶段><套件档>+ 　设置包装套件的完成阶段，并指定套件档的文件名称。
- -c 　只列出组态配置文件，本参数需配合"-l"参数使用。
- -d 　只列出文本文件，本参数需配合"-l"参数使用。
- -e<套件档>或--erase<套件档> 　删除指定的套件。
- -f<文件>+ 　查询拥有指定文件的套件。
- -h或--hash 　套件安装时列出标记。
- -i 　显示套件的相关信息。
- -i<套件档>或--install<套件档> 　安装指定的套件档。
- -l 　显示套件的文件列表。
- -p<套件档>+ 　查询指定的RPM套件档。
- -q 　使用询问模式，当遇到任何问题时，rpm指令会先询问用户。
- -R 　显示套件的关联性信息。
- -s 　显示文件状态，本参数需配合"-l"参数使用。
- -U<套件档>或--upgrade<套件档> 升级指定的套件档。
- -v 　显示指令执行过程。
- -vv 　详细显示指令执行过程，便于排错。
- -addsign<套件档>+ 　在指定的套件里加上新的签名认证。
- --allfiles 　安装所有文件。
- --allmatches 　删除符合指定的套件所包含的文件。
- --badreloc 　发生错误时，重新配置文件。
- --buildroot<根目录> 　设置产生套件时，欲当作根目录的目录。
- --changelog 　显示套件的更改记录。
- --checksig<套件档>+ 　检验该套件的签名认证。
- --clean 　完成套件的包装后，删除包装过程中所建立的目录。
- --dbpath<数据库目录> 　设置欲存放RPM数据库的目录。
- --dump 　显示每个文件的验证信息。本参数需配合"-l"参数使用。
- --excludedocs 　安装套件时，不要安装文件。
- --excludepath<排除目录> 　忽略在指定目录里的所有文件。
- --force 　强行置换套件或文件。
- --ftpproxy<主机名称或IP地址> 　指定FTP代理服务器。
- --ftpport<通信端口> 　设置FTP服务器或代理服务器使用的通信端口。
- --help 　在线帮助。
- --httpproxy<主机名称或IP地址> 　指定HTTP代理服务器。
- --httpport<通信端口> 　设置HTTP服务器或代理服务器使用的通信端口。
- --ignorearch 　不验证套件档的结构正确性。
- --ignoreos 　不验证套件档的结构正确性。
- --ignoresize 　安装前不检查磁盘空间是否足够。
- --includedocs 　安装套件时，一并安装文件。
- --initdb 　确认有正确的数据库可以使用。
- --justdb 　更新数据库，当不变动任何文件。
- --nobulid 　不执行任何完成阶段。
- --nodeps 　不验证套件档的相互关联性。
- --nofiles 　不验证文件的属性。
- --nogpg 　略过所有GPG的签名认证。
- --nomd5 　不使用MD5编码演算确认文件的大小与正确性。
- --nopgp 　略过所有PGP的签名认证。
- --noorder 　不重新编排套件的安装顺序，以便满足其彼此间的关联性。
- --noscripts 　不执行任何安装Script文件。
- --notriggers 　不执行该套件包装内的任何Script文件。
- --oldpackage 　升级成旧版本的套件。
- --percent 　安装套件时显示完成度百分比。
- --pipe<执行指令> 　建立管道，把输出结果转为该执行指令的输入数据。
- --prefix<目的目录> 　若重新配置文件，就把文件放到指定的目录下。
- --provides 　查询该套件所提供的兼容度。
- --queryformat<档头格式> 　设置档头的表示方式。
- --querytags 　列出可用于档头格式的标签。
- --rcfile<配置文件> 　使用指定的配置文件。
- --rebulid<套件档> 　安装原始代码套件，重新产生二进制文件的套件。
- --rebuliddb 　以现有的数据库为主，重建一份数据库。
- --recompile<套件档> 　此参数的效果和指定"--rebulid"参数类似，当不产生套件档。
- --relocate<原目录>=<新目录> 　把本来会放到原目录下的文件改放到新目录。
- --replacefiles 　强行置换文件。
- --replacepkgs 　强行置换套件。
- --requires 　查询该套件所需要的兼容度。
- --resing<套件档>+ 　删除现有认证，重新产生签名认证。
- --rmsource 　完成套件的包装后，删除原始代码。
- --rmsource<文件> 　删除原始代码和指定的文件。
- --root<根目录> 　设置欲当作根目录的目录。
- --scripts 　列出安装套件的Script的变量。
- --setperms 　设置文件的权限。
- --setugids 　设置文件的拥有者和所属群组。
- --short-circuit 　直接略过指定完成阶段的步骤。
- --sign 　产生PGP或GPG的签名认证。
- --target=<安装平台>+ 　设置产生的套件的安装平台。
- --test 　仅作测试，并不真的安装套件。
- --timecheck<检查秒数> 　设置检查时间的计时秒数。
- --triggeredby<套件档> 　查询该套件的包装者。
- --triggers 　展示套件档内的包装Script。
- --verify 　此参数的效果和指定"-q"参数相同。
- --version 　显示版本信息。
- --whatprovides<功能特性> 　查询该套件对指定的功能特性所提供的兼容度。
- --whatrequires<功能特性> 　查询该套件对指定的功能特性所需要的兼容度。

真的害怕==



**yum**

yum永远嘀神。这是直接在网上找到对应的资源包，然后自动下载依赖，免去了自动配置的烦恼了！

这就不做演示了==前面安装gcc 和 Linux文档中也讲过



## 关于进程

首先如果你想要一个进程在后台运行只需要在命令后面加上一个`&`, 例如：

```bash
ping baidu.com >> ping.txt &
```

通过`jobs`来看后台运行的进程，有多种状态`running`, `stopped`, `Terminated`

```bash
jobs
```

关于`fg`， 将后台的命令调用到前台执行格式为

```bash
fg %jobnumber
```

关于`bg`, 将一个在后台暂停的命令，继续执行，同样 也是

```bash
bg %jobnumber
```

`kill`，这个法子就多了

```bash
1, 通过job查看，执行 kill %jobnumber
2, 通过ps查看，执行 kill pid
```

`nohup` 指的是关闭了中断也还可以运行。`&`只能在中端开着的时候用。关闭就没了，所以啊，比如Java要运行一个jar包就用这个

```bash
nohup java -jar xxx.jar >> log.txt 2>&1 &
```



## 管道符和重定向

我们先说管道符`|`

```bash
命令行1 | 命令行2 # 这里的意思是将命令行1的输出作为命令行2的输入。并得出结果
```

例如

```bash
ls | grep bin # 这个grep表示正则查找的吧。然后就把ls输出的做一个过滤 找到bin啊
```



再来说说重定向，先来看看区别

```bash
命令行1 | 命令行2 # 将命令1传递给命令2
命令行1 > 某个文件 # 是将命令行1的输出送到文件中保存
```

例如我们前面在后台中运行`ping`的指令

```bash
ping baidu.com >> ping.txt &
```

这个的意思就是把`ping baidu.com`这个命令的输出加载到`ping.txt`中啊。

同时 `>`这个表示重置文件，`>>`这个表示追加文件内容。



## 基本命令

接上Linux文档中没有讲的。

Linux中的复制，移动，删除文件。

```bash
cp 要被复制的文件 被送去的位置 # 复制

mv 文件1 文件2 # 将文件1的移动到文件2中

rm -r 文件名 # 删除文件
```



ok 等有必要实操`数据库`, `nginx`等到那时候在更新操作。不过应该会等到2021年的3 4月份了

&nbsp;

&nbsp;

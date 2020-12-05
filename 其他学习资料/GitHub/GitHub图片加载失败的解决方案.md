# GitHub图片加载失败解决方案

## 前言
随便打开一个项目，项目里边的图片都不能显示了，甚至个人头像也没了。于是找了一些方法，成功解决问题，有相同问题的可以参考一下。

## 解决方案
修改hosts文件

hosts文件位置

- Windows:&emsp;C:\Windows\System32\drivers\etc\hosts
- Mac/Linux:&emsp;/etc/hosts


找到hosts文件之后，打开，在文件末尾添加

```
# GitHub Start 
140.82.112.4 github.com
140.82.114.3 gist.github.com
185.199.111.153 assets-cdn.github.com
151.101.208.133 raw.githubusercontent.com
151.101.248.133 gist.githubusercontent.com
151.101.248.133 cloud.githubusercontent.com
151.101.208.133 camo.githubusercontent.com
151.101.248.133 avatars0.githubusercontent.com
151.101.208.133 avatars1.githubusercontent.com
151.101.208.133 avatars2.githubusercontent.com
151.101.208.133 avatars3.githubusercontent.com
151.101.208.133 avatars4.githubusercontent.com
151.101.208.133 avatars5.githubusercontent.com
151.101.208.133 avatars6.githubusercontent.com
151.101.248.133 avatars7.githubusercontent.com
151.101.208.133 avatars8.githubusercontent.com
185.199.108.154 github.githubassets.com
140.82.121.5 api.github.com
# GitHub End 
```
修改完保存文件就OK了。
如果无法保存、没有修改权限，可以鼠标右键单击hosts文件-属性-安全-修改权限；也可以将hosts文件复制一份，修改复制版，然后替换原文件夹中的文件。

## 最后
更改hosts后，注意`ipconfig /flushdns`刷新DNS缓存。

具体步骤：
1. Win+R -> 输入cmd -> 按回车
2. 输入：`ipconfig /flushdns` ,然后回车，执行命令，可以重建刷新本地DNS缓存。

**如果图片再次无法加载，可能是IP地址换了，可以再来更新一下。**

本文件的IP地址，随缘更新 :sweat_smile:


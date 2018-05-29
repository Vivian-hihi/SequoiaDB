##检查fuselib库##
- 使用root用户权限查看fuse版本号

```lang-javascript
# which fusermount
/bin/fusermount
# fusermount -V
fusermount version: 2.9.4
```    
以上结果显示fuselib已经安装并且fusermount的版本为2.9.4，未安装或者检测版本号小于2.9.4，请参考以下步骤进行fuselib的安装。

##安装fuselib##
###安装前准备###
- 使用root用户权限安装fuselib库
- 下载fuselib的源码包   
[fuse-2.9.4.tar.gz](https://github.com/libfuse/libfuse/releases/download/fuse_2_9_4/fuse-2.9.4.tar.gz)     

###编译安装步骤###
（1）解压源码包  
  
```lang-javascript
# tar -xzvf fuse-2.9.4.tar.gz  
```    
（2）进行编译安装    

```lang-javascript 
# cd fuse-2.9.4/
# ./configure  
# make   
# make install
```  
编译无报错，即表明编译安装通过，进一步检查是否成功安装。   
（3）检查安装结果

 ```lang-javascript
# which fusermount
/usr/local/bin/fusermount
# fusermount -V
fusermount version: 2.9.4
```
如果此处检测出的fusermount版本依然不匹配，则查看/bin目录下是否有旧版本的fusermount，可以用新版本的fusermount进行替换。

##SequoiaFS的安装##
SequoiaFS可执行程序集成于SequoiaDB的安装包中，所以可以通过从SequoiaDB的bin文件tar包中解压获取，或者可以通过安装SequoiaDB的run包获取，也可以从其他部署相同版本的SequoiaDB的环境进行拷贝到本地节点使用。下面以从tar包中解压为例进行说明。  
安装包获取地址:[SequoiaDB](http://www.sequoiadb.com/cn/index.php?a=index&m=Download)    

 ```lang-javascript
# tar -xzvf sequoiadb-3.0-linux_x86_64-bin.tar.gz 
# cd bin
# chmod +x sequoiafs 
# cp sequoiafs  /usr/local/bin/
# which sequoiafs
/usr/local/bin/sequoiafs
```
如果which没找到需要将/usr/local/bin添加到环境变量PATH中。

```lang-javascript
# sequoiafs -v
SequoiaFS version: 3.0
Release: 35747
2018-05-25-09.07.55
```

通过安装SequoiaDB的run包获取，可以参照[数据库安装](installation/deployment/command_installation/installation.md),安装完成后无需进行部署，只需将安装目录下bin/sequoiafs按以上步骤cp到/usr/local/bin并配置好环境变量即可，从其他部署好的相同版本的环境上进行拷贝也只需要配置好环境变量即可。


##使用方法##

```
sequoiafs mountpoint [options]
```

该命令用以挂载目录mountpoint，mountpoint目录为本地创建用于挂载映射目标集合的目录，挂载之后mountpoint目录即和一普通文件系统目录一样，即可在mountpoint目录下进行常见的文件操作，如常见的创建子目录、创建文件、删除文件等linux系统命令，也可以通过常见的普通文件API接口对目录文件进行操作。
     
在本例中：远程DB节点表示SequoiaDB集群安装节点，FS节点表示通过SequoiaFS映射挂载目录的节点。   

两者可以是相同节点也可以是不同节点，可以在一个FS节点上的不同目录下，映射挂载同一DB集群节点或不同DB集群节点，也可以在不同FS节点进行映射挂载同一DB集群节点。  

以下例子中DB节点为一常见普通集群，部署了一个coord、三个catalog和三个data节点，在FS节点上利用SequoiaFS挂载映射目录。

####1、在DB节点上创建目标集合####
首次启动时，需要在远程DB节点上创建映射的目标集合collection。后面挂载目录之后，mountpoint目录下的所有文件的实际内容会以lob的形式存放在该集合下。而所有文件的属性信息会分别存放在目录元数据集合和文件元数据集合中。

```lang-javascript
$sdb
Welcome to SequoiaDB shell!
help() for help, Ctrl+c or quit to exit
> var db = new Sdb("localhost", 11810) 
Takes 0.124118s.
> db.createCS("foo")
localhost:11800.foo
Takes 0.352408s.
> db.foo.createCL("bar")
localhost:11800.foo.bar
Takes 2.466226s.
>  
```

####2、在FS节点上创建挂载目录及配置文件####
挂载目录mountpoint为FS节点上的目录，用以挂载映射远程DB节点的目标集合，所以需要在FS节点上创建该目录。

启动SequoiaFS时可以指定从配置文件中读取配置参数，建议首次启动前创建配置文件并进行参数设置，配置文件及日志路径建议参考[配置文件规则](sequoiafs/usage/mount.md#配置文件及日志路径规则)进行设置，以防止出现多次映射时互相覆盖的情况。

```lang-javascript
$mkdir -p /opt/sequoiadb/mountpoint
$mkdir -p /opt/sequoiafs/conf/foo_bar/001/
$mkdir -p /opt/sequoiafs/log/foo_bar/001/
```
该例中按照默认参数值进行启动，所以不对参数进行配置，只是创建一个空配置文件，实际使用时按需写入相关配置值。

```lang-javascript
$touch /opt/sequoiafs/conf/foo_bar/001/sequoiafs.conf
```

####3、挂载目录 ####

挂载目录时，除了目标集合collection外，还需要指定一系列参数，具体参数选项详情请查看[选项](sequoiafs/usage/mount.md#选项)。
。

通过-i或者--hosts进行指定远程DB节点（协调节点），一旦挂载之后，mountpoint目录下的所有文件的属性信息会存放在远程DB节点上的目录元数据集合及文件元数据集合中，而文件内容会以lob的形式存放在目标集合下。目录元数据集合和文件元数据集合可以分别通过-d(或--metadircollection)和-f（或--metafilecollection）在进行指定，也可以直接通过指定--autocreate默认生成，该例指定默认生成。

```lang-javascript
$sequoiafs /opt/sequoiadb/mountpoint -i localhost:11810 -l foo.bar --autocreate -c /opt/sequoiafs/conf/foo_bar/001/ --diagpath  /opt/sequoiafs/log/foo_bar/001/ -o big_writes -o auto_unmount -o max_write=131072 -o max_read=131072
```
这里除了SequoiaFS相关参数，还指定了FUSE的参数-o big_writes（开启大页写）和-o auto_unmount（强杀进程时自动unmount目录），具体参数详情可以参见[FUSE选项](sequoiafs/usage/mount.md#FUSE选项)。
 

####4、查看挂载信息 ####
####4.1 本地FS节点通过mount可以看到挂载信息####

```lang-javascript
$ mount
/dev/sda1 on / type ext4 (rw,errors=remount-ro)
proc on /proc type proc (rw,noexec,nosuid,nodev)
sysfs on /sys type sysfs (rw,noexec,nosuid,nodev)
none on /sys/fs/fuse/connections type fusectl (rw)
none on /sys/kernel/debug type debugfs (rw)
none on /sys/kernel/security type securityfs (rw)
udev on /dev type devtmpfs (rw,mode=0755)
devpts on /dev/pts type devpts (rw,noexec,nosuid,gid=5,mode=0620)
tmpfs on /run type tmpfs (rw,noexec,nosuid,size=10%,mode=0755)
none on /run/lock type tmpfs (rw,noexec,nosuid,nodev,size=5242880)
none on /run/shm type tmpfs (rw,nosuid,nodev)
sequoiafs on /opt/sequoiadb/mountpoint type fuse.sequoiafs (rw,nosuid,nodev,user=sdbadmin)
```
可以看到，/opt/sequoiadb/mountpoint已经通过sequoiafs已经挂载上了，文件系统类型为fuse.sequoiafs。

####4.2 在DB节点可以查看相关信息####

```lang-javascript
> var db = new Sdb("localhost", 11810) 
Takes 0.001705s.
> db.list(4)
{
  "Name": "sequoiafs.maphistory"
}
{
  "Name": "sequoiafs.sequenceid"
}
{
  "Name": "sequoiafs.bar_dir148139183721030"
}
{
  "Name": "sequoiafs.bar_file148139183721030"
}
{
  "Name": "foo.bar"
}
```

对于每次mount，可以通过以上5张表查看相关信息，后续会介绍各表的作用，sequoiafs.maphistory为映射挂载历史信息表，记录历史挂载的关键数据信息。  


```lang-javascript
>  db.sequoiafs.maphistory.find()
{
  "_id": {
    "$oid": "5aff94db15d4f9e718e723cd"
  },
  "SourceCL": "foo.bar",
  "DirMetaCL": "sequoiafs.bar_dir148139183721030",
  "FileMetaCL": "sequoiafs.bar_file148139183721030",
  "Address": "eth0:192.168.20.45;",
  "MountPoint": "/opt/sequoiadb/mountpoint",
  "MountTime": {
    "MountTime": "2018-05-19-11.07.07.866247"
  }
}
```
每次挂载时都会记录一条历史数据，以供历史查询，其基本含义如下：    

|记录名称   | 描述说明                 |
|-----------|--------------------------|
|SourceCL  	| 目标映射集合名称	       |
|DirMetaCL	| 目录元数据集合名称       |
|FileMetaCL	| 文件元数据集合名称       |
|Address	| FS节点地址        	   |
|MountPoint | FS节点挂载时的目录       |
sequoiafs.sequenceid为目录元数据中目录记录的id序列表，目的用于构造目录的唯一性。  

sequoiafs.bar_dir148139183721030和sequoiafs.bar_file148139183721030分别为目录和文件的元数据集合表，由于SequoiaFS启动挂载时指定了--autocreate，所以这里是默认生成的，用以记录FS挂载目录下的目录和文件信息。

####4.3 在FS节点挂载目录下创建文件和目录####

```lang-javascript
$ cd /opt/sequoiadb/mountpoint/
$ touch testfile
$ echo 'hello, this is a testfile!' >> testfile
$ cat testfile 
hello, this is a testfile!
$ mkdir testdir
$ ls
testdir  testfile
``` 
上面我们在FS挂载目录下创建了文件testfile并写入'hello, this is a testfile!'，并创建了子目录testdir。在DB节点查看目录元数据集合，可以查到testdir目录元数据信息记录。

```lang-javascript
> db.sequoiafs.bar_dir148139183721030.find()
{
  "_id": {
    "$oid": "5affae7115d4f9e718e723d0"
  },
  "Name": "testdir",
  "Mode": 16877,
  "Uid": 2109,
  "Gid": 2000,
  "Pid": 1,
  "Id": 621,
  "NLink": 0,
  "Size": 4096,
  "CreateTime": 1526705777945,
  "ModifyTime": 1526705777945,
  "AccessTime": 1526705777945,
  "SymLink": ""
}
Return 1 row(s).
Takes 0.019212s.
```
目录元数据信息的具体含义如下：

|记录名称   | 描述说明             |数据类型|
|-----------|----------------------|--------|
|_id  	| 对象ID	       	       | OID  |
|Name	| 目录名称      	       |字符串|
|Mode	| 目录属性模式             |整数|
|Uid	| 目录属主	               |整数|
|Gid    | 目录组属主               |整数|
|Pid  	| 目录父目录ID，不同于_id  |长整数|
|Id	    | 目录ID                   |长整数|
|NLink	| 目录link                 |整数|
|Size	| 目录大小	               |长整数|
|CreateTime | 创建时间             |长整数|
|ModifyTime | 修改时间	           |长整数|
|AccessTime	| 访问时间             |长整数|
|SymLink	| 软链接               |字符串|


DB节点查看文件元数据集合，可以查到testfile文件元数据信息记录。  

```lang-javascript
> db.sequoiafs.bar_file148139183721030.find()
{
  "AccessTime": 1526705729062,
  "CreateTime": 1526705729000,
  "Gid": 2000,
  "LobOid": "5affae4015d4f9e718e723ce",
  "Mode": 33188,
  "ModifyTime": 1526705729062,
  "NLink": 1,
  "Name": "testfile",
  "Pid": 1,
  "Size": 27,
  "SymLink": "",
  "Uid": 2109,
  "_id": {
    "$oid": "5affae4015d4f9e718e723cf"
  }
}
Return 1 row(s).
Takes 0.010137s.
> db.foo.bar.listLobs()
{
  "Size": 27,
  "Oid": {
    "$oid": "5affae4015d4f9e718e723ce"
  },
  "CreateTime": {
    "$timestamp": "2018-05-19-12.55.28.833000"
  },
  "ModificationTime": {
    "$timestamp": "2018-05-19-12.56.08.073000"
  },
  "Available": true,
  "HasPiecesInfo": false
}
```
文件元数据信息具体含义如下：

|记录名称   | 描述说明             |数据类型|
|-----------|----------------------|--------|
|_id  	| 对象ID	       	       | OID  |
|Name	| 文件名称      	       |字符串|
|Mode	| 文件属性模式             |整数|
|Uid	| 文件属主	               |整数|
|Gid    | 文件组属主               |整数|
|Pid  	| 文件父目录ID，不同于_id  |长整数|
|LobOid | 文件对应lob对象ID        |字符串|
|NLink	| 文件link数               |整数|
|Size	| 文件大小	               |长整数|
|CreateTime | 创建时间             |长整数|
|ModifyTime | 修改时间	           |长整数|
|AccessTime	| 访问时间             |长整数|
|SymLink	| 软链接               |字符串|

从上表可以看出，文件元数据和目录元数据大致相同，不同的是，文件实际对应着一个Lob文件（通过LobOid映射到该文件），以保存文件的实际内容。并且文件没有ID属性，因为文件只从属于某个目录，所以只需要PID属性。

>**注意**  
>以上5张表使用时，最好通过SequoiaFS映射目录进行操作，如果需要通过DB客户端进行操作时，变更元数据信息时，数据结构要符合以上表格中的各记录数据类型，否则FS文件系统会读取异常。

接下来，即可在/opt/sequoiadb/mountpoint/目录下进行一系列文件操作，如创建删除文件，写入读取文件以及修改文件属性等。

>**说明**  
>关于系统命令，支持基于以上接口的一些常见系统命令如mkdir、vi、cp、rm、touch、cat、mv、ln、chown、chmod、truncate等，超出以上接口之外的系统命令暂时不支持，如tar、unzip压缩等命令。    

 
###选项###
####通用选项####
| 参数                 |缩写| 描述                                                                                   | 默认值          | 是否必填 |
|----------------------|----|----------------------------------------------------------------------------------------|-----------------|----------|
|--help	               | -h	| 显示帮助信息                                                                           |        		   |          |
|--helpfuse			   |    | 显示fuse帮助信息，查看FUSE相关选项信息                                                 |        		   |          |
|--version	           | -v	| 显示版本信息                                                                           |        		   |          |
|--diaglevel	       | -g	| 设置日志级别，取值范围[0-5]	                                                         | 3      		   | 否       |
|--hosts	           | -i	| 指定需要映射的集合的所属主机节点地址（hostname:svcname），<br>用","分隔多个地址        | localhost:11810 | 否       |
|--username	           | -u	| 数据库用户名                                                                           |       		   | 否       |
|--passwd	           | -p	| 数据库密码                                                                             |       		   | 否       |
|--collection	       | -l	| 指定需要映射的集合全名                                                                 |       		   | 是       |
|--metafilecollection  | -f	| 指定文件元数据集合全名，默认根据目标映射集合<br>生成对应集合名称                       |       		   | 否       |
|--metadircollection   | -d	| 指定目录元数据集合全名，默认根据目标映射集合<br>生成对应集合名称                       |       		   | 否       |
|--connectionnum	   | -n	| 指定连接池最大支持连接数大小，取值范围[50-1000]                                                | 100    		   | 否       |
|--cachesize	       | -s	| 目录LRU缓存大小，单位M，取值范围[1-200]                                                | 2     		   | 否       |
|--confpath	           | -c	| 配置文件路径，默认为当前目录下的sequoiafs.conf                                         |       		   | 否       |
|--diagnum		       |    | 指定日志文件最大个数，-1表示无限制                                                     | 20    		   | 否       |
|--diagpath		       |    | 指定日志文件目录，默认当前目录下diaglog                                                |       		   | 否       |
|--autocreate		   |    | 如果未显示指定文件和目录元数据集合全名，即未指定-d和-f，<br>则需要指定该选项进行自动生成 |       		   | 否       |
|mountpoint	           | 	| 指定映射集合的目标挂载目录                                                             |       		   | 是       |

首次启动时，其中-l collection参数和mountpoint是必须指定的，collection为需要映射的目标集合名称，为目标SequoiaDB节点中创建的集合，需要提前在DB中创建好。目标SequoiaDB节点可以通过-i或者--hosts进行指定，一旦挂载之后，mountpoint目录下的所有文件的属性信息会存放在目标SequoiaDB节点上的目录元数据集合及文件元数据集合中，而文件内容会以lob的形式存放在目标集合下。目录元数据集合和文件元数据分别可以通过-d(或--metadircollection)和-f（或--metafilecollection）在启动时进行指定，也可以直接通过指定--autocreate默认生成。

####FUSE选项####
| 参数		                | 描述	
|---------------------------|-----------------------------------------
|-d –o debug		        | 启用调试输出（隐含-f选项）             |
|-f		                    | 前台运行模式	                         |
|-s		                    | 禁止多线程模式	                     |
|-o allow_other		        | 允许其他用户访问权限	                 |
|-o allow_root		        | 允许root用户访问权限	                 |
|-o auto_unmount		    | 进程终止后自动unmount文件系统	         |
|-o nonempty         		| 允许mount在为非空文件夹上	             |
|-o default-permissions		| 允许内核权限审查	                     |
|-o fsname=NAME      		| 指定文件系统名称	                     |
|-o subtype=NAME     		| 指定文件系统类别	                     |
|-o large_read         		| 指定大页读取	                         |
|-o max_read=N       		| 指定read请求的最大size	             |
|-o hard_remove       		| 立即删除，无隐藏文件	                 |
|-o use_ino           		| 文件系统设置inode	                     |
|-o readdir_ino        		| Readdir时候尝试填充d_ino	             |
|-o direct_io           	| 使用direct I/O	                     |
|-o kernel_cache       		| 允许内核缓存文件	                     |
|-o [no]auto_cache     		| 允许根据修改次数来缓存文件，默认关闭	 |
|-o umask=M          		| 指定文件权限mask	                     |
|-o uid=N             		| 指定文件owner	                         |
|-o gid=N             		| 指定文件group	                         |
|-o entry_timeout=T    		| 缓存文件名称的超时时间，默认1s	     |
|-o negative_timeout=T 		| 缓存删除文件名称的超时时间，默认0s	 |
|-o attr_timeout=T     		| 缓存文件属性的超时时间，默认1s	     |
|-o ac_attr_timeout=T   	| 自动设置缓存文件属性的超时时间，默认1s |
|-o noforget           		| Inodes缓存永存	                     |
|-o remember=T       		| 指定缓存inodes时间为T(默认0s)	         |
|-o nopath            		| 非必要不提供文件路径	                 |
|-o intr               		| 允许requests请求被中断	             |
|-o intr_signal=NUM    		| 中断时发送的信号量（默认10）	         |
|-o modules=Ml[:M2…]  		| 指定文件堆中的模块名称	             |
|-o max_write=N       		| 指定write请求的最大size	             |
|-o max_readahead=N  		| 指定最大readahead的size	             |
|-o max_background=N  		| 指定backgroud的最大请求数	             |
|-o congestion_threshold=N	| 指定内核congestion的阈值	             |
|-o async_read         		| 异步IO读，默认为异步	                 |
|-o sync_read          		| 同步IO读	                             |
|-o atomic_o_trunc     		| 允许open+truncate的原子操作	         |
|-o big_writes         		| 允许超过4KB页的写操作，最大32K	     |
|-o no_remote_lock     		| 关闭远程文件锁	                     |
|-o no_remote_flock    		| 关闭远程文件锁（BSD）	                 |
|-o no_remote_posix_lock 	| 不允许删除文件锁（POSIX）	             |
|-o [no_]splice_write    	| 利用splice写入到fuse设备中	         |
|-o [no_]splice_move   		| 当splice到fuse设备时move数据	         |
|-o [no_]splice_read        | 允许从fuse设备进行splice读取	         |
    
>**注意:**   
>1、sequoiafs对于fuse选项只需要关注allow_other、allow_root、large_read、max_read、max_write、big_writes等常见选项即可；   
>2、需要指定allow_other时，需要在/etc/fuse.conf配置中写入对应的配置项，如在/etc/fuse.conf插入一行"user_allow_other"，其他类似，具体可以查看fuse的使用方法；  
>3、初始化时最好带上参数-o big_writes和-o large_read, 指定大页读写以提升性能；   
>4、sequoiafs不支持splice_write、splice_move、splice_read选项。

###配置文件及日志路径规则###
因为SequoiaFS在同一个节点可以挂载映射同一套DB或者不同套DB的同一个目标集合或者不同目标集合，所以在创建配置文件及指定日志路径时，建议参考以下规则进行配置，以防止出现配置文件互相干扰覆盖或者日志文件互相覆盖的情况。     

配置文件路径及日志文件路径参考规则： 

```
/opt/sequoiafs/conf/collection/001/sequoiafs.conf
/opt/sequoiafs/log/collection/001/diaglog/sequoiafs.log
/opt/sequoiafs/conf/collection/002/sequoiafs.conf
/opt/sequoiafs/log/collection/002/diaglog/sequoiafs.log 
```
collection即SequoiaFS启动时指定的目标集合实际名称，001和002表示挂载映射次数，从而防止映射同一个目标映射集合时日志文件互相覆盖。

配置样本文件：  

如果SequoiaFS是通过run包进行安装的，则可以从安装目录conf/samples/下拷贝配置文件样本sequoiafs.conf到指定的配置路径下。否则可以通过手工创建sequoiafs.conf，并根据需要写入以下配置内容，参数具体值根据实际情况写入，不进行配置的参数可以不用写入进配置文件中，以下为SequoiaFS配置文件样本内容：  

```
# Coord addr (hostname1:servicename1,hostname2:servicename2,...)
hosts=localhost:11810

# User name of source sdb
username=sdbadmin

# User passwd of source sdb
passwd=sdbadmin

# The target collection that be mounted
collection=

# The dir meta collection, default: sequoiafs.xxx
metafilecollection=

# The file meta collection, default: sequoiafs.xxx
metadircollection=

# Max connection num of connection pool
connectionnum=100

# Cache size of directory meta records, default:2(unit:M), value range: [1-200]
cachesize=2

# The path of configure file
confpath=

# Diagnostic level, default:3, value range: [0-5]
diaglevel=3

# The max number of diagnostic log files, default:20, -1:unlimited
diagnum=20

# Diagnostic log file path
diagpath=

```

###API接口###
SequoiaFS现支持以下文件操作API：

|接口函数   | 参数                     | 描述          
|-----------|--------------------------|-----------------------------------------------------------------------------------------------------------
|opendir()	| const char *name	       | 打开目录文件                                                                                             |
|readdir()	| DIR *dir	               | 读取目录文件                                                                                             |
|closedir()	| DIR *dir	               | 关闭目录文件                                                                                             |
|open()	    | const char *pathname	   | 创建或打开一个文件，flags只支持O_RDONLY, O_WRONLY, O_CREATE, <br>其他报错。忽略可选参数mode，默认权限644。   |
|           | int flags	               |                                                                                                          |
|           | [mode_t mode]	           |                                                                                                          |
|close()	| int fd	               | 关闭文件                                                                                                 |
|remove()	| const char *pathname	   | 删除文件                                                                                                 |
|lseek()	| FILE *stream	           | 设置读写偏移                                                                                             |
|           | long offset	           |                                                                                                          |
|           | int whence	           |                                                                                                          |
|read()	    | int fd	               | 读取文件数据                                                                                             |
|           | void *buf	               |                                                                                                          |
|           | size_t count	           |                                                                                                          |
|write()	| int fd	               | 写文件数据                                                                                               |
|           | const void* buf	       |                                                                                                          |
|           | size_t count	           |                                                                                                          |
|stat()	    | const char *pathname	   | 获取文件的属性信息                                                                                       |
|           | struct stat *buf	       |                                                                                                          |
|utime()	| const char * pathname	   | 更改访问和修改时间                                                                                       |
|           | struct utimebuf * buf	   |                                                                                                          |
|link()	    | const char *oldpath	   | 创建链接文件（硬链接）                                                                                   |
|           | const char *newpath	   |                                                                                                          |
|unlink()	| const char * pathname	   | 删除指定文件，如果该文件为最后的链接点，则文件会被删除。<br>如果为符号链接，则链接删除。                 |
|symlink()	| const char *oldpath	   | 创建符号链接文件, oldpath指定文件允许不存在。                                                            |
|           | const char *newpath	   |                                                                                                          |
|truncate()	| const char *pathname     | 截取文件内容，将path指定的文件大小改为参数length的大小，<br>如果原来文件比length大，则超过的部分会被删除。|
|           | off_t length	           |                                                                                                          |
|mkdir()	| const char *pathname     | 创建目录文件                                                                                             |
|           | mode_t  mode	           |                                                                                                          |
|rmdir()	| const char *pathname	   | 删除目录文件                                                                                             |
|rename     | const char *pathname     | 更改文件名称                                                                                             |
|           | const char *newpathname  |                                                                                                          |
|chmod      | const char *pathname     | 更改文件权限                                                                                             |
|           | mode_t mode              |

>**注意:**   
>open文件时，只支持指定单独的O_WRONLY或者O_RDONLY，不支持O_RDWR模式。


###API使用实例###
下面实例演示了通过API在mountpoint目录下简单地创建了一个testfile文件并写入testdata内容。  

```lang-javascript
#include<stdio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>

static char testdata[] = "abcdefghijklmnopqrstuvwxyz";
static int testdatalen = sizeof(testdata) - 1;
#define testfile "/opt/sequoiadb/mountpoint/testfile"

int main()
{
    int rc = 0;
    int fd = 0;
    const char *data = testdata;
    int datalen = testdatalen;

    fd = open(testfile,  O_WRONLY|O_CREAT);
    if(0 > fd)
    {
        printf("Failed to open file:%s\n", testfile);
        goto error;
    }

    rc = write(fd, data, datalen);
    if(0 > rc)
    {
        printf("Failed to write file:%s\n", testfile);
        goto error;
    }

    rc = lseek(fd, 4, SEEK_SET);
    if(0 > rc)
    {
        printf("Failed to lseek file:%s\n", testfile);
        goto error;
    }

    rc = write(fd, "DF", 2);
    if(0 > rc)
    {
        printf("Failed to write file:%s\n", testfile);
        goto error;
    }

    rc = close(fd);
    if(0 > rc)
    {
        printf("Failed to close file:%s\n", testfile);
        goto error;
    }
done:
    return rc;
error:
    goto done;
}     
```  



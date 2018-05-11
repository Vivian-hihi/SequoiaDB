
##选项##
##通用选项##
| 参数                 |缩写| 描述                                                                                   | 默认值          | 是否必填 |
|----------------------|----|----------------------------------------------------------------------------------------|-----------------|----------|
|--help	               | -h	| 显示帮助信息                                                                           |        		   |          |
|--helpfuse			   |    | 显示fuse帮助信息，查看FUSE相关选项信息                                                 |        		   |          |
|--version	           | -v	| 显示版本信息                                                                           |        		   |          |
|--diaglevel	       | -g	| 设置日志级别，取值范围[0-5]	                                                         | 3      		   | 否       |
|--hosts	           | -i	| 指定需要映射的集合的所属主机节点地址（hostname:svcname），用","分隔多个地址            | localhost:11810 | 否       |
|--username	           | -u	| 数据库用户名                                                                           |       		   | 否       |
|--passwd	           | -p	| 数据库密码                                                                             |       		   | 否       |
|--collection	       | -l	| 指定需要映射的集合全名                                                                 |       		   | 是       |
|--metafilecollection  | -f	| 指定文件元数据集合全名，默认根据目标映射集合生成对应集合名称                           |       		   | 否       |
|--metadircollection   | -d	| 指定目录元数据集合全名，默认根据目标映射集合生成对应集合名称                           |       		   | 否       |
|--connectionnum	   | -n	| 指定初始化连接池大小，取值范围[50-1000]                                                | 50    		   | 否       |
|--cachesize	       | -s	| 目录LRU缓存大小，单位M，取值范围[1-200]                                                | 2     		   | 否       |
|--confpath	           | -c	| 配置文件路径，默认安装目录下../conf/sequoiafs.conf                                     |       		   | 否       |
|--diagnum		       |    | 指定日志文件最大个数，-1表示无限制                                                     | 20    		   | 否       |
|--diagpath		       |    | 指定日志文件目录，默认当前目录下diaglog                                                |       		   | 否       |
|--autocreate		   |    | 如果未显示指定文件和目录元数据集合全名，即未指定-d和-f，则需要指定该选项进行自动生成   |       		   | 否       |
|mountpoint	           | 	| 指定映射集合的目标挂载目录                                                             |       		   | 是       |


##FUSE选项##
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
    
注意：sequoiafs对于fuse选项只需要关注allow_other、allow_root、large_read、max_read、max_write、big_writes等常见选项即可，需要指定allow_other时，需要在/etc/fuse.conf配置中写入对应的配置项，其他类似，具体可以查看fuse的使用方法。sequoiafs不支持splice_write、splice_move、splice_read选项。


###支持以下文件操作API###
|接口函数   | 参数                     | 描述          
|-----------|--------------------------|-----------------------------------------------------------------------------------------------------------
|opendir()	| const char *name	       | 打开目录文件                                                                                             |
|readdir()	| DIR *dir	               | 读取目录文件                                                                                             |
|closedir()	| DIR *dir	               | 关闭目录文件                                                                                             |
|open()	    | const char *pathname	   | 创建或打开一个文件，flags只支持O_RDONLY, O_WRONLY, O_CREATE, 其他报错。忽略可选参数mode，默认权限644。   |
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
|unlink()	| const char * pathname	   | 删除指定文件，如果该文件为最后的连接点，则文件会被删除。如果为符号链接，则链接删除。                     |
|symlink()	| const char *oldpath	   | 创建符号链接文件, oldpath指定文件允许不存在。                                                            |
|           | const char *newpath	   |                                                                                                          |
|truncate()	| const char *pathname     | 截取文件内容，将path指定的文件大小改为参数length的大小，如果原来文件比length大，则超过的部分会被删除。   |
|           | off_t length	           |                                                                                                          |
|mkdir()	| const char *pathname     | 创建目录文件                                                                                             |
|           | mode_t  mode	           |                                                                                                          |
|rmdir()	| const char *pathname	   | 删除目录文件                                                                                             |
|rename     | const char *pathname     | 更改文件名称                                                                                             |
|           | const char *newpathname  |                                                                                                          |
|chmod      | const char *pathname     | 更改文件权限                                                                                             |
|           | mode_t mode              |

注意：open文件时，只支持指定单独的O_WRONLY或者O_RDONLY，不支持O_RDWR模式。关于系统命令，支持基于以上接口的一些常见系统命令如mkdir、vi、cp、rm、touch、cat、mv、ln、chown、chmod、truncate等，超出以上接口之外的系统命令暂时不支持，如tar、unzip压缩等命令。

##使用##
###挂载目录###
(1)将sequoiadb的foo.bar集合映射到本地的mountpoint目录，指定文件元数据和目录元数据集合默认生成

```shell
$/opt/sequoiadb/bin/sequoiafs -i localhost:11810 -l foo.bar -c /opt/sequoiadb/conf --autocreate /opt/sequoiadb/mountpoint
```
即可在mountpoint目录下进行普通文件操作，例如文件的创建、删除和读写。

```shell
$touch testfile
$echo "hello, this is a testfile!" >> testfile
$mkdir testdir
```
![](sequoiafs/mount.png)   
除了用系统命令进行文件和目录的操作外，还可以利用通用的文件操作API进行操作，支持的API如上表。

```c
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

同样，也可以在多个不同节点映射同一个集合，这样在多个节点上进行操作的是同一套文件系统，即某个节点创建删除的文件，在所有映射的节点目录下都会进行同步。

###卸载目录###
卸载目录可以使用fuse自带的程序fusermount

```shell
$fusermount -u /opt/sequoiadb/mountpoint
```
也可以直接kill程序

```shell
$ps -ef | grep sequoiafs
$kill 程序PID
```


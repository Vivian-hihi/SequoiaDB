###卸载目录###
卸载目录可以通过fusermount程序指定-u来进行，也可以直接kill掉sequoiafs进程。   

####1、fusermount卸载####

```lang-javascript
$fusermount -u /opt/sequoiadb/mountpoint
```
####2、kill进程####


```lang-javascript
$ps -ef | grep sequoiafs
$kill 程序PID
```
注意：如果使用kill -9进行强杀进程，进程结束后会导致原mountpoint目录无法被linux文件系统正常访问的情况，为了避免这种情况，可以在sequoiafs初始化的时候，指定带上fuse选项-o auto_unmount。
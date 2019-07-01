##描述##

有些配置项修改后不能直接重启节点，还需将相应的文件进行转移。如dbpath、indexpath、lobpath、lobmetapath。以indexpath修改为例。

>   **Note:**
>
>   dbpath、lobpath、lobmetapath与indexpath修改一致。<br>
>   主节点和从节点修改没有差异。主节点关闭，之后会有一个选主的过程，大约几秒钟，选主成功后原来的主节点会变成从节点。

##indexpath修改##

索引文件默认存储路径为:/opt/sequoiadb/database/data/11820，修改为：/opt/sequoiadb/database/data/11820/indexpath。

1. 关闭要修改配置的节点11820。

  ```lang-javascript
  $ sdbstop -p 11820
  ```

2. 进入该节点索引文件所在位置，创建新的索引目录indexptah。将该目录下原有索引文件进行转移。

```lang-bash
  $ cd /opt/sequoiadb/database/data/11820
  $ mkdir indexpath
  $ mv *.idx indexpath/
  ```

 >   **Note:**
 >
 >   注意目录的权限要与之前的保持一致。

3. 进入该节点的配置文件所在位置，重新配置参数。将indexpath修改为/opt/sequoiadb/database/data/11820/indexpath。

  ```lang-bash
  $ cd /opt/sequoiadb/conf/local/11820
  $ vim sdb.conf
  ```

  修改配置文件如下：

  ```lang-ini
  ...
  indexpath=/opt/sequoiadb/database/data/11820/indexpath
  ...
  ```

4. 重新启动节点。

  ```lang-javascript
  $ sdbstart -p 11820
  ```

5. 连接协调节点11810，使用快照查看节点11820的配置参数。

  ```lang-javascript
  > var db=new Sdb("localhost",11810)
  > db.snapshot(SDB_SNAP_CONFIGS,{"svcname":"11820"},{"indexpath":""})
  {
  "indexpath": "/opt/sequoiadb/database/data/11820/indexpath/"
  }
  ```
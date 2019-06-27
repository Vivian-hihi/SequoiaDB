##描述##

有些配置项修改后不能直接重启节点，还需将相应的文件进行转移。如dbpath、indexpath、lobpath、lobmetapath。以indexpath修改为例。

>   **Note:**
>
>   dbpath、lobpath、lobmetapath与indexpath修改一致。

##indexpath修改##

索引文件默认存储路径为:/opt/sequoiadb/database/data/11820，修改为：/opt/sequoiadb/database/data/11820/indexpath。

1. 连接到集群管理服务进程，关闭要修改配置的节点11820。

  ```lang-javascript
  > var oma = new Oma( "localhost", 11790 )
  > oma.stopNode(11820)
  ```

2. 进入该节点索引文件所在位置，创建新的索引目录indexptah。将该目录下原有索引文件进行转移。

  ```lang-bash
  $ cd /opt/sequoiadb/database/data/11820
  $ mkdir indexpath
  $ mv *.idx indexpath/
  ```

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
  > oma.startNode(11820)
  ```

5. 连接协调节点11810，使用快照查看节点11820的配置参数。

  ```lang-javascript
  > var db=new Sdb("localhost",11810)
  > db.snapshot(SDB_SNAP_CONFIGS,{"svcname":"11820"},{"indexpath":""})
  {
  "indexpath": "/opt/sequoiadb/database/data/11820/indexpath/"
  }
  ```
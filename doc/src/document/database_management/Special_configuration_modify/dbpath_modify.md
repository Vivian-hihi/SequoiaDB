##dbpath修改##

数据文件默认存储路径为：/opt/sequoiadb/database/data/11820，修改为：/opt/sequoiadb/database/data/11820/dbpath。indexpath、lobpath、lobmetapath默认与数据文件存储路径相同。在没有指定它们路径的情况下，改变数据文件存储路径，不仅要将数据文件进行转移，还要将索引文件，大对象文件一并转移，如果指定了路径，则不需要转移。以下均为默认存储路径。

1. 关闭要修改配置的节点11820。

  ```lang-javascript
  $ sdbstop -p 11820
  ```

2. 进入该节点数据文件所在位置，创建新的数据文件存储目录dbptah。将原有的数据文件*.data、索引文件*.idx、大对象数据文件*.lobd、大对象元数据文件*.lobm进行转移。

  ```lang-bash
  $ cd /opt/sequoiadb/database/data/11820
  $ mkdir dbpath
  $ chown -R sdbadmin:sdbadmin_group dbpath
  $ mv *.data *.idx *.lobd *.lobm dbpath/
  ```

 >   **Note:**
 >
 >   系统默认情况下没有创建大对象文件。<br>
 >   注意新创建的目录要与之前的目录权限保持一致，可以通过chown -R sdbadmin:sdbadmin_group dbpath来保证。其中sdbadmin:sdbadmin_group为启动sequoiadb的用户名和用户组。

3. 进入该节点的配置文件所在位置，重新配置参数。将dbpath修改为/opt/sequoiadb/database/data/11820/dbpath。

  ```lang-bash
  $ cd /opt/sequoiadb/conf/local/11820
  $ vim sdb.conf
  ```

  修改配置文件如下：

  ```lang-ini
  ...
  dbpath=/opt/sequoiadb/database/data/11820/dbpath
  ...
  ```

4. 重新启动节点。

  ```lang-javascript
  $ sdbstart -p 11820
  ```

5. 连接协调节点11810，使用快照查看节点11820的配置参数。

  ```lang-javascript
  > var db=new Sdb("localhost",11810)
  > db.snapshot(SDB_SNAP_CONFIGS,{"svcname":"11820"},{"dbpath":""})
  {
  "dbpath": "/opt/sequoiadb/database/data/11820/dbpath/"
  }
  ```
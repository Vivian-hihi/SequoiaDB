##lobpath修改##

大对象数据文件默认存储路径与数据文件相同:/opt/sequoiadb/database/data/11820。修改为：/opt/sequoiadb/database/data/11820/lobpath。

1. 关闭要修改配置的节点11820。

  ```lang-javascript
  $ sdbstop -p 11820
  ```

2. 进入该节点大对象数据文件所在位置，创建新的索引目录lobptah。将原有的大对象数据文件*.lobd和大对象元数据文件*.lobm进行转移。

  ```lang-bash
  $ cd /opt/sequoiadb/database/data/11820
  $ mkdir   lobpath
  $ chown -R sdbadmin:sdbadmin_group lobpath
  $ mv *.lobd  *.lobm lobpath/
  ```

  >   **Note:**
  >
  >   lobmetapath默认与lobpath保持一致，所以在修改lobpath的时候要将大对象元数据文件*.lobm一并转移，如果指定了lobmetapath，则不需要转移。
  >   注意新创建的目录要与之前的目录权限保持一致，可以通过chown -R sdbadmin:sdbadmin_group lobpath来保证。其中sdbadmin:sdbadmin_group为启动sequoiadb的用户名和用户组。

3. 进入该节点的配置文件所在位置，重新配置参数。将lobpath修改为/opt/sequoiadb/database/data/11820/lobpath。

  ```lang-bash
  $ cd /opt/sequoiadb/conf/local/11820
  $ vim sdb.conf
  ```

  修改配置文件如下：

  ```lang-ini
  ...
    lobpath=/opt/sequoiadb/database/data/11820/lobpath
  ...
  ```

4. 重新启动节点。

  ```lang-javascript
  $ sdbstart -p 11820
  ```

5. 连接协调节点11810，使用快照查看节点11820的配置参数。

  ```lang-javascript
  > var db=new Sdb("localhost",11810)
  > db.snapshot(SDB_SNAP_CONFIGS,{"svcname":"11820"},{"lobpath":""})
  {
  "  lobpath": "/opt/sequoiadb/database/data/11820/  lobpath/"
  }
  ```
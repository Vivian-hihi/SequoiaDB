当前版本中，数据备份支持离线备份，即数据备份期间需要中断插入、更新、删除等变更操作，只支持查询操作。当前备份支持两种方式：全量备份和增量备份

-   全量备份：备份整个数据库的配置、数据和日志；
-   增量备份：在上一个全量备份或增量备份的基础上备份新增的日志和配置；

##离线备份参数说明##

  参数          说明
  ------------- ---------------------------------------------------------------------------------------------------------------
  Name          备份名称，缺省则以当前时间格式命名，如“2013-11-13-15:00:00”。
  Description   备份用户描述信息。
  Path          本次备份的指定路径，缺省为配置参数“bkuppath”中指定的路径。
  EnsureInc     备份方式，true 表示增量备份，false 表示全量备份，缺省为 false。
  OverWrite     对于同名备份是否覆盖，true 表示覆盖，false 表示不覆盖，如果同名则报错；缺省为 true。
  GroupName     对指定组进行备份，缺省为对全系统备份，当需要对多个组进行备份可以指定为数组类型，如：["datagroup1","datagroup2"]。

##备份整个数据库##

1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost",11810);</pre>

2.执行备份命令

<pre class="prettyprint lang-javascript">
> db.backupOffline({Name:"backupName",Description:"backup for all"})</pre>

##备份指定组的数据库##

1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost",11810);</pre>

2.执行备份命令

<pre class="prettyprint lang-javascript">
> db.backupOffline({Name:"backupName",Description:"backup group1",GroupName:"datagroup1"})</pre>

##备份指定节点的数据库##

1.连接到指定节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var dbdata = new Sdb("hostname1","servicename1");</pre>

2.执行备份命令

<pre class="prettyprint lang-javascript">
> dbdata.backupOffline({Name:"backupName",Description:"backup data node"})</pre>

**Note:**

catalog 编目组的名称固定为 SYSCatalogGroup

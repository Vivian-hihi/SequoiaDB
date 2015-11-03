备份信息查看可以通过客户端和手工查看。

##查看备份信息参数说明##

  参数        说明
  ----------- -----------------------------------------------------------------------------------------------------------------------------
  Name        备份名称，缺省则查看目录下所有备份信息。
  Path        查看备份的指定路径，缺省为配置参数“bkuppath”中指定的路径。
  GroupName   查看指定组的备份信息，缺省为查看全系统备份信息，当需要查看多个组的备份信息可以指定为数组类型，如：["datagroup1","datagroup2"]。

##查看全系统备份信息##

1.连接到协调节点

  <pre class="prettyprint lang-javascript">
  $ /opt/sequoiadb/bin/sdb
  > var db = new Sdb("localhost",11810);</pre>

2.执行查看备份信息命令

  <pre class="prettyprint lang-javascript">
  > db.listBackup()
  {
    "Name": "test_bk",
    "NodeName": "vmsvr2-suse-x64-1:11800",
    "GroupName": "SYSCatalogGroup",
    "EnsureInc": false,
    "BeginLSNOffset": 0,
    "EndLSNOffset": 18744,
    "StartTime": "2013-11-13-16:06:31",
    "HasError": false
  }
  {
    "Name": "test_bk",
    "NodeName": "vmsvr2-suse-x64-1:11820",
    "GroupName": "db1",
    "EnsureInc": false,
    "BeginLSNOffset": 0,
    "EndLSNOffset": 920424,
    "StartTime": "2013-11-13-16:06:31",
    "HasError": false
  }</pre>

##查看指定名称的备份信息##

1.连接到协调节点

  <pre class="prettyprint lang-javascript">
  $ /opt/sequoiadb/bin/sdb
  > var db = new Sdb("localhost",11810);</pre>

2.执行查看备份信息命令

  <pre class="prettyprint lang-javascript">
  > db.listBackup({Name:"backup1"})
  {
    "Name": "backup1",
    "NodeName": "vmsvr2-suse-x64-1:11820",
    "GroupName": "group1",
    "EnsureInc": false,
    "BeginLSNOffset": 0,
    "EndLSNOffset": 108744,
    "StartTime": "2013-11-13-16:06:31",
    "HasError": false
  }</pre>

##手工查看备份信息##

手工查看备份信息直接通过终端登入指定机器，并进入到相应的备份目录中，执行“ls -l”

  <pre class="prettyprint lang-javascript">
  use@vmsvr2-suse-x64-1:/opt/sequoiadb/database/11820/bakfile> ls -l
  total 37328
  -rw-r----- 1 sdbadmin sdbadmin  38157784 Nov 13 16:06 test_bk.1
  -rw-r----- 1 sdbadmin sdbadmin     65536 Nov 13 16:06 test_bk.bak</pre>

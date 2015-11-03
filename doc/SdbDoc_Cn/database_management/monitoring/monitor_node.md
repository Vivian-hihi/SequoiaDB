用户可以使用 snapshot 监控每个节点的状态。

1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost", 11810) ;</pre>

2.得到分区组

<pre class="prettyprint lang-javascript">
> datarg = db.getRG ( "< datagroup1 >" ) ;</pre>

3.得到数据节点

<pre class="prettyprint lang-javascript">
> datanode = datarg.getNode ( "< hostname1 >", "< servicename1 >" ) ;</pre>

4.得到该节点的快照

<pre class="prettyprint lang-javascript">
> datanode.connect().snapshot(SDB_SNAP_DATABASE)</pre>

快照类型分为：

[SDB_SNAP_CONTEXTS](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_CONTEXTS_CURRENT](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_SESSIONS](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_SESSIONS_CURRENT](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_COLLECTIONS](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_COLLECTIONSPACES](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_DATABASE](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_SYSTEM](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_CATALOG](SdbDoc_Cn/database_management/monitoring/snapshot.html)

用户可以使用 Shell 脚本监控，例如：

<pre class="prettyprint lang-javascript">
[sequoiadb@vmsvr1-rhel-x64 sequoiadb]$ cat monitor_insert.sh
#!/bin/bash
~/sequoiadb/bin/sdb "db=new Sdb('localhost', 11810)" > /dev/null
~/sequoiadb/bin/sdb "db.getRG('foo').getNode('vmsvr1-rhel-x64',11820).connect().snapshot(SDB_SNAP_DATABASE)" | grep TotalInsert
~/sequoiadb/bin/sdb "quit"
[sequoiadb@vmsvr1-rhel-x64 sequoiadb]$ ./monitor_insert.sh
"TotalInsert": 0,</pre>

启动 Hive Shell 命令行窗口，执行如下命令创建数据表；
<pre class="prettyprint lang-javascript">
hive> create external table sdb_tab(id INT, name STRING, value DOUBLE) stored by "com.sequoiadb.hive.SdbHiveStorageHandler" tblproperties("sdb.address" = "localhost:11810");

OK
Time taken: 0.386 seconds</pre>

其中 sdb.address 用于指定 SequoiaDB 协调节点的 IP 和端口，如果有多个协调节点，可以写入多个，之间用逗号隔开。表的数据库对应 SequoiaDB 的集合空间，表对应集合空间中的集合。

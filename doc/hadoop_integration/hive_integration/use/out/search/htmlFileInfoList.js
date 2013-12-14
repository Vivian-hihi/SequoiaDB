fil = new Array();
fil["0"]= "create.html@@@创建基于SequoiaDB的表@@@启动hive shell命令行窗口，执行如下命令创建数据表； hive> create external table sdb_tab(id INT, name STRING, value DOUBLE) stored by  com.sequoiadb.hive.SdbHiveStorageHandler  tblproperties( sdb.address...";
fil["1"]= "inquiry.html@@@查询数据@@@hive> select * from new_tab; OK 0 false 0.0 ALGERIA 1 true 1.0 ARGENTINA 2 true 1.0 BRAZIL 3 true 1.0 CANADA 4 true 4.0 EGYPT 5 false 0.0 ETHIOPIA 6 true 3.0 FRANCE 7 true 3.0 GERMANY 8 true 2.0 INDI...";
fil["2"]= "insert.html@@@从HDFS文件中导入数据到SequoiaDB表@@@hive> insert overwrite table sdb_tab select * from hdfs_tab; Total MapReduce jobs = 1 Launching Job 1 out of 1 Number of reduce tasks is set to 0 since there s no reduce operator Starting Job...";

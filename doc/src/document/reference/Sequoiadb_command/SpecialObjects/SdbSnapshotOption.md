统一指定 cond/sel/sort/skip/limit/options 参数。


##语法##

***SdbSnapshotOption.cond([cond]).sel([sel]).sort([sort]).options([options]).skip([skipNum]).limit([retNum])***

##参数描述##

| 参数名 			| 参数类型 	| 描述 		| 是否必填 |
| ------ 			| ------ 	| ------ 	| ------   |
| cond 				| Json 对象 | 选择条件，只返回 cond 字段指定的节点或分区组的快照信息，为 null 时，返回整个集群的快照信息。 	| 否 |
| sel 				| Json 对象 | 选择返回字段名。为 null 时，返回所有的字段名。 	| 否 |
| sort 				| Json 对象 | 对返回的记录按选定的字段排序。1为升序；-1为降序。 | 否 |
| options|	Json 对象 | 指定快照参数，因不同快照类型而异，在对应[快照类型](database_management/monitoring/snapshot/snapshot.md)查看选项及示例。目前拥有快照参数的有[配置快照](database_management/monitoring/snapshot/SDB_SNAP_CONFIGS.md)。| 否 |
| skipNum | int | 自定义结果集从哪条记录返回。 | 否 |
| retNum  |	int | 自定义返回结果集的记录条数。 | 否 |



> **Note：**

>* sel 参数是一个json结构，如：{字段名:字段值}，字段值一般指定为空串。sel中指定的字段名在记录中存在，设置字段值不生效；不存在则返回sel中指定的字段名和字段值。
>* 记录中字段值类型为数组的，我们可以在sel中指定该字段名，用"."操作符加上双引号("")来引用数组元素。
> skip() 方法的定义格式包含 skipNum 参数，它是 int 类型。如果不设定 num 的内容或者设定 num 的值为0，相当于返回所有的结果集；如果想从结果集的第3条记录开始返回，可是设置 num 的值等于2。
> 2. limit() 方法的定义格式包含 retNum 参数，它是 int 类型。如果不设定 num 的内容，相当于返回所有的结果集记录。如果想返回结果集的前5条记录，可是设置 num 的值为5。


##返回值##

返回自身，类型为 SdbSnapshotOption。

##错误##

[错误码](reference/Sequoiadb_error_code.md)


##示例##

* 通过组名或组 ID 查询某个分区组的快照信息，如：

	```lang-javascript
    > var option = new SdbSnapshotOption().cond( { GroupName:'data1' } )
	> db.snapshot( SDB_SNAP_CONTEXTS, option )
	{
	  "SessionID": "vmsvr1-cent-x64-1:11820:22",
	  "Contexts": [
		{
		  "ContextID": 11,
		  "Type": "DUMP",
		  "Description": "BufferSize:0",
		  "DataRead": 0,
		  "IndexRead": 0,
		  "QueryTimeSpent": 0,
		  "StartTimestamp": "2013-12-28-16.13.57.864245"
		}
	  ]
	}
	{
	  "SessionID": "vmsvr1-cent-x64-1:11840:23",
	  "Contexts": [
		{
		  "ContextID": 10,
		  "Type": "DUMP",
		  "Description": "BufferSize:0",
		  "DataRead": 0,
		  "IndexRead": 0,
		  "QueryTimeSpent": 0,
		  "StartTimestamp": "2013-12-28-16.13.57.865103"
		}
	  ]
	}
	```

* 通过“组名+主机名+服务名”或“组 ID+节点 ID”查询某个节点的快照信息，如：

	```lang-javascript
    > var option = new SdbSnapshotOption().cond( { GroupName: 'data1', HostName: "vmsvr1-cent-x64-1", svcname: "11820" } )
	> db.snapshot( SDB_SNAP_CONTEXTS, option )
	{
	  "SessionID": "vmsvr1-cent-x64-1:11820:22",
	  "Contexts": [
		{
		  "ContextID": 11,
		  "Type": "DUMP",
		  "Description": "BufferSize:0",
		  "DataRead": 0,
		  "IndexRead": 0,
		  "QueryTimeSpent": 0,
		  "StartTimestamp": "2013-12-28-16.13.57.864245"
		}
	  ]
	}
	```

* 通过“主机名+服务名”查询某个节点的快照信息，如：

	```lang-javascript
    > var option = new SdbSnapshotOption().cond( { HostName: "ubuntu-200-043", svcname: "11820" } )
	> db.snapshot( SDB_SNAP_CONTEXTS, option )
	{
	  "NodeName": "ubuntu-200-043:11820",
	  "SessionID": 18,
	  "Contexts": [
		{
		  "ContextID": 31,
		  "Type": "DUMP",
		  "Description": "IsOpened:1,HitEnd:0,BufferSize:0",
		  "DataRead": 0,
		  "IndexRead": 0,
		  "QueryTimeSpent": 0,
		  "StartTimestamp": "2016-10-27-17.53.45.042061"
		}
	  ]
	}
	```

* 查看数据组 db1 中数据节点 20000 上配置文件中的配置信息

	```lang-javascript
	> var option = new SdbSnapshotOption().cond( { GroupName:'db1', SvcName:'20000' } ).options( { "mode": "local", "expand": false } )
	> db.snapshot( SDB_SNAP_CONFIGS, option )
	{
  	"NodeName": "ubuntu-zwb:20000",
  	"dbpath": "/home/equoiadb/20000/",
  	"svcname": "20000",
  	"diaglevel": 3,
  	"role": "data",
  	"catalogaddr": "ubuntu-zwb:30003,ubuntu-zwb:30013,ubuntu-zwb:30023",
  	"perfstat": "FALSE",
  	"businessname": "yyy",
  	"clustername": "xxx"
	}
	Return 1 row(s).
	```
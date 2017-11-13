##语法##
***rg.getSlave([positions])***

获取当前分区组的从节点。

##参数描述##

| 参数名  | 参数类型  | 描述                         | 是否必填 |
| ------- | --------- | -----------------------------| -------- |
| positions    | int32    | 节点位置。目前，节点位置定义为该节点在 catalog 元数据的 "Group" 数组中的位置。其起始值为1，范围为[1,7]    | 否 |

##返回值##

返回分区组的从节点，类型为 Object 对象。

**Note:**

1. 当分区组只有一个节点，不管是否指定节点位置，直接返回唯一的节点对象。
2. 当分区组有多个节点，在不指定节点位置的情况下，返回的节点必为备节点。
3. 当分区组有多个节点，在指定节点位置的情况下，若节点位置包含备节点，优先随机返回包含的备节点。
4. 当分区组有多个节点，在指定节点位置的情况下，若指定的节点位置大于分区组的节点数，这些节点位置将按照公式 (position - 1) % nodeCount + 1 进行转换。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

1. 获取 group1 分区组的从节点。

	```lang-javascript
	> var rg = db.getRG("group1")
	> rg.getSlave()
	hostname1:42000
	```

2. group1 分区组信息如下：

	```lang-javascript
	> db.list(SDB_SNAP_SYSTEM, {"GroupName":"group1"})
	{
  	"Group": [
    	{
      	"HostName": "hostname1",
      	"Status": 1,
      	"dbpath": "/sequoiadb/database/40000/",
      	"Service": [
        	{
          	"Type": 0,
          	"Name": "40000"
        	},
        	{
          	"Type": 1,
          	"Name": "40001"
        	},
        	{
          	"Type": 2,
          	"Name": "40002"
        	}
      	],
      	"NodeID": 1001
    	},
    	{
      	"HostName": "hostname1",
      	"Status": 1,
      	"dbpath": "/sequoiadb/database/41000/",
      	"Service": [
        	{
          	"Type": 0,
          	"Name": "41000"
        	},
        	{
          	"Type": 1,
          	"Name": "41001"
        	},
        	{
          	"Type": 2,
          	"Name": "41002"
        	}
      	],
      	"NodeID": 1002
    	},
    	{
      	"HostName": "hostname1",
      	"Status": 1,
      	"dbpath": "/sequoiadb/database/42000/",
      	"Service": [
        	{
          	"Type": 0,
          	"Name": "42000"
        	},
        	{
          	"Type": 1,
          	"Name": "42001"
        	},
        	{
          	"Type": 2,
          	"Name": "42002"
        	}
      	],
      	"NodeID": 1003
    	}
  	],
  	"GroupID": 1001,
  	"GroupName": "group1",
  	"PrimaryNode": 1002,
  	"Role": 0,
  	"SecretID": 1425460557,
  	"Status": 1,
  	"Version": 7,
  	"_id": {
    	"$oid": "5a045460c517f3cf06a32976"
  	}
	}
	```
	
	其中，节点为：  
    hostname1:40000(备节点，节点位置为1)；  
    hostname1:41000(主节点，节点位置为2)；  
    hostname1:42000(备节点，节点位置为3)；  
	从 group1 分区组中，随机获取位置1和位置2节点中的备节点：

	```lang-javascript
	> var rg = db.getRG("group1")
	> rg.getSlave(1,2)
	hostname1:40000
	```

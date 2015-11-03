## 语法##
***db.listReplicaGroups()***

枚举分区组。

## 示例##

* 返回所有分区组信息，命令如下：

<pre class="prettyprint lang-javascript">
> db.listReplicaGroups()</pre>

返回：

<pre class="prettyprint lang-diy">
{
"Group": 
[
  {
    "dbpath": "/opt/sequoiadb/data/11800",
    "HostName": "vmsvr2-suse-x64",
    "Service": [
      {
        "Type": 0,
        "Name": "11800"
      },
      {
        "Type": 1,
        "Name": "11801"
      },
      {
        "Type": 2,
        "Name": "11802"
      },
      {
        "Type": 3,
        "Name": "11803"
      }
    ],
    "NodeID": 1000
  },
  {
    "dbpath": "/opt/sequoiadb/data/11850",
    "HostName": "vmsvr2-suse-x64",
    "Service": [
      {
        "Type": 0,
        "Name": "11850"
      },
      {
        "Type": 1,
        "Name": "11851"
      },
      {
        "Type": 2,
        "Name": "11852"
      },
      {
        "Type": 3,
        "Name": "11853"
      }
    ],
    "NodeID": 1001
  }
],
"GroupID": 1001,
"GroupName": "group",
"PrimaryNode": 1001,
"Role": 0,
"Status": 1,
"Version": 5,
"_id": {
  "$oid": "517b2fc33d7e6f820fc0eb57"
  }
}</pre>

这个分区组有有两个节点：11800和11850，其中11850为主节点。分区组详细信息请见分区组列表

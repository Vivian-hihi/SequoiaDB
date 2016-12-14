##语法##
***rg.attachNode(&lt;host&gt;,&lt;service&gt;, [options])***

将一个已经创建完成但不属于任何组的节点加入到当前组。搭配 rg.detachNode() 使用。

##参数描述##

参数名    参数类型    描述                        是否必填
--------- ----------- --------------------------- ----------
host      string      节点的主机名或者主机 IP。   是
service   string      节点服务名或者端口。        是
options   Json 对象   attachNode 选项。           否

##options 选项##

参数名     参数类型   描述                           默认值
---------- ---------- ------------------------------ --------
KeepData   bool       是否保留目标节点原有的数据。   false

**Note:**

如果目标节点原本不属于当前组，切勿打开 KeepData。

##常见的错误##

+----------+------------------------+------------------------------+
| 错误码   | 可能的原因             | 解决方法                     |
+==========+========================+==============================+
| -15      | 网络错误               | 1.检查 sdbcm 状态是否正常    |
|          |                        | 2.检查填写的 host 是否正确   |
+----------+------------------------+------------------------------+
| -146     | 节点不存在             | 检查目标节点是否被创建过     |
+----------+------------------------+------------------------------+
| -157     | 节点已存在于其他复制组 | 检查节点是否已属于其他复制组 |
+----------+------------------------+------------------------------+

##示例##

将一个节点从 datagroup1 中分离，加入到 datagroup2 中。

使用 db.listReplicaGroups() 查看当前节点信息；

<pre class="prettyprint lang-diy">
{
"Group": [
  {
    "HostName": "host1",
    "dbpath": "/data1",
    "Service": [
      {
        "Type": 0,
        "Name": "40060"
      },
      {
        "Type": 1,
        "Name": "40061"
      },
      {
        "Type": 2,
        "Name": "40062"
      }
    ],
    "NodeID": 1004
  },
  {
    "HostName": "host1",
    "dbpath": "/data2",
    "Service": [
      {
        "Type": 0,
        "Name": "40070"
      },
      {
        "Type": 1,
        "Name": "40071"
      },
      {
        "Type": 2,
        "Name": "40072"
      }
    ],
    "NodeID": 1005
  }
],
"GroupID": 1002,
"GroupName": "datagroup1",
"PrimaryNode": 1005,
"Role": 0,
"Status": 1,
"Version": 3,
"_id": {
  "$oid": "555d7b71d1cbaf20ed74e7df"
}
}

{
"Group": [
  {
    "HostName": "host1",
    "dbpath": "/data3",
    "Service": [
      {
        "Type": 0,
        "Name": "40040"
      },
      {
        "Type": 1,
        "Name": "40041"
      },
      {
        "Type": 2,
        "Name": "40042"
      }
    ],
    "NodeID": 1003
  }
],
"GroupID": 1001,
"GroupName": "datagroup2",
"PrimaryNode": 1003,
"Role": 0,
"Status": 1,
"Version": 4,
"_id": {
  "$oid": "555d7b5fd1cbaf20ed74e7de"
  }
}</pre>

分离“host:40060”节点；

<pre class="prettyprint lang-javascript">
> db.getRG('datagroup1').detachNode('host1', '40060');</pre>

将节点加入到 datagroup2 中；

<pre class="prettyprint lang-javascript">
> db.getRG('datagroup2').attachNode('host1', '40060')</pre>

使用 db.listReplicaGroups() 查看当前节点信息：

<pre class="prettyprint lang-diy">
{
  "Group": [
    {
      "HostName": "host1",
      "dbpath": "/data2",
      "Service": [
        {
          "Type": 0,
          "Name": "40070"
        },
        {
          "Type": 1,
          "Name": "40071"
        },
        {
          "Type": 2,
          "Name": "40072"
        }
      ],
      "NodeID": 1005
    }
  ],
  "GroupID": 1002,
  "GroupName": "datagroup1",
  "PrimaryNode": 1005,
  "Role": 0,
  "Status": 1,
  "Version": 3,
  "_id": {
    "$oid": "555d7b71d1cbaf20ed74e7df"
  }
}

{
  "Group": [
    {
      "HostName": "host1",
      "dbpath": "/data3",
      "Service": [
        {
          "Type": 0,
          "Name": "40040"
        },
        {
          "Type": 1,
          "Name": "40041"
        },
        {
          "Type": 2,
          "Name": "40042"
        }
      ],
      "NodeID": 1003
    },
    {
      "HostName": "host1",
      "dbpath": "/data1",
      "Service": [
        {
          "Type": 0,
          "Name": "40060"
        },
        {
          "Type": 1,
          "Name": "40061"
        },
        {
          "Type": 2,
          "Name": "40062"
        }
      ],
      "NodeID": 1006
    }
  ],
  "GroupID": 1001,
  "GroupName": "datagroup2",
  "PrimaryNode": 1003,
  "Role": 0,
  "Status": 1,
  "Version": 4,
  "_id": {
    "$oid": "555d7b5fd1cbaf20ed74e7de"
  }
}</pre>

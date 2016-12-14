## 语法##
***db.snapshot(&lt;snapType&gt;,[cond],[sel],[sort])***

枚举快照，快照是一种得到当前系统状态的命令。查看更多有关快照信息

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| snapType | 枚举 | 快照类型。 | 是 |
| cond | Json 对象 | 选择条件，只返回 cond 字段指定的节点或分区组的快照信息，为 null 时，返回整个集群的快照信息。 | 否 |
| sel | Json 对象 | 选择返回字段名。为 null 时，返回所有的字段名。 | 否 |
| sort | Json 对象 | 对返回的记录按选定的字段排序。1为升序；-1为降序。 | 否 |

## 格式##

snapshot() 方法定义格式有 snapType，cond两个字段，snapType 为枚举类型，cond 为 Json 对象，格式如下：

<pre class="prettyprint lang-diy">
{"snapType":"<快照类型>",["cond":"{"字段名1":{"操作符1":"值1"},"字段名2":{"操作符2":"值2"}...}"]}</pre>

**Note:**

* snapType 字段的值请参考快照类型
* sel 参数是一个 json 结构，字段名的值一般指定为空串。如果指定为如下结构：{"字段名1":"值1","字段名2":"值2",...}，对于记录中存在所选字段名的话，设定的值其实无效；对于记录中不存在所选字段名的话，返回{"字段名1":"值1","字段名2":"值2",...}
* 字段的值是数组的话，我们用“.”操作符引用数组内的元素。并加上双引号（""）

## 示例##

* 指定 snapType 的值为 SDB_SNAP_CONTEXTS：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS)</pre>

返回整个集群的上下文快照：

<pre class="prettyprint lang-diy">
{
  "SessionID": "vmsvr1-cent-x64-1:11820:22",
  "Contexts": [
    {
      "ContextID": 8,
      "Type": "DUMP",
      "Description": "BufferSize:0",
      "DataRead": 0,
      "IndexRead": 0,
      "QueryTimeSpent": 0,
      "StartTimestamp": "2013-12-28-16.07.59.146399"
    }
  ]
}
{
  "SessionID": "vmsvr1-cent-x64-1:11830:22",
  "Contexts": [
    {
      "ContextID": 6,
      "Type": "DUMP",
      "Description": "BufferSize:0",
      "DataRead": 0,
      "IndexRead": 0,
      "QueryTimeSpent": 0,
      "StartTimestamp": "2013-12-28-16.07.59.147576"
    }
  ]
}
{
  "SessionID": "vmsvr1-cent-x64-1:11840:23",
  "Contexts": [
    {
      "ContextID": 7,
      "Type": "DUMP",
      "Description": "BufferSize:0",
      "DataRead": 0,
      "IndexRead": 0,
      "QueryTimeSpent": 0,
      "StartTimestamp": "2013-12-28-16.07.59.148603"
    }
  ]
}</pre>

* 通过组名或组 ID 查询某个分区组的快照信息，如：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS,{GroupName:'data1'})</pre>

返回组名为“data1”的分区组快照信息

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS,{GroupID:1000})</pre>

返回组 ID 为“1000”的分区组快照信息
<pre class="prettyprint lang-diy">
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
}</pre>

* 通过“组名+主机名+服务名”或“组 ID+节点 ID”查询某个节点的快照信息，如：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS,{GroupName:'data1',HostName:"vmsvr1-cent-x64-1",svcname:"11820"});
> db.snapshot(SDB_SNAP_CONTEXTS,{GroupID:1000,NodeID:1001});</pre>

返回：

<pre class="prettyprint lang-diy">
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
}</pre>

* 通过“主机名+服务名”查询某个节点的快照信息，如：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS,{HostName:"vmsvr1-cent-x64-1",svcname:"11820"})</pre>

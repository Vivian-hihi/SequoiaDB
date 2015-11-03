##语法##
***query.explain([option])***

返回查询的访问计划。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| option | json 对象 | 访问计划执行参数，目前有 Run 字段项，表示是否执行访问计划，true 表示执行访问计划，获取数据和时间信息，false 表示只获取访问计划的信息，并不执行 | 否，默认为 false |

## 访问计划描述##

| 字段名 | 类型 | 描述 |
| ------ | ------ | ------ |
| Name | string | 集合名 |
| ScanType | string | 扫描方式—— 表扫描：“tbscan”； 索引扫描：“ixscan”  |
| IndexName | string | 使用索引的名称 |
| UseExtSort | bool | 是否使用非索引排序 |
| NodeName | string | 节点名 |
| ReturnNum | int64 | 返回记录数量 |
| ElapsedTime | float64 | 查询耗时（秒） |
| IndexRead | int64 | 索引记录扫描条数 |
| DataRead | int64 | 数据记录扫描条数 |
| UserCPU | float64 | 用户态 CPU 使用时间（秒） |
| SysCPU | float64 | 内核态 CPU 使用时间（秒） |
| SubCollections | Json Array | 垂直分区表中各子表访问计划 |

## 格式##

**Note:**

如果集合经过 split 分布在多个复制组，访问计划会按照一组一记录的方式返回。

## 示例##

* foo.bar 是一个水平分区集合，分布在三个复制组上。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find().sort({b:1}).explain({Run:true})</pre>

返回：
<pre class="prettyprint lang-diy">
{
  "Name": "foo.bar",
  "ScanType": "tbscan",
  "IndexName": "",
  "UseExtSort": true,
  "NodeName": "vmsvr2-cent-x64:40020",
  "ReturnNum": 38,
  "ElapsedTime": 0.000477,
  "IndexRead": 0,
  "DataRead": 38,
  "UserCPU": 0,
  "SysCPU": 0
}
{
  "Name": "foo.bar",
  "ScanType": "tbscan",
  "IndexName": "",
  "UseExtSort": true,
  "NodeName": "vmsvr2-cent-x64:40000",
  "ReturnNum": 34,
  "ElapsedTime": 0.000415,
  "IndexRead": 0,
  "DataRead": 34,
  "UserCPU": 0,
  "SysCPU": 0
}
{
  "Name": "foo.bar",
  "ScanType": "tbscan",
  "IndexName": "",
  "UseExtSort": true,
  "NodeName": "vmsvr2-cent-x64:40010",
  "ReturnNum": 28,
  "ElapsedTime": 0.000517,
  "IndexRead": 0,
  "DataRead": 28,
  "UserCPU": 0,
  "SysCPU": 0
}</pre>

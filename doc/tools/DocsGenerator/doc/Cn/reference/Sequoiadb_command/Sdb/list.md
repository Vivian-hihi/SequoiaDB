## 语法##
***db.list(&lt;listType&gt;,[con],[sel],[sort])***

枚举列表。列表是一种轻量级得到当前系统状态的命令。查看更多有关列表信息

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| listType | 枚举 | [列表类型](SdbDoc_Cn/database_management/monitoring/overview.html)。 | 是 |
| con | Json 对象 | 选择条件，只返回符合 con 字段值的记录，为 null 时，返回所有。 | 否 |
| sel | Json 对象 | 选择返回字段名。为 null 时，返回所有的字段名。 | 否 |
| sort | Json 对象 | 对返回的记录按选定的字段排序。1为升序；-1为降序。 | 否 |

## 格式##

list() 方法定义格式有 listType，con，sel，sort四个参数，listType 为枚举类型，其他全部为 Json 对象，格式如下：

<pre class="prettyprint lang-diy">
{"listType":"<列表类型>",["con":"{"字段名1":{"操作符1":"值1"},"字段名2":{"操作符2":"值2"}...}"],
["sel":"{"字段名1":"","字段名2":"",...}"],["sort":"{"字段名1":-1|1,"字段名2":1|-1,...}"]}</pre>

**Note:**

*  listType 字段的值请参考 [列表类型](SdbDoc_Cn/database_management/monitoring/overview.html)
*  sel 参数是一个 json 结构，字段名的值一般指定为空串。如果指定为如下结构：{"字段名1":"值1","字段名2":"值2",...}，对于记录中存在所选字段名的话，设定的值其实无效；对于记录中不存在所选字段名的话，返回{"字段名1":"值1","字段名2":"值2",...}
*  字段的值是数组的话，我们用"."操作符引用数组内的元素。并加上双引号("")

## 示例##

* 指定 listType 的值为 SDB_LIST_CONTEXTS：

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_CONTEXTS)</pre>

返回：

<pre class="prettyprint lang-diy">
{ 
  "SessionID": 4, 
  "Contexts": [ 0 ] 
}  ...</pre>

* 指定 listType 的值为 SDB_LIST_STORAGEUNITS：

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_STORAGEUNITS)</pre>

返回：

<pre class="prettyprint lang-javascript">
{ 
  "Name": "foo", 
  "ID": 4094, 
  "Logical ID": 1, 
  "PageSize": 4096, 
  "Sequence": 1, 
  "NumCollections": 1, 
  "CollectionHWM": 3, 
  "Size": 172032000 
}</pre>

* 返回符合条件 Logical ID 大于1的记录，并且每条记录只返回 Name 和 ID 这两个字段，记录按 Name 字段的值升序排序

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_STORAGEUNITS,{"Logical ID":{$gt:1}},{Name:"space",ID:2},{Name：1})</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "ID": 4091,
  "Name": "foo"
}
{
  "ID": 4093,
  "Name": "name"
}...</pre>

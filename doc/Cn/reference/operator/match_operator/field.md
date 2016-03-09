## 语法##

<pre class="prettyprint lang-diy">
{ <字段名1>: { $field: <字段名2> }, ...} 或者
{ <字段名1>: { <匹配符>: { $field: <字段名2> } }, ...}</pre>

## 描述##

$field 是字段符，选择满足“字段名1”匹配“字段名2”的记录。

## 示例##

* 返回集合 bar 中 t1 字段值大于 t2 字段的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find( { t1: { $gt: { $field: "t2" } } } )</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "521d5446e2d3c4e31c000000"
  },
  "t1": 3
  "t2": 0
}
...</pre>

* 返回集合 bar 中 t1 字段值等于 t2 字段的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find( { t1: { $field: "t2" } } )</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "521d5446e2d3c4e31c000001"
  },
  "t1": 100
  "t2": 100
}
...</pre>
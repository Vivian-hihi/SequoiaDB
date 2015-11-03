## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$lt:<值>}}</pre>

## 描述##

$lt 选择满足“字段名”的值小于（<）指定“值”的记录。

## 示例##

* 查询集合 bar 中字段名为 age，其值小于20的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$lt:20}})</pre>

* $lt 匹配嵌套对象中的字段名。使用 update() 方法更新嵌套对象 service 中的 ID 字段值小于2的记录，将这些记录的 age 字段值设定为25。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{age:25}},{"service.ID":{$lt:15}})</pre>

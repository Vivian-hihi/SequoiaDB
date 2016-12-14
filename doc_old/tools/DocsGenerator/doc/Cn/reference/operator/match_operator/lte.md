## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$lte:<值>}}</pre>

## 描述##

$lte 选择满足“字段名”的值小于等于（<=）指定“值”的记录。

## 示例##

* 选择查询集合空间 foo 下集合 bar 中字段名为 age 的值小于等于20的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$lte:20}})</pre>

* $lte 匹配一个嵌套对象中的字段名。使用 update() 方法更新嵌套对象 service 中的 ID 字段值小于等于2的记录，将这些记录的 age 字段值设定为25。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{age:25}},{"service.ID":{$lte:2}})</pre>

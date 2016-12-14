## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$et:<值>}}</pre>

## 描述##

$et 选择满足“字段名”的值等于（=）指定“值”的记录。

## 示例

* 返回集合 bar 中 age 字段值等于 20 的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$et:20}}) 等价于 db.foo.bar.find({age:20})</pre>

* $et 匹配嵌套对象中的字段名。使用 update() 方法更新嵌套对象 service 中的 type 字段值等于15的记录，将这些记录的 age 字段值设定为25。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{age:25}},{"service.type":{$et:15}})</pre>

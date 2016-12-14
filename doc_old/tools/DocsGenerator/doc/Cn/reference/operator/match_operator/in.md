## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$in:[<值1>,<值2>,...<值N>]}}</pre>

## 描述##

选择集合中“<字段名>”值匹配给定数组（[<值1>,<值2>,...<值N>]）中任意一个值的记录；如果“<字段名>”本身是数组类型，那么只要满足“<字段名>”中任意一个值等于给定数组（[<值1>,<值2>,...<值N>]）中值的记录都会返回。

## 示例##

* 选择集合 bar 下 age 字段的值是20或25的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$in:[20,25]}})</pre>

* $in 匹配嵌套数组对象中的元素。选择集合 bar 中数组对象 name 存在元素“Tom”或“Mike”的记录，并将这些记录的 age 字段删除。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{age:""}},{name:{$in:["Tom","Mike"]}})</pre>

**Note:**

当给定数组只有一个值时，即{<字段名>:{$in:[<值>]}}，等价于{<字段名>:<值>}

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$in:[20]}})等价于db.foo.bar.find({age:20})</pre>

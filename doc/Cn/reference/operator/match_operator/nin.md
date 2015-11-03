## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$nin:[<值1>,<值2>,...<值N>]}}</pre>

## 描述##

选择集合中“<字段名>”值不等于给定数组（[<值1>,<值2>,...<值N>]）中任意一个值的记录或者不存在给定字段名的记录；如果“<字段名>”本身是数组类型，那么选择“<字段名>”中任意一个值都不等于给定数组（[<值1>,<值2>,...<值N>]）中值的记录。

## 示例##

* 选择集合 bar 下 age 字段的值不等于20和25或集合 bar 下不存在 age 字段的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$nin:[20,25]}})</pre>

* $nin 匹配数组对象中的元素。选择集合 bar 中存在数组对象 name 且其元素不包含“Tom”和“Mike”或者选择集合 bar 中不存在数组对象 name 的记录，并将这些记录的 age 字段删除。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{age:""}},{name:{$nin:["Tom","Mike"]}})</pre>

**Note:**

当给定数组只有一个值时，即{<字段名>:{$nin:[<值>]}}，等价于{<字段名>:{$ne:<值>}}

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$nin:[20]}})等价于db.foo.bar.find({age:{$ne:20}})</pre>

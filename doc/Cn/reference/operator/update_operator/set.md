## 语法##

<pre class="prettyprint lang-diy">
{$set:{<字段名1>:<值1>,<字段名2>:<值2>,...}}</pre>

## 描述##

$set 操作是将指定的“<字段名>”更新为指定的“<值>”。如果原记录中没有指定的字段名，那将字段名和值填充到记录中；如果原记录中存在指定的字段名，那么将字段名的值更新为指定的值。

## 示例##

* 选择集合 bar 下不存在 age 字段的记录，使用 $set 更新这些记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{age:5,ID:10}},{age:{$exists:0}})</pre>

* 更新集合 bar 下的所有记录，使所有记录的字段 str 的值更新为“abc”

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{str:"abd"}})</pre>

* 使用 $set 更新嵌套数组对象里面的元素。字段名 arr 在集合 bar 中是一个嵌套数组对象，例如有两条记录：{arr:[1,2,3],name:"Tom"},{name:"Mike",age:20}第二条记录没有 arr 字段名

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{"arr.1":4}},{name:{$exists:1}})</pre>

此操作是选择含有 name 字段的所有记录，然后使用 \$set 更新这些记录的数组对象 arr。如果原记录中没有数组对象 arr，使用 $set 会将 arr 字段以嵌套对象的方式插入到记录中。上面两条记录更新之后为：

<pre class="prettyprint lang-diy">
{arr:[1,4,3],name:"Tom"},{arr:{"1":4},name:"Mike",age:20}</pre>

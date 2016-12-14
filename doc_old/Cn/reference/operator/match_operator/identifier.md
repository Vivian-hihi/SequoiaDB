## 语法##

<pre class="prettyprint lang-diy">
{"字段名.$+标识符":value}</pre>


## 描述##

$+标识符是一种特殊的命令符，这种命令符只作用于数组对象，标识符是一个整数，如 $1，$3，标识符相当于一个临时的存储，会把匹配成功的数组元素的索引存储起来。下面这些是错误的书写格式：$5.4，$a2，$3c，$MA。

这种命令符只作用于数组，用来代替数组的索引 Key，并且可以把匹配到的第一个索引值传递到方法 update 的 rule 参数中。

## 示例##

* 查询

有记录：{a:[1,2,3,4,5]};{a:[1,4,5]};{a:[4,2,1]}现在要查询出数组中存在元素5的记录，使用如下命令

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({"a.$1":5},{a:1})</pre>

只要记录中数组对象 a 存在元素5，就能返回。返回结果如下：

<pre class="prettyprint lang-diy">
{ "a": [ 1, 4, 5 ] }
{ "a": [ 1, 2, 3, 4, 5 ] }</pre>

* 更新

（1） 有记录 { a : [ 1, 2, 3, 4, 5 ] }，现在要修改数组 a 中的元素，把值为4的元素改成100，使用如下命令

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{"a.$1":100}},{"a.$1":4})</pre>

在匹配时元素4的索引 Key 是3，因此在更新规则 { "$set" : { "a.$1": 100 } } 中，$1的值为3，系统会自动把更新规则转换成 { "$set" : { "a.3" : 100 } }

更新后记录为：

<pre class="prettyprint lang-diy">
{ a : [ 1, 2, 3, 100, 5 ] }</pre>

（2） 有记录 { a : [ 1, 2, 3, 4, 5 ], b : [ 6, 7, 8 ] }，现要修改数组 a 中的元素，把值为4的元素改成100，且把数组 b 中值为6的元素修改为200，使用如下命令

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({ "$set" : { "a.$1" : 100, "b.$2" : 200 } },{ "a.$1": 4, "b.$2" : 6 })</pre>

更新后记录为：

<pre class="prettyprint lang-diy">
{ a : [ 1, 2, 3, 100, 5 ], b : [ 200, 7, 8 ] }</pre>

**Note:**

如果有多个元素符合规则，那么只会修改第一个。如下例：

（3） 有记录 {  a : [ 1, 2, 2, 2, 5 ] }，现要修改数组 a 中的元素，把值为2的元素改成100，使用如下命令

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({ "$set" : { "a.$1" : 100 } },{ "a.$1": 2 })</pre>

更新后记录为：

<pre class="prettyprint lang-diy">
{ a : [ 1, 100, 2, 2, 5 ]  }</pre>

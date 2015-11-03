## 语法##

<pre class="prettyprint lang-diy">
{$push:{<字段名1>:<值1>,<字段名2>:<值2>,...}}</pre>

## 描述##

$push 将给定数值（<值1>）插入到目标数组（<字段名1>）中，操作对象必须为数组类型的字段。如果记录中不存在指定的字段名，将指定的字段名以数组对象的形式推入到记录中并填充其指定的数值；如果记录中存在指定的字段名，且字段名存在指定的数值，指定的数值也会被推入到记录中。

## 示例##

* 向集合 bar 下的 arr 数组对象推入数值1。原记录中 arr 数组对象存在元素1，如有记录：

<pre class="prettyprint lang-diy">
{arr[1,2,4],age:10,name:["Tom","Mike"]}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$push:{arr:1}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr[1,2,4,1],age:10,name:["Tom","Mike"]}</pre>

虽然原来 arr 中有元素1，使用 $push 操作符，还是会将元素1推入到 arr 数组对象中。

* 向集合 bar 中推入不存在的数组对象和值。原记录中不存在数组对象 name，如有记录：

<pre class="prettyprint lang-diy">
{arr[1,2],age:20}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$push:{name:"Tom"}},{name:{$exists:0}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr[1,2],age:20,name:["Tom"]}</pre>

原记录中不存在数组对象 name，使用 $push 操作符，会将 name 以数组对象的形式推入到记录中。

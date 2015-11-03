## 语法##

<pre class="prettyprint lang-diy">
{$pull:{<字段名1>:<值1>,<字段名2>:<值2>,...}}</pre>

## 描述##

$pull 清除指定数组对象（<字段名1>,<字段名2>,...）的指定值（<值1>,<值2>,...）。操作对象必须为数组类型的字段。如果记录中不存在指定的数组对象，跳过不做任何操作；如果指定的值不存在数组对象中，也不做任何操作。

## 示例##

* 清除集合 bar 下数组对象 arr 值为2的元素以及数组对象 name 中元素值为“Tom”的元素。如有记录：

<pre class="prettyprint lang-diy">
{arr[1,2,4,5],age:10,name:["Tom","Mike"]}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pull:{arr:2,name:"Tom"}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr[1,4,5],age:10,name:["Mike"]}</pre>

* 清除集合 bar 下数组对象 arr 中元素值等于2的元素以及数组对象 name 中元素值为“Tom”的元素。如有记录：

<pre class="prettyprint lang-diy">
{arr[1,3,4,5],age:10,name:["Tom","Mike"]}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pull:{arr:2,name:"Tom"}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr[1,3,4,5],age:10,name:["Mike"]}</pre>

由于 arr 数组对象没有元素值为2的元素，因此对 arr 对象不做任何操作。

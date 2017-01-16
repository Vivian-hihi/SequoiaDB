## 语法##

<pre class="prettyprint lang-diy">
{$push_all:{<字段名1>:[<值1>,<值2>,...,<值N>],<字段名2>:[<值1>,<值2>,...,<值N>],...}}</pre>

## 描述##

$push_all 向指定数组对象（如<字段名1>）推入每一个指定值（[<值1>,<值2>,...,<值N>]）。操作对象必须为数组类型的字段。如果记录中不存在指定的数组对象，向记录推入指定的数组对象和每一个指定的值（[<值1>,<值2>,...,<值N>]）；如果指定的值存在数组对象中，同样被推入到数组对象中。

## 示例##

* 向集合 bar 下的 arr 数组对象推入[1,2,8,9]数组。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,4,5],age:10,name:["Tom","Mike"]}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$push_all:{arr:[1,2,8,9]}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,2,4,5,1,2,8,9],age:10,name:["Mike"]}</pre>

虽然原来记录 arr 对象有元素1和2，使用 $push_all 操作符，会将[1,2,8,9]全部值推入到数组对象 arr 中。

* 向集合 bar 中推入数组对象 name，假设原记录不存在数组对象 name。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,3,4,5],age:10}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$push_all:{name:["Tom","Jhon"]}},{name:{$exists:0}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,3,4,5],age:10,name:["Tom","Mike"]}</pre>

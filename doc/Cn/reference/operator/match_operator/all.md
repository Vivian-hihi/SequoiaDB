## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$all:[<值1>,<值2>,...<值N>]}}</pre>

## 描述##

$all 的操作对象是数组类型的字段名，选择“<字段名>”包含所有给定数组（[<值1>,<值2>,...<值N>]）中的值。

## 示例##

* 选择集合 bar 下 name 字段的值包含“Tom”和“Mike”的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({name:{$all:["Tom","Mike"]}})</pre>

因此，上面的语句会匹配集合 bar 中有 name 字段，且值形如下面数组的记录：

<pre class="prettyprint lang-diy">
["Tom","Mike",..]
["Tom","Jhon","Mike",...]</pre>

但是不会匹配集合 bar 下 name 字段值形如下面数组的记录

<pre class="prettyprint lang-diy">
["Tom","Jhon"]</pre>

**Note:**

使用 $all 操作一个非数组类型的字段的话，例如：

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$all:[20]}}) 它等价于 db.foo.bar.find({age:20})</pre>

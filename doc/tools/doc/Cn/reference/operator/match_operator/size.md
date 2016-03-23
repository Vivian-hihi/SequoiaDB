## 语法##

<pre class="prettyprint lang-diy">
{"<字段名>":{$size:"<值>"}}</pre>

## 描述##

$size 的操作对象为数组型字段，匹配数组长度为指定“<值>”的记录。

## 示例##

* 返回集合 bar 中数组类型字段 arr 的长度为2的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({arr:{$size:2}})</pre>

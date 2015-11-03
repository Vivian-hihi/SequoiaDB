##语法##

<pre class="prettyprint lang-diy">
{$replace:{<字段名1>:<值1>,<字段名2>:<值2>,...}}</pre>

## 描述##

$replace 操作是将文档全部替换成"{<字段名1>:<值1>,<字段名2>:<值2>,...}"。除了保留原始的 _id 之外，原始文档的内容会全部清空，并替换成"{<字段名1>:<值1>,<字段名2>:<值2>,...}"。

## 示例##

* 选择集合 bar 下不存在 age 字段的记录，使用 $replace 替换这些记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$replace:{age:0,name:'default'}},{age:{$exists:0}})</pre>

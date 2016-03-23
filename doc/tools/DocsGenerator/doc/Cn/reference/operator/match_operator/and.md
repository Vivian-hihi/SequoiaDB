## 语法##

<pre class="prettyprint lang-diy">
{$and:[{<表达式1>}，{<表达式2>},...,{<表达式N>}]}</pre>

## 描述##

$and 是一个逻辑“与”操作。它的作用是选择满足所有表达式（<表达式1>，<表达式2>,...,<表达式N>）的记录，但是如果第一个表达式（<表达式1>）的计算结果为 false，SequoiaDB 将不会再执行后面的表达式。

## 示例##

* 选择集合 bar 下 age 字段值为20，price 字段值小于10的记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({$and:[{age:20},{price:{$lt:10}}]})</pre>

**Note:**

SequoiaDB 提供了一种隐式的 and 操作，用逗号（,）隔开个表达式，例如

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:20,price:{$lt:10}})</pre>

当使用 and 操作对同一个字段名时，如{age：{$lt:20}}and{age:{$exists:1}}。那么可以用 $and 操作两个分开的表达式，也可以合并这两个表达式{age:{$lt:20,$exists:1}}。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$inc:{salary:200}},{$and:[{age:{$lt:20}},{age:{$exists:1}}]})
> db.foo.bar.update({$inc:{salary:200}},{age:{$lt:20,$exists:1}})</pre>

两个操作的结果相同，首先查询集合 bar 下存在 age 字段并且 age 的值小于20的记录，然后对这些记录的 salary 字段的值增加200。

## 语法##

<pre class="prettyprint lang-diy">
{$not:[{<表达式1>},{<表达式2>},...,{<表达式N>}]}</pre>

## 描述##

$not 是一个逻辑“非”操作。它的作用是选择不匹配表达式（<表达式1><表达式2>,...,<表达式N>）的记录。只要不满足其中的任意一个表达式，记录就会返回。

## 示例##

* 选择集合 bar 下 age 字段值不等于20或 price 字段值不小于10的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({$not:[{age:20},{price:{$lt:10}}]})</pre>

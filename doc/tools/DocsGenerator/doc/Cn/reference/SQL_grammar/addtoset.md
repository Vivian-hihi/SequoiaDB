## addtoset() 函数##

将多个记录中的字段合并为一个没有重复值的数组。

## 语法##

<pre class="prettyprint lang-javascript">
> addtoset(field name)</pre>

## 示例##

* 将表中多个记录中的字段合并为一个没有重复值的数组

表中原始记录

<pre class="prettyprint lang-diy">
{a:1, b:1}
{a:2, b:2)
{a:2, b:3}
{a:2, b:3}</pre>

<pre class="prettyprint lang-javascript">
SELECT a, ADDTOSET(b) AS b FROM foo.bar GROUP BY a</pre>

得到记录

<pre class="prettyprint lang-diy">
{a:1, b:[1]}
{a:2, b:[2,3]}</pre>
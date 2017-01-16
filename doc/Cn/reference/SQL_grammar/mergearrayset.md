## mergearrayset() 函数##

将多个数组字段合并为一个不包含重复字段的数组。

## 语法##

<pre class="prettyprint lang-javascript">
mergearrayset(field name)</pre>

## 示例##

* 将表中多个数组字段合并为一个不包含重复字段的数组

表中原始记录

<pre class="prettyprint lang-diy">
{a:1, b:[1,2,3]}
{a:1, b:[2,2,3]}</pre>

<pre class="prettyprint lang-javascript">
SELECT a, MERGEARRAYSET(b) AS b FROM foo.bar GROUP BY a</pre>

得到记录

<pre class="prettyprint lang-diy">
{a:1, b:[1,2,3]}</pre>

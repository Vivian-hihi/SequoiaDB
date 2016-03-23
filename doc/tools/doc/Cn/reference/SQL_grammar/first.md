## first() 函数##

选择范围内第一条数据。

## 语法##

<pre class="prettyprint lang-javascript">
first(field name)</pre>

## 示例##

* 选择表中第一条数据

表中原始记录

<pre class="prettyprint lang-diy">
{a:1, b:2}
{a:2, b:3}
{a:3, b:3)</pre>

<pre class="prettyprint lang-javascript">
SELECT FIRST(a) AS a, b FROM foo.bar GROUP BY b</pre>

得到记录

<pre class="prettyprint lang-diy">
{a:1, b:2}
{a:2, b:3)</pre>

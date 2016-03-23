## split by 语句##

按照某个数组字段将记录拆分。

## 语法##

<pre class="prettyprint lang-javascript">
split by &lt;field name&gt;</pre>

## 示例##

* 拆分表中原始记录｛a:1,b:2,c:[3,4,5]｝

<pre class="prettyprint lang-javascript">
SELECT * FROM foo.bar SPLIT BY c</pre>

得到结果为：

<pre class="prettyprint lang-diy">
{a:1, b:2, c:3}
{a:1, b:2, c:4}
{a:1, b:2, c:5}</pre>

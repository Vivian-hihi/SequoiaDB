## buildobj() 函数##

将记录中多个字段合并为一个对象。

## 语法##

<pre class="prettyprint lang-javascript">
buildobj(field name1,fieldname2,...)</pre>

## 示例##

* 将表中记录中多个字段合并为一个对象

表中原始记录

<pre class="prettyprint lang-diy">
{a:1,b:1,c:1}
{a:2,b:2,c:2}
{a:3,b:3,c:3}</pre>

<pre class="prettyprint lang-javascript">
SELECT a, buildobj(d, c) AS d FROM foo.bar</pre>

得到记录

<pre class="prettyprint lang-diy">
{a:1, d:{b:1, c:1}}
{a:2, d:{b:2, c:2}}
{a:3, d:{b:3, c:3}}</pre>

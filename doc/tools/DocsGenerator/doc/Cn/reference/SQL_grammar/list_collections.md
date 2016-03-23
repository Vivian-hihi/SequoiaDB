## list collections 语句##

枚举集合空间中的集合。

## 语法##

<pre class="prettyprint lang-javascript">
list collections</pre>

## 示例##

* 本例会返回集合空间中的所有集合

<pre class="prettyprint lang-javascript">
> db.exec("list collections") </pre>

结果：

<pre class="prettyprint lang-javascript">
{
	"Name": "testfoo.testbar"
}
{
	"Name":"big.small"
}
Return 2 row(s).
</pre>
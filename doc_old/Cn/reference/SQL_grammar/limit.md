## limit 语句##

用于限制返回记录个数。

## 语法##

<pre class="prettyprint lang-javascript">
limit&lt;limit_num&gt;</pre>

-   &lt;limit_num&gt;：限制数

## 示例##

* 希望返回集合中前10条记录：

<pre class="prettyprint lang-javascript">
> db.exec("select * from foo.bar limit 10") </pre>
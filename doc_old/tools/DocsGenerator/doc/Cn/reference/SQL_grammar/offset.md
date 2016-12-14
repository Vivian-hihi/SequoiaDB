## offset语句##

用于设置跳过的记录个数。

## 语法##

<pre class="prettyprint lang-javascript">
offset&lt;offset_num&gt;</pre>

-   &lt;offset_num&gt;：跳过记录数

## 示例##

* 希望跳过前5条记录，从第5条后面开始返回：

<pre class="prettyprint lang-javascript">
> db.exec("select * from foo.bar offset 5") </pre>
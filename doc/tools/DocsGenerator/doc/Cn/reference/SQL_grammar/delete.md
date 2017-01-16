## delete 语句##

用于删除集合中的记录。

## 语法##

<pre class="prettyprint lang-javascript">
delete from &lt;cs_name&gt;.&lt;cl_name&gt; [where &lt;condition&gt;]</pre>

-   &lt;cs_name&gt;：集合空间名

-   &lt;cl_name&gt;：集合名

-   &lt;condition&gt;：条件，只对符合条件的记录删除

## 示例##

* 本例会删除集合中的所有记录：

<pre class="prettyprint lang-javascript">
> db.execUpdate("delete from foo.bar") </pre>

* 本例会删除符合条件 age < 10 的记录：

<pre class="prettyprint lang-javascript">
> db.execUpdate("delete from foo.bar where age &lt;10")</pre>
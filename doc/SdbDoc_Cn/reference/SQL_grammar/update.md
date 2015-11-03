## update 语句##

用于修改集合中的记录。

## 语法##

<pre class="prettyprint lang-javascript">
update &lt;cs_name&gt;.&lt;cl_name&gt; set (&lt;field1_name&gt;=&lt;value1&gt;,...) [where &lt;condition&gt;]</pre>

-   &lt;cs_name&gt;：集合空间名

-   &lt;cl_name&gt;：集合名

-   &lt;condition&gt;：条件，只对符合条件的记录更新

## 示例##

* 本例会修改集合中全部的记录，将记录中的 age 字段值修改为20，如果记录中不存在 age 字段，则将 age：20添加到记录中：

<pre class="prettyprint lang-javascript">
> db.execUpdate("update foo.bar set age=20") </pre>

* 本例会修改符合条件的记录，只对符合条件 age &lt; 10 的记录做修改操作：

<pre class="prettyprint lang-javascript">
> db.execUpdate("update foo.bar set age=20 where age &lt; 10")</pre>

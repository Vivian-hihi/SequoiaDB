## drop index 语句##

用于删除集合中的索引。

## 语法##

<pre class="prettyprint lang-javascript">
drop index &lt;index_name&gt; on &lt;cs_name&gt;.&lt;cl_name&gt;</pre>

-   &lt;index_name&gt;：索引名

-   &lt;cs_name&gt;：集合空间名

-   &lt;cl_name&gt;：集合名

## 示例##

* 删除集合空间 foo 中集合 bar 下名为 test_index 的索引名

<pre class="prettyprint lang-javascript">
> db.execUpdate("drop index test_index on foo.bar") //等价于 db.foo.bar.dropIndex("test_index")</pre>

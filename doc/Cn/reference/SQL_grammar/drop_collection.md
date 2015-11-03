## drop collection 语句##

用于删除集合空间中的集合。

## 语法##

<pre class="prettyprint lang-javascript">
drop collection &lt;cs_name&gt;.&lt;cl_name&gt;</pre>

-    &lt;cs_name&gt;：数据库中的集合空间名，集合空间名必须在数据库中存在；
-    &lt;cl_name&gt;：集合名，集合名也必须在指定的集合空间中存在。

## 示例##

* 删除集合空间 foo 中的集合 bar

<pre class="prettyprint lang-javascript">
> db.execUpdate("drop collection foo.bar") //等价于 db.foo.dropCL("bar")</pre>

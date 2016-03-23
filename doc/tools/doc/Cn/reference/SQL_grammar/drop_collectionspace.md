## drop collectionspace 语句##

用于删除数据库中的集合空间。

## 语法##

<pre class="prettyprint lang-javascript">
drop collectionspace &lt;cs_name&gt;</pre>

-    &lt;cs_name&gt;：集合空间名，集合空间名必须在数据库中存在。

## 示例##

* 删除名为 foo 的集合空间

<pre class="prettyprint lang-javascript">
> db.execUpdate("drop collectionspace foo") //等价于 db.dropCS("foo")</pre>

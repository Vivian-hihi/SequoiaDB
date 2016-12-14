## create collectionspace 语句##

用于创建数据库中的集合空间。

## 语法##

<pre class="prettyprint lang-javascript">
create collectionspace &lt;cs_name&gt;</pre>

-   &lt;cs_name&gt;：集合空间名称，集合空间名的最大长度为127Byte，并且不能为空。

## 示例##

* 创建名为 foo 的集合空间

<pre class="prettyprint lang-javascript">
> db.execUpdate("create collectionspace foo") //等价于 db.createCS("foo")</pre>
## create collection 语句##

用于创建集合，必须指定集合所在的集合空间。

## 语法##

<pre class="prettyprint lang-javascript">
create collection &lt;cs_name&gt;.&lt;cl_name&gt;</pre>

-   &lt;cs_name&gt;：数据库中的集合空间名称。
-   &lt;cl_name&gt;：集合名，集合名长度不能超过127Byte，并且不能为空，在同一个集合空间中不能存在相同的集合名。

## 示例##

* 在集合空间foo下创建集合bar。

<pre class="prettyprint lang-javascript">
> db.execUpdate("create collection foo.bar") //等价于 db.foo.createCL("bar")</pre>
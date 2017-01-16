## create index 语句##

用于在集合中创建索引。在不读取整个集合的情况下，索引使数据库应用程序可以更快地查找数据。

## 语法##
<pre class="prettyprint lang-javascript">
create [unique] index &lt;index_name&gt; on &lt;cs_name&gt;.&lt;cl_name&gt; (field1_name [asc|desc],...)</pre>

-   [unique]：标识创建的索引是否唯一。在唯一索引所指定的索引键字段上，集合中不可存在一条以上的记录完全重复。

-   &lt;index_name&gt;：索引名称

-   &lt;cs_name&gt;：集合空间名称

-   &lt;cl_name&gt;：集合名称

-   field1_name：创建索引所在的字段名，同一个索引名可以在多个字段名上创建

-   [asc|desc]：排序，asc 表示升序索引某个字段中的值，desc 表示降序索引某个字段中的值，默认为升序。

## 示例##

* 本例会创建一个简单的索引，名为“test_index”，在 foo 集合空间的 bar 集合上的 age 字段：

<pre class="prettyprint lang-javascript">
> db.execUpdate("create index test_index on foo.bar (age)")</pre>

* 如果希望以降序索引某个字段中的值，可以在字段名后面添加保留字 desc：

<pre class="prettyprint lang-javascript">
> db.execUpdate("create index test_index on foo.bar (age desc)")</pre>

* 如果希望索引不止在一个字段上，可以在括号中列出这些字段的名称，用逗号隔开：

<pre class="prettyprint lang-javascript">
> db.execUpdate("create index test_index on foo.bar (age desc,name asc)")</pre>

* 下面的实例会创建一个唯一索引：

<pre class="prettyprint lang-javascript">
> db.execUpdate("create unique index test_index on foo.bar (age)")</pre>

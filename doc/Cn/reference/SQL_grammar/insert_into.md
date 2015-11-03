## insert into 语句##

用于向集合中插入新的数据。

## 语法##

<pre class="prettyprint lang-javascript">
insert into &lt;cs_name&gt;.&lt;cl_name&gt;(&lt;field1_name,field2_name,...&gt;) values(&lt;value1,value2,...&gt;)</pre>

或者

<pre class="prettyprint lang-javascript">
insert into &lt;cs_name&gt;.&lt;cl_name&gt; &lt;select_set&gt;</pre>

-   &lt;cs_name&gt;：集合空间名

-   &lt;cl_name&gt;：集合名

-   &lt;field_name&gt;：字段名

-   &lt;value&gt;：字段名所对应的值

-   &lt;select_set&gt;：查询结果集

## 示例##

* 本例会向集合 bar 中插入一条新的数据，字段名为 age 和 name，对应的值分别为（25，“Tom”）：

<pre class="prettyprint lang-javascript">
> db.execUpdate("insert into foo.bar(age,name) values(25,"Tom")")</pre>

* 本例会向集合 bar 中插入批量的数据，这些数据为集合 small 中的查询结果集：

<pre class="prettyprint lang-javascript">
> db.execUpdate("insert into foo.bar select * from big.small")</pre>

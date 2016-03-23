## select 语句##

用于从集合中选取数据，结果被存储在一个结果集中。

## 语法##

<pre class="prettyprint lang-javascript">
select * from &lt;cs_name&gt;.&lt;cl_name&gt;</pre>

或者

<pre class="prettyprint lang-javascript">
select &lt;field1_name,field2_name,...&gt; from &lt;cs_name&gt;.&lt;cl_name&gt;</pre>

-   &lt;cs_name&gt;：集合空间名

-   &lt;cl_name&gt;：集合名

-   &lt;field_name&gt;：字段名

## 示例##

* 本例会选择指定的字段名返回，如果某条符合条件的记录没有指定的字段名，那么返回它的值为 null：

<pre class="prettyprint lang-javascript">
> db.exec("select age,name from foo.bar") </pre>

结果：

<pre class="prettyprint lang-diy">
{
  "age": 10,
  "name": null
}
{
  "age": 10,
  "name": "Tom"
}
...</pre>

* 本例返回集合中的所有记录的所有字段名

<pre class="prettyprint lang-javascript">
> db.exec("select * from foo.bar")</pre>

结果：

<pre class="prettyprint lang-diy">
{
  "_id": 
  {
    "$oid": "51c909b0c5b855e029000000"
  },
  "age": 10
}
{
  "_id": 
  {
    "$oid": "51c909b9c5b855e029000001"
  },
  "age": 10,
  "name": "Tom"
}
{
  "_id": 
  {
    "$oid": "51c909c2c5b855e029000002"
  },
  "age": 10,
  "name": "Tom",
  "phone": 123456
}
...</pre>

**Note:**

（1） 可以选择类似 where，group by，order by，limit，offset 的关键字对要选择的记录做控制。

（2） 如果查询源不为集合，则本层查询中所有字段均需要引用别名（\* 除外），例如：select T.a , T.b from (select \* from foo.bar) as T where T.a &lt; 10

（3） 子查询必须包含别名，子查询中出现的别名只作用于上一层。

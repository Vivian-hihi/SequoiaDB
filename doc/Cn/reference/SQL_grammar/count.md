## count() 函数##

用于计数，返回匹配指定字段名的条数。

## 语法##

<pre class="prettyprint lang-javascript">
count(field_name) as &lt;alisa_name&gt;</pre>

**Note:**

使用 count 函数对字段名计数，必须使用别名。

## 示例##

* 对集合 bar 中 age 字段进行计数：

<pre class="prettyprint lang-javascript">
db.exec("select count(age) as 数量 from foo.bar")</pre>

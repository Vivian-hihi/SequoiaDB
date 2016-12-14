## max() 函数##

用于返回指定字段名的最大值。

## 语法##

<pre class="prettyprint lang-javascript">
max(field_name) as &lt;alisa_name&gt;</pre>

**Note:**

使用 max 函数返回字段名的最大值时，必须使用别名。

## 示例##

* 对集合 bar 中 age 字段返回最大值：

<pre class="prettyprint lang-javascript">
> db.exec("select max(age) as 最大年龄 from foo.bar")</pre>
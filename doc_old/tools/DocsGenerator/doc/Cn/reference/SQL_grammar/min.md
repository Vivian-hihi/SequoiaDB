## min() 函数##

用于返回指定字段名的最小值。

## 语法##

<pre class="prettyprint lang-javascript">
min(field_name) as &lt;alisa_name&gt;</pre>

**Note:**

使用 min 函数返回字段名的最小值时，必须使用别名。

## 示例##

* 对集合 bar 中 age 字段返回最小值：

<pre class="prettyprint lang-javascript">
> db.exec("select min(age) as 最小年龄 from foo.bar")</pre>
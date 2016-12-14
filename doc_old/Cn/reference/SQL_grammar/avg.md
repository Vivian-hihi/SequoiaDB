## avg() 函数##

用于求指定字段名的平均值。

## 语法##

<pre class="prettyprint lang-javascript">
avg(field_name) as <alisa_name></pre>

**Note:**

（1） 使用 avg 函数对字段名求平均值，必须使用别名。

（2） 对非数值型字段自动跳过。

## 示例##

* 对集合 bar 中 age 字段进行求平均值：

<pre class="prettyprint lang-javascript">
> db.exec("select avg(age) as 平均年龄 from foo.bar")</pre>
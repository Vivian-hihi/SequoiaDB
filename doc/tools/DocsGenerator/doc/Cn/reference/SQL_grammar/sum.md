## sum() 函数##

用于求和。

## 语法##

<pre class="prettyprint lang-javascript">
sum(field_name) as &lt;alisa_name&gt;</pre>

**Note:**

（1） 使用 sum 函数对字段名求和，必须使用别名。

（2） 对非数值型字段自动跳过。


## 示例##

* 对集合 bar 中 age 字段进行求和：

<pre class="prettyprint lang-javascript">
> db.exec("select sum(age) as 年龄总和 from foo.bar")</pre>

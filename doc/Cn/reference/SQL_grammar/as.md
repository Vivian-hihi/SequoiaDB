## as 语句##

用于为集合名或者字段名指定别名（alias）。

## 语法##
<pre class="prettyprint lang-javascript">
&lt;cs_name.cl_name | (select_set) | field_name&gt; AS &lt;alias_name&gt;</pre>
<br>
**&lt;cs_name&gt;：**集合空间名
**&lt;cl_name&gt;：**集合名
**select_set：**结果集
**field_name：**字段名
**&lt;alias_name&gt;：**别名</pre>


## 示例##

* 集合别名

<pre class="prettyprint lang-javascript">
> db.exec("select T1.age,T1.name from foo.bar as T1 where T1.age>10") </pre>

* 字段别名

<pre class="prettyprint lang-javascript">
> db.exec("select age as 年龄 from foo.bar where age>10")</pre>

* 结果集别名

<pre class="prettyprint lang-javascript">
> db.exec("select T.age,T.name from (select age,name from foo.bar) as T where T.age>10")</pre>

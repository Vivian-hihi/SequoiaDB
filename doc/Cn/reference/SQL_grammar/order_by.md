## order by 语句##

用于根据指定的字段名对结果集进行排序，默认为升序排序。

## 语法##

<pre class="prettyprint lang-javascript">
order by &lt;field1_name [ASC|DESC ], ...&gt; </pre>

-   &lt;field_name&gt;：字段名

-   [ asc | desc ]：排序，asc 表示升序，desc 表示降序，默认为 asc

## 示例##

* 希望计算每个部门的员工数，并按字段名 dept_no 分组，并按字段名的降序排序：

<pre class="prettyprint lang-javascript">
> db.exec("select dept_no，count(emp_no) as 员工总数 from foo.bar group by dept_no order by dept_no desc")</pre> 

**Note:**

像 sum，count，min，max，avg 这样的计数函数必须使用别名。

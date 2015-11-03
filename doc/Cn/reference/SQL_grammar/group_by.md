## group by 语句##

用于删除集合中的记录。

## 语法##

用于结合合计函数，根据一个或多个字段名对结果集进行分组。

-   &lt;field_name&gt;：字段名

-   [ asc | desc ]：排序，asc 表示升序，desc 表示降序，默认为 asc

## 示例

* 希望计算每个部门的员工数，并按字段名 dept_no 分组：

<pre class="prettyprint lang-javascript">
> db.exec("select dept_no，count(emp_no) as 员工总数 from foo.bar group by dept_no") </pre>

**Note:**

像 sum，count，min，max，avg 这样的计数函数必须使用别名。

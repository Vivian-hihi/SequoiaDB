## right outer join 语句##

right outer join 会从右边的集合名（collection2_name）中返回所有的记录，即使在左边的集合名（collection1_name）中没有匹配的记录。

## 语法##

<pre class="prettyprint lang-javascript">
&lt;collection1_name | (select_set1)&gt; as &lt;alias1_name&gt; right outer join &lt;collection2_name | (select_set2)&gt; as &lt;alias2_name&gt; [ON condition]</pre>

## 示例##

* 有员工信息表 foo.emp 和部门信息表 foo.dept，查询员工号 emp_no &lt;10 所在的部门名 dept_name：

<pre class="prettyprint lang-javascript">
> db.exec("select E.emp_no,D.dept_name from foo.emp as E right outer join foo.dept as D on E.dept_no=D.dept_no where E.emp_no &lt; 10")</pre>
## inner join 语句##

用于根据两个或多个集合中的字段名之间的关系，从这些集合中查询数据。

## 语法##

<pre class="prettyprint lang-javascript">
&lt;collection1_name | (select_set1)&gt; as &lt;alias1_name&gt; inner join &lt;collection2_name | (select_set2)&gt; as &lt;alias2_name&gt; [ON condition]</pre>

## 示例##

* 有员工信息表 foo.emp 和部门信息表 foo.dept，查询员工号 emp_no 所在的部门名 dept_name：

<pre class="prettyprint lang-javascript">
> db.exec("select E.emp_no,D.dept_name from foo.emp as E inner join foo.dept as D on E.dept_no=D.dept_no")</pre>

**Note:**

（1）不能包含非联合条件，如下写法是错误的：

<pre class="prettyprint lang-javascript">
select T1.a,T2.b from foo.bar1 as T1 inner join foo.bar2 as T2 on T1.a &lt;10 </pre>

(2）不能在 join 本层使用 select * 语句。

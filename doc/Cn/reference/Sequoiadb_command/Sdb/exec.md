##语法##
***db.exec(&lt;select sql&gt;)***

执行 SQL 的 select 语句

##示例##

* 从集合 my.my 中查所 age = 20 的记录

<pre class="prettyprint lang-javascript">
> db.exec("select * from my.my where age = 20")</pre>
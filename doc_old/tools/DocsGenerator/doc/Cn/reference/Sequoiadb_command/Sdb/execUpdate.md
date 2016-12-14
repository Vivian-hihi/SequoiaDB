##语法##
***db.execUpdate()***

###db.execUpdate(&lt;other sql&gt;)###

执行 SQL 的其它语句

##示例##

* 向集合 my.my 中插入新的记录

<pre class="prettyprint lang-javascript">
> db.execUpdate("insert into my.my(name,age) values('zhangshang', 30) ")</pre>
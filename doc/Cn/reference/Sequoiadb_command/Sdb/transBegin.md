##语法##
***db.transBegin()***

开启[事务](SdbDoc_Cn/basic_operation/transaction.html) 。SequoiaDB 数据库事务是指作为单个逻辑工作单元执行的一系列操作。事务处理可以确保除非事务性单元内的所有操作都成功完成，否则不会永久更新面向数据的资源。

## 示例##

* 开启事务：

<pre class="prettyprint lang-javascript">
> db.transBegin()
Takes 0.4125s. </pre>

* 插入记录:

<pre class="prettyprint lang-javascript">
> cl.insert({date:99,id:8,a:0})
Takes 0.3434s. </pre>

* 回滚事务，插入的记录将被回滚，集合中无记录：

<pre class="prettyprint lang-javascript">
> db.transRollback()
Takes 0.6474s.
> cl.count()
Return 0 row(s). </pre>

* 开启事务：

<pre class="prettyprint lang-javascript">
> db.transBegin()
Takes 0.4084s. </pre>

* 插入记录:

<pre class="prettyprint lang-javascript">
> cl.insert({date:99,id:8,a:0})
Takes 0.2644s. </pre>

* 提交事务，插入的记录将被写入数据库：

<pre class="prettyprint lang-javascript">
> db.transCommit()
Takes 0.780s.
> cl.count()
Return 1 row(s). </pre>
##语法##
***db.transBegin()***

开启事务。SequoiaDB 数据库事务是指作为单个逻辑工作单元执行的一系列操作。事务处理可以确保除非事务性单元内的所有操作都成功完成，否则不会永久更新面向数据的资源。

## 示例##

* 开启事务命令：

<pre class="prettyprint lang-javascript">
> db.transBegin() </pre>

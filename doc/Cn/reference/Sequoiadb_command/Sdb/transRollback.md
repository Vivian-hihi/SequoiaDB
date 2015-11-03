##语法##
***db.transRollback()***

事务回滚。在开启事务之后，如果单个逻辑工作单元执行的操作出现异常，执行事务回滚命令，那么数据库回到原来状态。

## 示例##

* 事务回滚命令：

<pre class="prettyprint lang-javascript">
> db.transRollback() </pre>

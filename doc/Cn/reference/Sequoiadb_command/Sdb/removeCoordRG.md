##语法##
***db.removeCoordRG()***

删除数据库中协调分区组。

##示例##

-   删除协调分区组

<pre class="prettyprint lang-javascript">
> db.removeCoordRG()</pre>

**Note:**

删除协调分区组，原则上会把该分区组的所有协调节点都删除，但如果在删除这些节点过程中，先把db对象所连接上的协调节点删除，则有可能会遗留部分协调节点未删除，需要使用Oma类的removeCoord方法删除遗留的协调节点。

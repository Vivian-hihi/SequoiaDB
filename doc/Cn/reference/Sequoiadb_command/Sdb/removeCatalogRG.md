##语法##
***db.removeCatalogRG()***

删除编目分区组。

##示例##

-   删除编目分区组

<pre class="prettyprint lang-javascript">
> db.removeCatalogRG()</pre>

**Note:**

删除编目分区组，要求编目分区组上已经没有数据节点及协调节点的信息。删除编目分区组将会把该组中所有的编目节点都删除。

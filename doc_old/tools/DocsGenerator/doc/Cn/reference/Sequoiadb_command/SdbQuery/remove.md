##语法##
***query.remove()***

删除查询结果集。

##示例##

-   查询集合 bar 下 age 字段值大于10的记录，并将符合条件的记录删除。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).remove()</pre>

**Note:**

-    不能与 query.count()、query.update()同时使用。

-    与 query.sort()同时使用时，在单个节点上排序必须使用索引。

-    在集群中与 query.limit()或query.skip()同时使用时，要保证查询条件会在单个节点或单个子表上执行。

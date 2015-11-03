##语法##
***query.update(&lt;update&gt;,[returnNew])***

更新查询结果集。

##参数描述##

参数名      参数类型    描述                                   是否必填
----------- ----------- -------------------------------------- ----------
update      Json 对象    更新规则。记录按 update 的内容更新。   是
returnNew   bool        是否返回更新后的记录。                 否

##格式##

query.update()方法的定义格式包含 update 参数和 returnNew 参数。其中 update 参数是 Json 对象，与 [Collection.update()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/update.html)的 rule 参数相同。returnNew 参数可选，为 bool 类型，默认为 false。当为 true 时，返回修改后的记录值。

##示例##

查询集合 bar 下 age 字段值大于10的记录，并将符合条件的记录的 age 字段加1。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).update({$inc:{age:1}})</pre>

**Note:**

-    不能与 query.count()、query.remove()同时使用。

-    与 query.sort()同时使用时，在单个节点上排序必须使用索引。

-    在集群中与 query.limit()或 query.skip()同时使用时，要保证查询条件会在单个节点或单个子表上执行。

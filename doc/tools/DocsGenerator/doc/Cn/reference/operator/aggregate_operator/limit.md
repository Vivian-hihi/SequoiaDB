## 描述##

$limit 实现在结果集中限制返回的记录条数。如果指定的记录条数大于实际的记录总数，那么返回实际的记录总数。

## 示例##

* 限制返回结果集中的前10条记录

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate( { $limit : 10 } ) </pre>

该操作表示集合 collection 中读取前10条记录。

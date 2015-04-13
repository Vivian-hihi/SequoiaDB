##query.remove()

###query.remove()
删除查询结果集

###示例
- 查询集合bar下age字段值大于10的记录，并将符合条件的记录删除。
`db.foo.bar.find({age:{$gt:10}}).remove()`

#####Note:
- 不能与query.count()、query.update()同时使用。
- 与query.sort()同时使用时，在单个节点上排序必须使用索引。
- 在集群中与query.limit()或query.skip()同时使用时，要保证查询条件会在单个节点或单个子表上执行。

#####Related information
[db.collectionspace.collection.find()]()<br>
[query.update()]()<br>

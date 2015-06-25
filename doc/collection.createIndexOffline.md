##db.collectionspace.collection.createIndexOffline()

###db.collectionspace.collection.createIndexOffline(&lt;name>,&lt;indexDef>,[isUnique],[enforced])
以离线方式创建索引。

###格式
参数与createIndex()完全相同，请参考[db.collectionspace.collection.createIndex()](URL地址)。

#####Note:
- 在集合数据量较大时（大于1000万条记录）使用离线方式创建索引的性能比普通方式要好。
- 离线方式创建索引时使用了缓存，其大小由配置参数sortbuf决定。
- 在一个集合上同一时刻只能有一个以离线方式创建索引的操作。
- 在以离线方式创建索引期间该集合不能做写操作（插入、更新、删除）。
- 不建议离线方式创建索引与其它操作同时执行，尽量在系统空闲时间执行离线创建索引操作。

#####Related information
[db.collectionspace.collection.createIndex()]()

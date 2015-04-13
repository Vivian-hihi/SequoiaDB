##query.update()

###query.update(&lt;update>, [returnNew])
更新查询结果集

###参数描述
|参数名|参数类型|描述|是否必填|
|-|-|-|-|
|update|Json对象|更新规则。记录按update的内容更新|是|
|returnNew|bool|是否返回更新后的记录|否|

###格式
query.update()方法的定义格式包含update参数和returnNew参数。<br>
其中update参数是Json对象，与SdbCollection.update()的rule参数相同。
参见[db.collectionspace.collection.update()](URL地址)。<br>
returnNew参数可选，为bool类型，默认为false。当为true时，返回修改后的记录值。

###示例
- 查询集合bar下age字段值大于10的记录，并将符合条件的记录的age字段加1。
`db.foo.bar.find({age:{$gt:10}}).update({$inc:{age:1}})`

#####Note:
- 不能与query.count()、query.remove()同时使用。
- 与query.sort()同时使用时，在单个节点上排序必须使用索引。
- 在集群中与query.limit()或query.skip()同时使用时，要保证查询条件会在单个节点或单个子表上执行。

#####Related information
[db.collectionspace.collection.find()]()<br>
[db.collectionspace.collection.update()]()<br>
[query.remove()]()<br>

##query.update()

###query.update(&lt;update>, [returnNew])
update the queried document set

###Parameter Description
|Parameter|Type|Description|Required|
|-|-|-|-|
|update|json|update rule, document will be update by it|Yes|
|returnNew|bool|set true if want to return updated document|No|

###Format
The parameter 'update' is same with parameter 'rule' of SdbCollection.update().
see [db.collectionspace.collection.update()](URL).<br>
The parameter 'returnnew' is optional, default is false. when set it as true, the query will return updated document rather than original.

###Example
- query from collection 'bar' where age is large than 10, and increase field 'age' by 1 for documents satified the condition.
`db.foo.bar.find({age:{$gt:10}}).update({$inc:{age:1}})`

#####Note:
- can not be called simultaneously with query.count(), query.remove().
- sorting must use index when called with query.sort() in a single node.
- query must be ensured to run in a single node or a singe sub-collection when query.update() is called simultaneously with query.limit() or query.skip() in cluster.

#####Related information
[db.collectionspace.collection.find()](URL)  
[db.collectionspace.collection.update()](URL)  
[query.remove()](URL)  

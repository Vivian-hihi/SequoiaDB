##query.remove()

###query.remove()
delete the queried document set

###Example
- query from collection 'bar' where age is large than 10, and remove the documents satified the condition.
`db.foo.bar.find({age:{$gt:10}}).remove()`

#####Note:
- can not be called simultaneously with query.count(), query.remove().
- sorting must use index when called with query.sort() in a single node.
- query must be ensured to run in a single node or a singe sub-collection when query.update() is called simultaneously with query.limit() or query.skip() in cluster.

#####Related information
[db.collectionspace.collection.find()](URL)  
[query.update()](URL)  

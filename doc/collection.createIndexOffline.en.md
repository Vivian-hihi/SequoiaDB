##db.collectionspace.collection.createIndexOffline()

###db.collectionspace.collection.createIndexOffline(&lt;name>,&lt;indexDef>,[isUnique],[enforced])
Create index in offline mode.

###Format
The parameters are same as [db.collectionspace.collection.createIndex()](URL)。

#####Note:
- the offline mode uses cache, the size is determined by the sortbuf configuration.
- can't create indexes in offline mode on the same collection simultaneously.
- the collecton can't be written ( e.g. insert, update and remove ) when an index is creating on it in offline mode.
- it's better to create index in offline mode when the cluster is idle.

#####Related information
[db.collectionspace.collection.createIndex()]()

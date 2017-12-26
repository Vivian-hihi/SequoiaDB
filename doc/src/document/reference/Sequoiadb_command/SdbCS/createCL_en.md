##NAME##

createCL - Create a new collection.

##SYNOPSIS##
**db.collectionspace.createCL(\<name\>,[options])**

##CATEGORY##
Collection Space

##DESCRIPTION##
Create a collection in a specified collection space.
Collection is a logical object which stores records. Each record should 
belong to one and only one collection.

##PARAMETERS##

* `name` ( *String* , *Required* )

    The name of the collection, should be unique to each other in 
    a collection space.

* `options` ( *Object*, *Optional* )

    The options for creating the collection, could be a combination of 
    the following options:

    1. `ShardingKey` ( *Object* ): The sharding key of the collection.
    
        Format: `ShardingKey:{<field1> : <1|-1>,[<field1> : <1|-1>, ...]}`

    2. `ShardingType` ( *String* ): The sharding type of the collection, 
                                    could be one of the following:
        * "hash": Sharding by hash.
        * "range": Sharding by range.
      
        Format: `ShardingType:"hash"|"range"`

    3. `Partition` ( *Int32* ): Specify the number of hash slices, only valid when 
                                 `ShardingType` is "hash". The number of hash slices 
                                 should be power of 2 and in the range of [2\^3，2\^20],
  			                     default to be 1024.
                                 
        Format: `Partition: <num>`

    4. `ReplSize` ( *Int32* ): Number of WriteConcern, default to be 1,
                                can be one of the follow:

        * -1: WriteConcern equals to the number of the active nodes in the replica group.
        * 0: WriteConcern equals to the number of all the nodes in replica group.
        * 1 - 7: WriteConcern = 1-7.
  
        Format: `ReplSize: <num>`
  
    5. `Compressed` ( *Bool* ): Compress the collection or not, default to be false.
  
        Format: `Compressed:true|false`
      
    6. `CompressionType` ( *String* ): The algorithm for compressing, 
                                       default to be "snappy", can be one of the follow:

        * "snappy": use snappy algorithm to compress.
        * "lzw": use lzw algorithm to compress.
      
        Format: `CompressionType:"snappy"|"lzw"`
      
    7. `IsMainCL` ( *Bool* ): Is main collection or not, default to be false.
  
        Format: `IsMainCL:true|false`
      
    8. `AutoSplit` ( *Bool* ): Automatic split or not, default to be false.
  
        Format: `AutoSplit:true|false`
      
    9. `Group` ( *String* ): Specify a replica group which the 
                             collection should be in.
  
        Format: `Group:<group name>`
  
    10. `AutoIndexId` ( *Bool* ): Creating unique index by using field "_id" 
                                  or not, default to be true.
   
        Format: `AutoIndexId:true|false`
  
    11. `EnsureShardingIndex` ( *Bool* ): Creating index by using the fields
                                          of the `ShardingKey` or not, default 
                                          to be true.
                                             
        Format: `EnsureShardingIndex:true|false`

    12. `StrictDataMode` ( *Bool* ): using strict date mode in numeric operations 
or not, default to be false.

        Format: `StrictDataMode:true|false`

    **Note:**

    * The parameter `name` can not be an empty string, and can not 
      include "." or "$"; The length of it should not be greater 
      than 127B, or exception will occur.

    * The parameter `options` haves one or more fields, please use
      comma(,) to separate.
   
    *  AutoSplit must cooperate with hash partitioning and domain.

    *  AutoSplit and Group cannot be set at the same time.

    *  If AutoSplit is not specified in a collection, the value
       of AutoSplit will come from domain.

    *  If AutoSplit is specified in a collection which is belong to
       a domain, the value of AutoSplit will cover the AutoSplit value
       of domain.

    *  If AutoSplit is specified in a collection which is not belong
       to any domain, the collection will be splitted into all groups
       of SYSDOMAIN.

    *  Group must exist in a domain, which contains a collection
       space, which has the group(All replica group belong to
       SYSDOMAIN, that is to say, a collection space can be
       distributed into any replica group if no specific group is
       given)

##RETURN VALUE##

On success, createCL() creates a new collection and return the object.

On error, exception will be thrown.

##ERRORS##

the exceptions of `createCL()` are as below:

| Error Code | Error Type | Description | Solution |
| ------ | ------ | --- | ------ |
| -2 | SDB_OOM | Out of Memory. | Check the settings and usage of physical memory and virtual memory. |
| -11 | SDB_NOSPC | Out of space. | Check the remaining disk space. |
| -22 | SDB_DMS_EXIST | Collection already exist. | Check whether the collection exist or not. |

When error happen, use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)
to get the error message or use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md)
to get the error code. See [troubleshooting](troubleshooting/general/general_guide.md) for
more detail.

##HISTORY##
Since v1.0.

##EXAMPLES##
1. Create collection "bar" in collection space "foo" without shard key. If
   the collection space "foo" doesn't exist, it will automatically
   generate collection space "foo".

    ```lang-javascript
    > db.foo.createCL("bar")
    localhost:11810.foo.bar
    Takes 0.1250s.
    ```

2. Create a compressed normal collection "bar" in collection space "foo"
   with the hash shard key "age" and in ascending order, using 65535
   partitions with writeconcern 1.

    ```lang-javascript
    > db.foo.createCL("bar",{ShardingKey:{age:1},ShardingType:"hash",
      Partition:65535, ReplSize:1, Compressed:true, IsMainCL:false})
    localhost:11810.foo.bar
    Takes 0.32450s.
    ```
3. Create collection "bar" in collection space "foo" with the StrictDataMode trun on.
	If the collection space "foo" doesn't exist, it will automatically
   generate collection space "foo".

    ```lang-javascript
    > db.foo.createCL("bar", { StrictDataMode: true})
    localhost:11810.foo.bar
    Takes 0.1250s.
    ```


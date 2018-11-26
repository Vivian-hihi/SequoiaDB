##NAME##

insert - Insert record or records into the current collection.

##SYNOPSIS##

**db.collectionspace.collection.insert(\<doc|docs\>,[flag])**

**db.collectionspace.collection.insert(\<doc|docs\>,[options])**

##CATEGORY##

Collection

##DESCRIPTION##

Insert record or records into the current collection. If the current collection does not exist, please create it first.

**Note：**

When the provided record does not have an "_id" field, the database will add one for it.

##PARAMETERS##

* `doc/docs` ( *Object/Array of Object*， *Required* )

	Record or records to be inserted. Can not be null.

* `flag` ( *Int32*， *Optional* )

	Insert flag, use to control the behavior of inserting. Can be the follow value:

	* 0, default value.
	* SDB_INSERT_RETURN_ID, only available when inserting a single record, means return the value of "_id" field of the record.
	* SDB_INSERT_CONTONDUP, only available when 
bulk inserting records. This flag represent whether insert continue(no errors were reported) when hitting index key duplicate error.

* `options` ( *Object*， *Optional* )

	Options for control the behavior of inserting. Can be the follow options:

	* ContOnDup(Bool): represent whether insert continue(no errors were reported) when hitting index key duplicate error. 
	* ReturnOID(Bool): represent whether insert return the "_id" field of the record for user.

	Format: { ContOnDup: true, ReturnOID: false }

##RETURN VALUE##

On success, the follow result will be returned:

* When insert a single record with the flag of "SDB_INSERT_RETURN_ID", return the value of "_id" field(Only when the value of "_id" field is type of ObjectId. For the other element type, return a 12 bytes string which is "000000000000000000000000" ).
* When using the "ReturnOID" options to control the insert behavior, a Json object will be returned.
* Void for the other situations.

On error, exception will be thrown.

##ERRORS##

the exceptions of `insert()` are as below:

| Error code | Error type | Description | Solution |
| ------ | ------ | --- | ------ |
| -38 | SDB_IXM_DUP_KEY | The same unique index value conflicts with this record. | Check the record to make sure no duplicate  index key is offered. |
| -34 | SDB_DMS_CS_NOTEXIST | Collection space does not exist. | Check whether the collection space exist or not. |
| -23 | SDB_DMS_NOTEXIST| Collection does not exist. | heck whether the collection exist or not. |

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##HISTORY##

Since v1.0.

##EXAMPLES##

1. Insert a record.

	```lang-javascript
 	> db.foo.bar.insert( { name: "Tom", age: 20 } )
 	```

2. Insert some records. 

 	```lang-javascript
 	> db.foo.bar.insert( [ { _id: 20, name: "Mike", age: 15 }, { name: "John", age: 25, phone: 123 } ] )
 	```

3. Insert records with duplicate index keys.

	```lang-javascript
 	> db.foo.bar.insert( [ { _id: 1, a: 1 }, { _id: 1, b:2 }, { _id: 3, c: 3 } ],  SDB_INSERT_CONTONDUP )
 	```

 	```lang-javascript
 	> db.foo.bar.find()
 	{
      	"_id": 1,
      	"a": 1,
 	}
 	{
      	"_id": 3,
      	"c": 3
 	}
 	```

4. Insert record, and return Json object.

	```lang-javascript
 	> db.foo.bar.insert({a:1}, {ReturnOID:true, ContOnDup:true})
 	{
   		"_id": {
     		"$oid": "5becec3d6404b9295a63caca"
   		}
 	}
	```

	```lang-javascript
 	> db.foo.bar.insert([{a:1}, {b:1}], {ReturnOID:true, ContOnDup:true})
 	{
   		"_id": [
     		{
       			"$oid": "5bececdf6404b9295a63cacb"
     		},
     		{
       			"$oid": "5bececdf6404b9295a63cacc"
     		}
   		]
 	}
 	```
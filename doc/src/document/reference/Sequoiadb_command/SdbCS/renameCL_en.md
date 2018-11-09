##NAME##

renameCL - Rename collection.

##SYNOPSIS##

**db.collectionspace.renameCL( \<oldname\>, \<newname\> )**

##CATEGORY##

SdbCS

##DESCRIPTION##

Rename collection.

##PARAMETERS##

* `oldname` ( *String*， *Required* )

	The name of the collection. 

* `newname` ( *String*， *Required* )

	New name of the collection. 

**Note:**

1. Do not allow to connect data node to rename collection.

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

the exceptions of `renameCS()` are as below:

| Error Code | Error Type | Description | Solution |
| ------ | --- | ------------ | ----------- |
| -23 | SDB_DMS_NOTEXIST | The cl corresponding to "oldname" does not exist. | Rename an existing collection. |
| -22 | SDB_DMS_EXIST | The cl corresponding to "newname" already exists. | Set newname to a name that does not exist in the database. |

When error happen, use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)
to get the error message or use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md)
to get the error code. See [troubleshooting](troubleshooting/general/general_guide.md) for
more detail.

##HISTORY##

* since v3.0.1

##EXAMPLES##

1. Rename cl foo.bar to foo.bar_new.

	```lang-javascript
	> db = new Sdb( "localhost", 11810 )
	> db.foo.renameCL( "bar", "bar_new" )
	```
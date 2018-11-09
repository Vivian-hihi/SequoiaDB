##NAME##

renameCS - Rename collection space.

##SYNOPSIS##

**db.renameCS( \<oldname\>, \<newname\> )**

##CATEGORY##

Sdb

##DESCRIPTION##

Rename collection space.

##PARAMETERS##

* `oldname` ( *String*， *Required* )

	The name of the collection space. 

* `newname` ( *String*， *Required* )

	New name of the collection space. 

**Note:**

1. Do not allow to connect data node to rename collection spaces.

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

the exceptions of `renameCS()` are as below:

| Error Code | Error Type | Description | Solution |
| ------ | --- | ------------ | ----------- |
| -34 | SDB_DMS_CS_NOTEXIST | The cs corresponding to "oldname" does not exist. | Rename an existing collection space. |
| -33 | SDB_DMS_CS_EXIST | The cs corresponding to "newname" already exists. | Set newname to a name that does not exist in the database. |

When error happen, use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)
to get the error message or use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md)
to get the error code. See [troubleshooting](troubleshooting/general/general_guide.md) for
more detail.

##HISTORY##

* since v3.0.1

##EXAMPLES##

1. Rename cs foo to foo_new.

	```lang-javascript
	> db = new Sdb( "localhost", 11810 )
	> db.renameCS( "foo", "foo_new" )
	```
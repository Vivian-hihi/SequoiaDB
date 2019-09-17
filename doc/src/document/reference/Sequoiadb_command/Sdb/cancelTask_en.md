##NAME##

cancelTask - cancel task.

##SYNOPSIS##
**db.cancelTask( \<id\>, [isAsync] )**

##CATEGORY##

Sdb

##DESCRIPTION##

cancel task.

##PARAMETERS##

* `id` ( *Int32*, *Required* )

	Task id.

* `isAsync` ( *Bool*, *Required* )

	Whether the asynchronous.


##RETURN VALUE##

On success, return an object of Collection.

On error, exception will be thrown.

##ERRORS##

the exceptions of `cancelTask ()` are as below:

| Error Code | Error Type | Description | Solution |
| ------ | --- | ------------ | ----------- |
| | | | |


when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##HISTORY##

since v1.2

##EXAMPLES##

1. Stop split task.

	```lang-javascript
	> var taskid1 = db.foo.bar.splitAsync( "group1", "group2", 50 );
	> db.cancelTask( taskid1, true )
	```
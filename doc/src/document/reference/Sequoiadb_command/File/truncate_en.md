##NAME##

truncate - Truncate a file.

##SYNOPSIS##

***File.truncate( \[size\] )***

##CATEGORY##

File

##DESCRIPTION##

Truncate a file.

##PARAMETERS##

| Name   | Type     | Default | Description                          | Required or not |
| ------ | -------- | ------- | ------------------------------------ | --------------- |
| size   | int      | 0       | The file is truncated to size bytes. | not             |

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Truncate a file.

```lang-javascript
> var file = new File( "/opt/sequoiadb/file.txt" )
> file.truncate()
```
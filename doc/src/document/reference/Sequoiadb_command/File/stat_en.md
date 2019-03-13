##NAME##

stat - Display file or file system status.

##SYNOPSIS##

***File.stat( \<filepath\> )***

##CATEGORY##

File.

##DESCRIPTION##

Display file or file system status

##PARAMETERS##

| Name     | Type   | Description | Required or not |
| -------- | ------ | ----------- | --------------- |
| filepath | string | file path   | yes             |

##RETURN VALUE##

On success, return file status information.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Display file or file system status.

```lang-javacript
> File.stat( "/opt/trunk/test/test_one" )
{
  "name": "/opt/trunk/test/test_one",
  "size": "0",
  "mode": "rw-r--r--",
  "user": "root",
  "group": "root",
  "accessTime": "2019-02-27 10:21:45.540159133 +0800",
  "modifyTime": "2019-02-27 10:21:45.540159133 +0800",
  "changeTime": "2019-02-27 10:21:45.540159133 +0800",
  "type": "regular file"
}
```
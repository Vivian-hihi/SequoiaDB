##NAME##

getEWD - Acquire current directory

##SYNOPSIS##

***System.getDiskInfo()***

##CATEGORY##

System

##DESCRIPTION##

Acquire current directory

##PARAMETERS##

NULL

##RETURN VALUE##

On success, return disk information.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Acquire current directory

```lang-javascript
> System.getEWD()
/opt/trunk/bin
```
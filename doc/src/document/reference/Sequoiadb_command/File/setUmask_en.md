##NAME##

setUmask - Set file mode creation mask.

##SYNOPSIS##

***File.setUmask( \<umask\> )***

##CATEGORY##

File

##DESCRIPTION##

Set file mode creation mask.

##PARAMETERS##

| Name  | Type | Description                 | Required or not |
| ----- | ---- | --------------------------- | --------------- |
| umask | int  | permission mask of new file | yes             |

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Set file mode creation mask.

```lang-javacript
> File.getUmask( 8 )
0022
> File.setUmask( 0664 )
> File.getUmask( 8 )
0644
```
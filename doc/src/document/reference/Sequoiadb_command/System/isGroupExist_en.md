##NAME##

isGroupExist - Determine if a user group exists

##SYNOPSIS##

***System.isGroupExist( \<name\> )***

##CATEGORY##

System

##DESCRIPTION##

Determine if a user group exist

##PARAMETERS##

| Name      | Type     | Default | Description     | Required or not |
| ------- | -------- | ------------ | ------------ | -------- |
| name     | string   | ---    | user group name   | yes       |

##RETURN VALUE##

On success, return true or false.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Determine if a user group exist

```lang-javascript
> System.isGroupExist( "root" )
true
```
##NAME##

delUser - Delete a user

##SYNOPSIS##

***System.delUser( \<users\> )***

##CATEGORY##

System

##DESCRIPTION##

Delete a user

##PARAMETERS##

| Name      | Type     | Default | Description         | Required or not |
| ------- | -------- | ------------ | ---------------- | -------- |
| users | JSON   | ---    |  user information  | yes   |

The detail description of 'users' parameter is as follow:

| Attributes | Type    | Required or not | Format  | Description         |
| ---------- | ------- |---------------- | ------- | ---------------- |
| name    | string |   yes  | { "name": newUser }     | user name  |
| group    | string |  not   | { "group": newUser }     | user group name  |

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Delete a user

```lang-javascript
> System.delUser( { "name": "newUser2" } )
```
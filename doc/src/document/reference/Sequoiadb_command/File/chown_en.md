##NAME##

chown - Changes the user and/or group ownership of each given file.

##SYNOPSIS##

***File.chown( \<filepath\>, \<options\>, \[recursive\] )***

##CATEGORY##

File

##DESCRIPTION##

Changes the user and/or group ownership of each given file.

##PARAMETERS##

| Name      | Type     | Defaults | Description                  | Required or not |
| --------- | -------- | -------- | ---------------------------- | --------------- |
| filepath  | string   | ---      | source file path             | yes             |
| options   | JSON     | ---      | usename or groupname         | yes             |
| recursive | boolean  | false    | whether recursive processing | not             |

The detailed description of 'options' parameter is as follows:

| Name      | Type   | Required or not | Format                     | Description |
| --------- | ------ | --------------- | -------------------------- | ----------- |
| username  | string | not             | { username: "username" }   | username    |
| groupname | string | not             | { groupname: "groupname" } | groupname   |

>Note:

>Two parameters, username and groupname, must specify one of them.

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Changes the user and/or group ownership of each given file.

```lang-javascript
> File.chown( "/opt/trunk/file", { "username": "sequoiadb" }, false )
  // or
> File.chown( "/opt/trunk/file", { "groupname": "sequoiadbGroup" }, true )
```
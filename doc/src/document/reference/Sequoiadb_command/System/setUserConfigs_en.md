##NAME##

setUserConfigs - Setting user configuration

##SYNOPSIS##

***System.setUserConfigs( \<optionObj\> )***

##CATEGORY##

System

##DESCRIPTION##

Setting user configuration

##PARAMETERS##

| Name      | Type     | Default | Description         | Required or not |
| ------- | -------- | ------------ | ---------------- | -------- |
| optionObj  | JSON   | ---    |  new user configuration   | yes       |

The detail description of 'optionObj' parameter is as follow:

| Attributes | Type    | Required or not | Format  | Description         |
| ---------- | ------- |---------------- | ------- | ---------------- |
| name    | String |    yes   | { name: "username" }     |  username     |
| Group    | String |  not   | { Group: "groupname" }     |  new group name     |
| dir    | String |  not   | { dir: "dir" }     |  new home directory    |

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Add a user

  ```lang-javascript
  > System.setUserConfigs( { "name": "username", "Group": "groupname", "dir": "/home/username" } )
  ```
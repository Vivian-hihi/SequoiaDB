##NAME##

addAHostMap - Modify process resource limits

##SYNOPSIS##

***System.setProcUlimitConfigs( \<configsObj\> )***

##CATEGORY##

System

##DESCRIPTION##

Modify process resource limits

##PARAMETERS##

| Name      | Type     | Default | Description         | Required or not |
| ------- | -------- | ------------ | ---------------- | -------- |
| configsObj  | JSON   | ---  | new resource limits   | 是       |

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Modify the maximum memory size of the process

  ```lang-javascript
  > System.setProcUlimitConfigs( { "max_memory_size": -1, "open_files": 1024} )
  ```
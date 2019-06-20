##NAME##

isProcExist - Determine if a process exists

##SYNOPSIS##

***System.isProcExist( \<optionObj\> )***

##CATEGORY##

System

##DESCRIPTION##

Determine if a process exist

##PARAMETERS##

| Name      | Type     | Default | Description         | Required or not |
| ------- | -------- | ------------ | ---------------- | -------- |
| optionObj | JSON   | ---    |  process information  | yes   |

The detail description of 'optionObj' parameter is as follow:

| Attributes | Type    | Required or not | Format  | Description         |
| ---------- | ------- |---------------- | ------- | ---------------- |
| value   | string |  yes   | { value: "31831" }  | value of the specified type |
| type    | string | not  |  { type: "pid" }    | specified type |

##RETURN VALUE##

On success, return true or false.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Judging by the specified pid：

  ```lang-javascript
  > System.isProcExist( { value: "31831", type: "pid" } )
  true
  ```

* Judging by the specified service name：

  ```lang-javascript
  > System.isProcExist( { value: "sdbcm(11790)", type: "name" } )
  true
  ```
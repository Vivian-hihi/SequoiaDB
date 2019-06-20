##NAME##

listLoginUsers - List the information of logged-in users

##SYNOPSIS##

***System.listLoginUsers( \[options\], \[filter\] )***

##CATEGORY##

System

##DESCRIPTION##

List the information of logged-in users

##PARAMETERS##


| Name      | Type     | Description                             | Required or not |
| --------- | -------- | -----------------------------           | -------- |
| options   | JSON     | search pattern                          | not       |
| filter    | JSON     | filter, display all by default          | not      |

The detail description of 'options' parameter is as follow:

| Attributes | Type    | Required or not | Format  | Description         |
| ---------- | ------- |---------------- | ------- | -------------- |
| detail    | Bool |   not   | { detail: true }     | whether to display details   |

##RETURN VALUE##

On success, return the information of user groups.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* List all users

  ```lang-javascript
  > System.listLoginUsers()
  {
    "user": "sequoiadb"
  }
  {
    "user": "username"
  }
  ...
  ```

* Filter the results:

 ```lang-javascript
 > System.listLoginUsers( { detail: true }, { "tty": "tty1" } )
 {
   "user": "sequoiadb",
   "time": "2019-05-10 18:37",
   "from": "",
   "tty": "tty1"
 }
 ```

##NAME##

remove - Delete file.

##SYNOPSIS##

***File.remove( \<filepath\> )***

##CATEGORY##

File

##DESCRIPTION##

Delete file.

##PARAMETERS##

| Name     | Type     | Description | Required or not |
| -------- | -------- | ----------- | --------------- |
| filepath | string   | file path   | yes             |

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* View the file in the 'test' directory;

```lang-javascript
> File.list( { pathname: "/opt/trunk/test" } )
{
  "name": "test_twe",
  "mode": "-rw-r--r--",
  "user": "root"
}
{
  "name": "test_one",
  "mode": "-rw-r--r--",
  "user": "root"
}

* Delete the file named 'test_twe' in the 'test' directory;

```lang-javascript
> File.remove( "/opt/trunk/test/test_twe" )
```

* View the file in the 'test' directory again.

```lang-javascript
> File.list( { pathname: "/opt/trunk/test" } )
{
  "name": "test_one",
  "mode": "-rw-r--r--",
  "user": "root"
}
```
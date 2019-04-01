##NAME##

clear() - Clear buffer content.

##SYNOPSIS##

***clear()***

##CATEGORY##

FileContent

##DESCRIPTION##

Clear buffer content.

##PARAMETERS##

NULL

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Open a banary file and get a file descriptor;

```lang-javascript
> var binaryFile = new File( "/opt/trunk/test.dump" )
```

* Read the contents of the file into the fileContent object;

```lang-javascript
> var content = binaryFile.readContent( 10000 )
```

* Clear buffer content.

```lang-javascript
> content.clear()
```
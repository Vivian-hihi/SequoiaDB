##NAME##

writeContent - Write a fileContent object to file.

##SYNOPSIS##

***File.writeContent( \<fileContent\> )***

##CATEGORY##

File

##DESCRIPTION##

Write to a fileContent object to file.

##PARAMETERS##

| Name        | Type               | Description                 | Required or not |
| ----------- | ------------------ | --------------------------- | --------------- |
| fileContent | fileContent object | what is written to the file | yes             |

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Open a binary file and get a file descriptor;

```lang-javascript
> var test = new File( "/opt/sequoiadb/test.dump" )
```

* Read the contents of the file into the fileContent object;

```lang-javascript
> var content = test.readContent()
> content instanceof FileContent
true
```

* Write file.

```lang-javascript
> var file = new File( "/opt/trunk/file.dump" )
> file.writeContent( content )
```
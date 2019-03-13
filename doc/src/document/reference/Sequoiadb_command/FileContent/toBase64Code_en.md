##NAME##

toBase64Code() - Convert binary stream to base64 encoded.

##SYNOPSIS##

***toBase64Code()***

##CATEGORY##

FileContent

##DESCRIPTION##

Convert binary stream to base64 encoded.

##PARAMETERS##

NULL

##RETURN VALUE##

On success, return the string in base64 encoded format of binary stream.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Open a banary file and get a file descriptor;

```lang-javascript
> var binaryFile = new File( "/opt/trunk/test.dump" )
```

* Read the contents of the file into the fileContent object;

```lang-javacript
> var content = binaryFile.readContent( 10000 )
```

* Convert the binary stream in the fileContent to base64 encoded;

```lang-javascript
> var base64String = content.toBase64Code()
```

* You can write the string after conversion to a new file, so you can easily view the string.

```lang-javascript
> var base64StringFile = new File( "/opt/trunk/test.dump.base64" ) 
> base64StringFile.write( base64String )
```
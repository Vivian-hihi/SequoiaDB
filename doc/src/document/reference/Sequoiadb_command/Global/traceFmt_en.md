##NAME##

traceFmt - Format trace input to output by type.

##SYNOPSIS##

**traceFmt(\<formatType\>,\<input\>,\<output\>)**

##CATEGORY##

Global

##DESCRIPTION##

The trace file exported by db.traceOff() is in binary format and is not convenient for users to read. This command can be used to format the trace file into user-readable content and output it to the specified file. 

##PARAMETERS##

* `formatType` ( *Int32*, *Required* )

	`formatType` can take the following two values:
	* 0: Output analysis file, including thread execution sequence and other information(output file suffix is .flw).
	* 1: Output dump record information(output file suffix is .fmt).

	*Note:*
	
	When `formatType` is 0, it is accompanied by three other files: execution time analysis(csv file), execution time peak(except file), trace record error information(error file). When `formatType` is 1, no these three files.
 

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

the exceptions of `traceFmt()` are as below:

| Error code | Error type | Description | Solution |
| ------ | ------ | --- | ------ |
| -3 | SDB_PERM | Permission Error. | Check the path of the output file is ok or not. |
| -4 | SDB_FNE | File Not Exist. | Check the input file exist or not. |
| -6 | SDB_INVALIDARG | Invalid Argument. | Check the input format type is valid or not. |
| -189 | SDB_PD_TRACE_FILE_INVALID | Trace file is not valid. | Check the input file is ok or not.	|

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##HISTORY##

Since v1.0.

##EXAMPLES##

1. 格式化输出文件

	```lang-javascript
	> traceFmt( 0, "/opt/sequoiadb/trace.dump", "/opt/sequoiadb/trace_output" )
 	```

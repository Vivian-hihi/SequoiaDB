##NAME##

scp - Remote copy file.

##SYNOPSIS##

***File.scp( \<srcFile\>, \<dstFile\>, \[isreplace\], \[mode\] )***

##CATEGORY##

File

##DESCRIPTION##

Remote copy file.

##PARAMETERS##

| Name    | Type     | Defaults | Description                     | Required or not |
| ------- | -------- | -------- | ------------------------------- | --------------- |
| srcFile | string   | ---      | source file path                | yes             |
| dstFile | string   | ---      | destination file path           | yes             |
| replace | boolean  | false    | whether replace the source file | not             |
| mode    | int      | 0644     | set file permissions            | not             |

>Note:

>The specific format of parameters srcFile and dstFile are "ip:sdbcmPort@filepath",for example "192.168.20.71:11790@/opt/trunk/test/test_one".

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Remote copy file

```lang-javascript
> File.scp( "192.168.20.71:11790@/opt/trunk/test/test_one",  "192.168.20.71:11790@/opt/trunk/test/test_four" )
Success to copy file from 192.168.20.71:11790@/opt/trunk/test/test_one to 192.168.20.71:11790@/opt/trunk/test/test_four
```
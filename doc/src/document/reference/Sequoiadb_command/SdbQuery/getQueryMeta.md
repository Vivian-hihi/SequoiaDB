##语法##
***query.getQueryMeta()***

获取查询元数据信息。

##参数描述##

无

##返回值##

返回查询元数据信息，出错抛异常，并输出错误信息，可以通过
[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。


##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取查询元数据信息

```lang-javascript
 > query.getQueryMeta()
 {
   "HostName": "ubuntu",
   "ServiceName": "42000",
   "NodeID": [
     1001,
     1003
   ],
   "ScanType": "tbscan",
   "Datablocks": [
     9
   ]
 } 
```
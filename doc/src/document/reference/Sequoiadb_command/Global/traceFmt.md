##名称##

traceFmt - 将 db.traceOff() 导出来的 trace 文件格式化为用户可读的内容，
           并输出到指定文件。

##语法##

**traceFmt(\<formatType\>,\<input\>,\<output\>)**

##类别##

Global

##描述##

db.traceOff() 导出来的 trace 文件为二进制格式的内容，不便于用户阅读。可通过该命令将 trace 文件格式化为用户可读的内容，并输出到指定文件。

##参数##

* `formatType` ( *Int32*， *必填* )

	格式类型。

* `input` ( *String*， *必填* )

	db.traceOff() 导出来的二进制文件。

* `output` ( *String*， *必填* )

	格式化后输出的可读文件。

##返回值##

成功：无返回值。

失败：抛出异常。

##错误##

`traceFmt()`函数常见异常如下：

| 错误码 | 错误类型 | 可能的原因 | 解决方法 |
| ------ | --- | ------ | ------ |
| -189 | SDB_PD_TRACE_FILE_INVALID | 输入的trace文件不合法| 确认输入的文件是否合法	|

当异常抛出时，可以通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取[错误码](reference/Sequoiadb_error_code.md)，
或通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息。
可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)了解更多内容。

##版本##

v1.0及以上版本。

##示例##

1. 格式化输出文件

	```lang-javascript
	> traceFmt( 0, "/opt/sequoiadb/trace.dump", "/opt/sequoiadb/trace.flw" )
 	```
##语法##
***db.resetSnapshot( [cond] )***

重置快照。

##参数描述##

| 参数名 | 参数类型  | 描述 	| 是否必填 |
| ------ | --------- | -------- | -------- |
| cond 	 | Json 对象 | 匹配条件，只重置符合 cond 条件的快照记录，为 null 时，重置所有。 | 否 |

##返回值##
无返回值，出错抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

##错误##
常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 重置 SessionID 大于1的快照。

	```lang-javascript
	> db.resetSnapshot({SessionID:{$gt:1}})
	```

##语法##
***db.setSessionAttr ( \<options\> )***

设置会话属性。

## 参数描述##

| 参数名 	| 参数类型 	| 描述 			| 是否必填 	|
| --------- | --------- | ------------- | ----------|
| options 	| Json 对象 | 会话属性选项	| 是 		|

###options 格式###

| 属性名 | 描述 	| 格式 		|
| ------ | ------ 	| ------ 	|
| PreferedInstance 	| 会话读操作优先选取的数据库实例标识；取值"m"/"M"/"s"/"S"/"a"/"A"/1-7，分别表示 master/slave/anyone/node1-node7 | PreferedInstance:"M" |

##返回值##
无返回值，出错抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

##错误##
常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 设置会话优先从“主”数据库实例获取数据

	```lang-javascript
	> db.setSessionAttr( { PreferedInstance: "M" } )
	```

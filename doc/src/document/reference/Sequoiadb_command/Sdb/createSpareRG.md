##语法##
***db.createSpareRG()***

增加SYSSpare组，用于管理后备节点。

##返回值##

返回SYSSpare组的引用，出错抛异常，并输出错误信息，可以通过 [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) 获取错误信息 或 通过 [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) 获取错误码。关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md) 。

##示例##

- 创建一个SYSSpare分区组

 ```lang-javascript
 > db.createSpareRG()
 ```

##语法##
***db.resetSnapshot( [cond] )***

重置快照。

##参数描述##

| 参数名  | 参数类型 | 描述   | 是否必填 |
| ------- | -------- | ------ | -------- |
| options | Json 对象| 设定快照类型、会话号、[命令位置参数](reference/Sequoiadb_command/Overview/location.md) | 否 |

1. **options 格式**

 | 属性名 | 描述   | 默认值 | 格式 |
 | ------ | ------ | -------| ---- |
 | Type   | 指定重置的[快照类型](database_management/monitoring/snapshot/snapshot.md)。取值：<br/>"sessions"<br/>"sessions current"<br/>"database"<br/>"health"<br/>"all" | "all" | Type: "sessions" |
 | SessionID | 指定重置的会话ID。 | 所有会话 | SessionID: 1 |
 | Location Elements | 命令位置参数项，详细见 [命令位置参数](reference/Sequoiadb_command/Overview/location.md) | 所有节点 | GroupName:"db1" |

 > **Note:**
 >
 > * Type: "all" 表示重置所有快照。
 > * SessionID 字段只在 Type: "sessions" 才生效。

##返回值##
无返回值，出错抛异常，并输出错误信息。可以通过 [getLastErrObj()](reference/Sequoiadb_command/Global/getLastErrObj.md)  或 [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) 获取错误信息 或 通过 [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) 获取错误码。

关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

##错误##

| [错误码](reference/Sequoiadb_error_code.md) | 错误类型 | 描述 | 解决方法 |
| ---- | -------------- | ------------ | ------------ |
| -6   | SDB_INVALIDARG | 输入参数非法 | 参考以上描述 |
##示例##

* 重置 SessionID 为1的快照。

  ```lang-javascript
  > db.resetSnapshot( { Typ: "sessions", SessionID: 1 } )
  ```

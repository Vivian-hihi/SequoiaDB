##语法##

***db.unloadCS( \<csName\>, [options] )***

##类别##

Sdb

##描述##

卸载集合空间。

##参数##

| 参数名  | 参数类型 | 默认值  | 描述               | 是否必填 |
| ------- | -------- | ------- | ------------------ | -------- |
| csName  | string   | ---     | 集合空间名         | 是       |
| options | string   | 空      | 指定集合空间的信息 | 否       |

options 参数详细说明如下：

| 属性      | 值类型 | 描述       | 是否<br>必填 |
| --------- | ------ | ---------- | ------------ |
| GroupID   | int    | 复制组 ID  | 否           |
| GroupName | string | 复制组名称 | 否           |
| NodeID    | int    | 节点 ID    | 否           |
| HostName  | string | 主机名     | 否           |
| svcname   | string | 节点的端口 | 否           |

>**Note:**

>只有在连接协调节点时，options 参数才会生效

##返回值##

无返回值。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 卸载集合空间 “foo” 。（假定存在集合空间 “foo” ）

  ```lang-javascript
  > db.unloadCS( "foo" )
  ```
	
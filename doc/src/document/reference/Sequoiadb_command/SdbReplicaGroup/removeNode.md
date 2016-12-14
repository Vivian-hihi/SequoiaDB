##语法##
***rg.removeNode( \<host\>, \<service\>, [config] )***

删除当前分区组中的指定节点。

##参数描述##

| 参数名  | 参数类型   | 描述           | 是否必填 |
|---------|------------|----------------|----------|
| host    | string     | 节点主机名。   | 是       |
| service | int/string | 节点端口号。   | 是       |
| config  | Json 对象  | 节点配置信息。 | 否       |

> **Note:**  
> 格式：( "<主机名>", "<端口号>", [ { <configParam>: value, ... } ] )

##返回值##

无返回值，出错抛异常，并输出错误信息。可以通过 [getLastErrMsg](reference/Sequoiadb_command/Global/getLastErrMsg.md) 获取错误信息，或通过 [getLastError](reference/Sequoiadb_command/Global/getLastError.md) 获取错误码。关于错误处理可以参考 [常见错误处理指南](troubleshooting/general/general_guide.md) 。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

## 示例##

删除 group1 分区组中节点

```lang-javascript
> var rg = db.getRG("group1")
> rg.removeNode("vmsvr2-suse-x64",11800)
```


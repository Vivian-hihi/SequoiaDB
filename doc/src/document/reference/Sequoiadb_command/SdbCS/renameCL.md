##语法##
***db.collectionspace.renameCL( \<oldname\>, \<newname\> )***

集合改名。

##参数描述##

| 参数名  | 参数类型 | 描述             | 是否必填 |
| ------  | ------   | ------           | ------ |
| oldname | 字符串   | 要修改的集合名。 | 是 |
| newname | 字符串   | 集合新名字。     | 是 |

>**Note:**
>
> 不允许直连数据节点，对集合改名。

##返回值##

无返回值，出错抛异常，并输出错误信息。

##错误##

常见异常如下：

| [错误码](reference/Sequoiadb_error_code.md)| 错误类型 | 描述 | 解决方法 |
| ------| ----------------------- | --- | ------ |
| -23   | SDB_DMS_NOTEXIST | oldname对应的集合不存在。 | 对已存在的集合执行rename。 |
| -22   | SDB_DMS_EXIST    | newname对应的集合已存在。 | newname设为不存在的名字。 |

当异常抛出时，可以通过 [getLastErrObj()](reference/Sequoiadb_command/Global/getLastErrObj.md)  或 [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) 获取错误信息 或 通过 [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) 获取错误码。
更多错误可以参考[常见错误处理指南](troubleshooting/general/general_guide.md) 。

##版本信息##
3.0.1及以上版本

##示例##

对集合foo.bar，改名为foo.bar_new 。

 ```lang-javascript
 // 连接协调节点
 > db = new Sdb( "localhost", 11810 )
 > db.foo.renameCL( "bar", "bar_new" )
 ```


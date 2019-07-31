##语法##

***var securesdb = new SecureSdb( [hostname], [svcname] )***

##类别##

SecureSdb

##描述##

新建一个 SecureSdb 对象。

> **Note:**

> 1. SecureSdb 是 Sdb 的子类，SecureSdb 的对象使用 SSL 连接；

> 2. SecureSdb 对象和 Sdb 对象的方法和语法一致。 

##参数##

| 参数名   | 参数类型 | 默认值            | 描述         | 是否必填 |
| -------- | -------- | ----------------- | ------------ | -------- |
| hostname | string   | localhost         | 主机 IP 地址 | 否       |
| svcname  | int      | 本地 sdbcm 的端口 | sdbcm 的端口 | 否       |

##返回值##

无返回值。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 新建一个 SecureSdb 对象

   ```lang-javascript
   > var securesdb = new SecureSdb( "192.168.20.71", 11790 )
   ```
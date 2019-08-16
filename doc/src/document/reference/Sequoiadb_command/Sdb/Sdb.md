##名称##

Sdb - SequoiaDB 连接对象。 

##语法##

**var db = new Sdb([hostname],[svcname])**

**var db = new Sdb([hostname],[svcname],[username],[password])**

##类别##

Sdb

##描述##

新建一个 Sdb 对象，用于连接 SequoiaDB。

##参数##

| 参数名   | 参数类型 | 默认值            | 描述         | 是否必填 |
| -------- | -------- | ----------------- | ------------ | -------- |
| hostname | string   | localhost         | 主机名 | 否     |
| svcname  | int      | 11810 | 节点端口号 | 否     |
| username  | string      | 默认为空（''） | 用户名 | 否     |
| password  | string      | 默认为空（''）| 密码 | 否     |


> **Note:**

> 1. 可以通过 [createUsr()](reference/Sequoiadb_command/Sdb/createUsr.md) 创建 SequoiaDB 的用户，并设置对应的密码。

> 2. 当 SequoiaDB 没有用户时，创建 Sdb 对象可以不使用 username 和 password，否则必须使用相应的 username 和 password 去创建 Sdb 对象。


##返回值##

成功：返回 Sdb 对象。  

失败：抛出异常。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##版本##

v1.12及以上版本。

##示例##

1. 连接默认主机上的 SequoiaDB，hostname 默认为：localhost，svcname 默认为 11810。

	```lang-javascript
 	> var db = new Sdb()
 	```

2. 连接指定机器上的 SequoiaDB，目标机器："sdbserver1"。

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810)
	```

3. 连接指定机器上的 SequoiaDB，如果没有设置用户名和密码，则用户名和密码可以为空。

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810,"","")
	```

4. 使用用户名和密码连接指定机器上的 SequoiaDB。

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810,"admin","123")
	```

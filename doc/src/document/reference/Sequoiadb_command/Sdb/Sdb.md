##Sdb()##
##名称##

Sdb - 连接 sequoiadb 

##语法##

**var db = new Sdb([hostname],[svcname])**

**var db = new Sdb([hostname],[svcname],[username],[password])**

##类别##

Sdb

##描述##

新建一个 Sdb 对象，用于连接 sequoiadb

##参数##

* `hostname` ( *String*， *非必填* )

   主机名，默认为： "localhost"
   
* `svcname` ( *int*， *非必填* )

   端口号， 默认端口号为：11810。

* `username` ( *String*)

   sequoiadb 对应的用户名，如果安装时有设置用户名则为必填，未设置则为空。

* `password` ( *String* )

   sequoiadb 用户名对应的密码，如果安装时有设置密码则为必填，未设置则为空。

##返回值##

成功：返回 Sdb 对象。  

失败：抛出异常。

##错误##

`Sdb()`函数常见异常如下：

| 错误码 | 错误类型 | 描述 | 解决方法 |
| ------ | --- | ------------ | ----------- |
| -79 | SDB_NET_CANNOT_CONNECT | 无法连接指定的地址 | 检查地址、端口以及节点的配置信息是否正确。|

当异常抛出时，可以通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取[错误码](reference/Sequoiadb_error_code.md)，
或通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息。
可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)了解更多内容。

##版本##

v1.12及以上版本。

##示例##

1. 连接默认主机上的 sequoiadb，hostname 默认为：localhost，svcname 默认为 11810。

	```lang-javascript
 	> var db = new Sdb()
 	```

2. 连接指定机器上的 sequoiadb，目标主机："sdbserver1"。

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810)
	```

3. 连接指定机器上的 sequoiadb，如果安装 sequoiadb 时没有设置用户名和密码，则用户名和密码可以为空。

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810,"","")
	```

4. 连接指定机器上的 sequoiadb，安装 sequoiadb 时设置了用户名和密码则需要填写对应的用户名和密码。

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810,"sdbadmin","123")
	```

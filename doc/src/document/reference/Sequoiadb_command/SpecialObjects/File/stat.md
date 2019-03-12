##语法##

***File.stat( \<filepath\> )***

##类别##

File

##描述##

显示文件的状态信息

##参数##

| 参数名   | 参数类型 | 描述     | 是否必填 |
| -------- | -------- | -------- | -------- |
| filepath | string   | 文件路径 | 是       |

##返回值##

返回指定文件的状态信息。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 显示文件的状态信息。

  ```lang-javacript
  > File.stat( "/opt/trunk/test/test_one" )
  {
    "name": "/opt/trunk/test/test_one",
    "size": "0",
    "mode": "rw-r--r--",
    "user": "root",
    "group": "root",
    "accessTime": "2019-02-27 10:21:45.540159133 +0800",
    "modifyTime": "2019-02-27 10:21:45.540159133 +0800",
    "changeTime": "2019-02-27 10:21:45.540159133 +0800",
    "type": "regular file"
  }
  ```

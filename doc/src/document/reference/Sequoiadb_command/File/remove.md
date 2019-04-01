##语法##

***File.remove( \<filepath\> )***

##类别##

File

##描述##

删除文件。

##参数##

| 参数名   | 参数类型 | 描述     | 是否必填 |
| -------- | -------- | -------- | -------- |
| filepath | string   | 文件路径 | 是       |

##返回值##

无返回值。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 查看 test 目录下的文件；

  ```lang-javascript
  > File.list( { pathname: "/opt/trunk/test" } )
  {
    "name": "test_twe",
    "mode": "-rw-r--r--",
    "user": "root"
  }
  {
    "name": "test_one",
    "mode": "-rw-r--r--",
    "user": "root"
  }

* 删除 test 目录下的 test_twe 文件；

  ```lang-javascript
  > File.remove( "/opt/trunk/test/test_twe" )
  ```

* 再次查看 test 目录下的文件。

  ```lang-javascript
  > File.list( { pathname: "/opt/trunk/test" } )
  {
    "name": "test_one",
    "mode": "-rw-r--r--",
    "user": "root"
  }
  ```


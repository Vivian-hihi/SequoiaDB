##语法##

***System.listAllUsers( \[options\], \[filter\] )***

##类别##

System

##描述##

列出所有用户

##参数##

| 参数名    | 参数类型 | 描述                                     | 是否必填 |
| --------- | -------- | ---------------------------------------- | -------- |
| options   | JSON     | 查找模式和查找内容                       | 否       |
| filter    | JSON     | 筛选条件，不指定筛选条件默认显示全部内容 | 否       |

options 参数详细说明如下：

| 属性     | 值类型 | 是否<br>必填 | 格式 | 描述 |
| -------- | ------ | -------- | -------------------- | --------------- |
| detail    | Bool |     否   | { detail: true }     | 是否显示详细信息     |

filter 参数支持对结果集进行筛选。

##返回值##

返回查找的用户信息。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 默认显示所有用户；

  ```lang-javascript
  > System.listAllUsers( )
  {
    "user": "sequoiadb"
  }
  {
    "user": "sdbadmin"
  }
  ...
  ```

* 对结果进行筛选:

  ```lang-javascript
  > System.listAllUsers( { detail: true }, { "user": "sequoiadb" } )
  {
    "user": "sequoiadb",
    "gid": "1000",
    "dir": "/home/sequoiadb"
  }
  ```

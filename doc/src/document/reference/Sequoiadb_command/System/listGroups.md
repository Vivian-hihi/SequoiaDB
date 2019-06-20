##语法##

***System.listGroups( \[options\], \[filter\] )***

##类别##

System

##描述##

列出用户组信息

##参数##

| 参数名    | 参数类型 | 描述                                     | 是否必填 |
| --------- | -------- | ---------------------------------------- | -------- |
| options   | JSON     | 查找模式和查找内容                       | 否       |
| filter    | JSON     | 筛选条件，不指定筛选条件默认显示全部内容 | 否       |

options 参数详细说明如下：

| 属性     | 值类型 | 是否<br>必填 | 格式 | 描述 |
| -------- | ------ | -------- | -------------------- | ---------------------------------- |
| detail    | Bool |     否   | { detail: true }     | 是否显示详细信息                        |

filter 参数支持对结果集进行筛选。

##返回值##

返回查找的用户组信息。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 默认显示所有进程信息；

  ```lang-javascript
  > System.listGroups()
  {
    "name": "sequoiadb"
  }
  {
    "name": "lpadmin"
  }
  {
    "name": "sambashare"
  }
  {
    "name": "sdbadmin_group"
  }
  ...
  ```

* 对结果进行筛选:

  ```lang-javascript
  > System.listGroups( { detail: true }, { "name": "sequoiadb" } )
  {
    "name": "sequoiadb",
    "gid": "1000",
    "members": ""
  }
  ```



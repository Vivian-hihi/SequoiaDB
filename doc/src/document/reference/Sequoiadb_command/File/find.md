##语法##

***File.find( \<options\>, \[filter\] )***

##类别##

File

##描述##

查找文件

##参数##

| 参数名    | 参数类型 | 描述                                     | 是否必填 |
| --------- | -------- | ---------------------------------------- | -------- |
| options   | JSON     | 查找模式和查找内容                       | 是       |
| filter    | JSON     | 筛选条件，不指定筛选条件默认显示全部内容 | 否       |

options 参数详细说明如下：

| 属性     | 值类型 | 是否<br>必填 | 格式 | 描述 |
| -------- | ------ | -------- | -------------------- | ---------------------------------- |
| mode     | char   | 是       | { mode: 'n' }<br>{ mode: 'u' }<br>{ mode: 'g' }<br>{ mode: 'p' } | 根据文件名( filename )查找文件<br>根据用户名( username)查找文件<br>根据用户组名( groupname)查找文件<br>根据权限( perssion )查找文件 | 
| pathname | string | 否       | { pathname: "pathname" } | 指定查找的文件路径，默认为当前目录 |
| value    | string | 是       | { value: "content" }     | 查找的内容                         |

filter 参数支持对结果中的某些字段进行 and 、 or 、not 和精确匹配计算，对结果集进行筛选。

##返回值##

返回查找内容。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 查找文件；

  ```lang-javacript
  > File.find( { mode: "n", value: "tmp" } )
  {
    "pathname": "./tmp"
  }
  {
    "pathname": "./database/50000/tmp"
  }
  {
    "pathname": "./.svn/tmp"
  }
  {
    "pathname": "./bin/tmp"
  }
  ```

* 查找文件后，对结果进行筛选。

 ```lang-javasript
 > File.find( { mode: "n", value: "tmp" }, { $or: [ { pathname: "./bin/tmp" }, { pathname: "./database/41000/tmp" } ] } )
  {
    "pathname": "./database/41000/tmp"
  }
  {
    "pathname": "./bin/tmp"
  }
 ```

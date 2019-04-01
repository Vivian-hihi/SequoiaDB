##语法##

***File.list( \[options\], \[filter\] )***

##类别##

File

##描述##

列出当前目录的文件

##参数##

| 参数名  | 参数类型 | 描述                                     | 是否必填 |
| ------- | -------- | ---------------------------------------- | -------- |
| options | JSON     | 可选参数                                 | 否       |
| filter  | JSON     | 筛选条件，不指定筛选条件默认显示全部内容 | 否       |

options 参数详细说明如下：

| 属性     | 值类型  | 是否必填 | 格式                                    | 描述             |
| -------- | ------- |--------- | --------------------------------------- | ---------------- |
| detail   | boolean | 否       | { detail: true } 或者 { detail: false } | 是否显示详细内容 |
| pathname | string  | 否       | { pathname: "pathname" }                | 文件路径         |

参数 filter 支持对结果中的某些字段进行 and 、 or 、not 和精确匹配计算，对结果集进行筛选。

##返回值##

返回指定目录下的文件信息。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 列出当前目录的文件；

  ```lang-javascript
  > File.list( { detail: true, pathname: "/opt/trunk/test" } )
  {
    "name": "test_one",
    "size": "0",
    "mode": "-rw-r--r--",
    "user": "root",
    "group": "root",
    "lasttime": "2月 27 10:21"
  }
  {
    "name": "test_twe",
    "size": "0",
    "mode": "-rw-r--r--",
    "user": "root",
    "group": "root",
    "lasttime": "2月 27 10:22"
  }
  ```

* 列出当前目录的文件后，对结果进行筛选。

  ```lang-javascript
  > File.list( { detail: true, pathname: "/opt/trunk/test" }, { $and: [ { name: "test_one" }, { size: "0" } ] } )
  {
    "name": "test_one",
    "size": "0",
    "mode": "-rw-r--r--",
    "user": "root",
    "group": "root",
    "lasttime": "2月 27 10:21"
  }
  ```

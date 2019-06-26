##语法##

***Sdbtool.listNodes( [option], [filter], [rootPath] )***

##类别##

Sdbtool

##描述##

显示节点信息

##参数##

| 参数名   | 参数类型 | 默认值               | 描述                | 是否必填 |
| -------- | -------- | -------------------- | ------------------- | -------- |
| options  | JSON     | 默认显示所有节点     | 显示指定节点        | 否       |
| filter   | JSON     | 默认显示全部内容     | 筛选条件            | 否       |
| rootPath | string   | 默认系统配置文件路径 | 指定配置文件根路径  | 否       |

options 参数详细说明如下：

| 属性 | 值类型 | 默认值 | 格式 | 描述 |
| ---- | ------ | ------ | ---- | ---- |
| type | string |  db |{ type: "all" }<br>{ type: "db" }<br>{ type: "om" }<br>{ type: "cm" } |  显示所有的节点信息<br>显示所有的节点信息<br>显示 om 节点<br>显示 cm 节点 |
| mode | string | run |{ mode: "run" }<br>{ mode: "local" } | 显示正在运行的节点<br>显示本地节点，无论是否正在运行 |
| role      | string | --- | { role: "data" }<br>{ role: "coord" }<br>{ role: "catalog" }<br>{ role: "standalone" }<br>{ role: "om" }<br>{ role: "cm" } | 显示数据节点<br>显示协调节点<br>显示编目节点<br>独立模式下显示节点信息<br>显示 om 节点<br>显示 cm 节点 |
| svcname   | string | --- | { svcname: "11790" } | 显示指定端口的节点信息 |
| showalone | bool | false | { showalone：true }<br>{ showalone: false } | 独立模式下是否显示 cm 节点 |
| expand    | bool | false | { expand: true }<br>{ expand: false } | 是否显示详细的扩展配置 |

> Note：

> 1. 当指定多个 svcname 时，可以以 ‘,’ 隔开。

> 2. filter 参数支持对结果中的某些字段进行 and 、 or 、not 和精确匹配计算，对结果集进行筛选。

##返回值##

返回节点信息。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 显示节点信息；

  ```lang-javascript
  > Sdbtool.listNodes( { type: "all", mode: "local", role: "data", svcname: "20000, 40000" } )
  {
      "svcname": "20000",
      "type": "sequoiadb",
      "role": "data",
      "pid": 17390,
      "groupid": 1000,
      "nodeid": 1000,
      "primary": 1,
      "isalone": 0,
      "groupname": "db1",
      "starttime": "2019-05-31-17.14.14",
      "dbpath": "/opt/trunk/database/20000/"
  }
  {
      "svcname": "40000",
      "type": "sequoiadb",
      "role": "data",
      "pid": 17399,
      "groupid": 1001,
      "nodeid": 1001,
      "primary": 0,
      "isalone": 0,
      "groupname": "db2",
      "starttime": "2019-05-31-17.14.14",
      "dbpath": "/opt/trunk/database/40000/"
  }
  ```

* 显示节点信息后，对结果进行筛选。

  ```lang-javascript
  > Sdbtool.listNodes( { type: "all", mode: "local", role: "data", svcname: "20000, 40000" }, { groupname: "db2" } )
  {
      "svcname": "40000",
      "type": "sequoiadb",
      "role": "data",
      "pid": 17399,
      "groupid": 1001,
      "nodeid": 1001,
      "primary": 0,
      "isalone": 0,
      "groupname": "db2",
      "starttime": "2019-05-31-17.14.14",
      "dbpath": "/opt/trunk/database/40000/"
  }
  ```

##NAME##

listNodes - Display node information.

##SYNOPSIS##

***Sdbtool.listNodes( [option], [filter], [rootPath] )***

##CATEGORY##

Sdbtool

##DESCRIPTION##

Display node information.

##PARAMETERS##

| Name     | Type     | Default | Description | Required or not |
| -------- | -------- | ------- | ----------- | --------------- |
| options  | JSON     | display all nodes by default | display specified node | not |
| filter   | JSON     | display all information by default | Filtered conditions | not |
| rootPath | JSON     | system configuration filepath | specify the configuration file path | not |

The detail description of 'options' parameter is as follow:

| Attributes | Type | Default | Format | Description |
| ---------- | ---- | ------- | ------ | ----------- |
| type       | string | db  | { type: "all" }<br>{ type: "db" }<br>{ type: "om" }<br>{ type: "cm" } | display information about all nodes<br>display infomation about all nodes<br>display infomation about om node<br>display infomation about cm node |
| mode       | string | run | { mode: "run" }<br>{ mode: "local" } | display infomation about running nodes<br>display infomation about local nodes whether run or not | 
| role       | string | --- | { role: "data" }<br>{ role: "coord" }<br>{ role: "catalog" }<br>{ role: "standalone" }<br>{ role: "om" }<br>{ role: "cm" } | display infomation about data nodes<br>display infomation about coord nodes<br>display infomation about catalog nodes<br>display infomation about nodes in tandalone mode<br>display infomation about om node<br>display infomation about cm node |
| svcname    | string | --- | { svcname: "11790" } | display node information of the specified port | 
| showalone  | bool   | false | { showalone: true }<br>{ showalone: false } | whether to diplay information about om node in tandalone mode |
| expand     | bool   | false | { expand: true }<br>{ expand: false } | whether to display detailed extended configuration |

>Note:

>1. When specifying multiple svcnames, you can separate the svcnames with ",".

>2. The optional parameter filterObj supports the AND, the OR, the NOT and exact matching of some fields in the result, and the result set is filtered.

##RETURN VALUE##

On success, return rearch result set.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Display node information.

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

* Display node information and filter the result set

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
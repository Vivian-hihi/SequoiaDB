##NAME##

unloadCS -  Load the specific collection space.

##SYNOPSIS##

***db.loadCS( \<csName\>, [options] )***

##CATEGORY##

Sdb

##DESCRIPTION##

Load the specific collection space.

##PARAMETERS##

| Name    | Type   | Default | Description                          | Required or not |
| ------- | ------ | ------- | ------------------------------------ | --------------- |
| csName  | string | ---     | collection space name                | yes             |
| options | string | NULL    | specify collection space information | not             |

The detail description of 'options' parameter is as follow:

| Attributes | Type   | Description            | Required or not |
| ---------- | ------ | ---------------------- | --------------- |
| GroupID    | int    | replication group ID   | not             |
| GroupName  | string | replication group name | not             |
| NodeID     | int    | node ID                | not             |
| HostName   | string | hostname               | not             |
| svcname    | string | the port of the node   | not             |

>**Note:**

>Only when connecting to the coordination node, the options parameter will take effect.

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Load the specific collection space named "foo".(Suppose the specific collection space named "foo" existes)

```lang-javascript
> db.loadCS( "foo" )
```

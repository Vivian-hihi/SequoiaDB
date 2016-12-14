##语法##
***db.forceSession( \<sessionID\> )***

终止指定会话的当前操作。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| sessionID | int | 会话编号 | 是 |

> **Note:**
>
> * 只有用户会话可以被终止。
> * 终止会话，需要通过直连数据节点或编目节点后获取会话编号。会话编号可以通过[list()](reference\Sequoiadb_command\Sdb\list.md)或[snapshot()](reference\Sequoiadb_command\Sdb\snapshot.md)获取。
> * 如果终止的会话是当前会话，连接会被断开，不能再执行操作。

##返回值##

* 终止当前会话，连接被断开，抛出-16的错误码，错误信息显示网络已从远处关闭。
* 终止其他会话，无返回值，出错抛异常，并输出错误信息，可以通过 [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) 获取错误信息 或 通过 [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) 获取错误码。关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md) 。

##示例##

* 终止直连数据节点的当前会话。

 ```lang-javascript
 // 连接数据节点
 > db = new Sdb( "localhost", 11830 )
 // 获取当前会话
 > db.list( SDB_LIST_SESSIONS_CURRENT )
 {
   "NodeName": "sdbserver1:11830",
   "SessionID": 35,
   "TID": 14192,
   "Status": "Running",
   "Type": "Agent",
   "Name": "127.0.0.1:39179",
   "RelatedID": "c0a81f3d2e3600003770"
 }
 // 终止当前会话，连接断开
 > db.forceSession( 35 )
 (nofile):0 uncaught exception: -16
 Network is closed from remote
 ```
* 终止直连数据节点的指定会话（非当前会话）

 ```lang-javascript
 // 连接数据节点
 > db = new Sdb( "localhost", 11830 )
 // 获取所有会话
 > db.list( SDB_LIST_SESSIONS )
 {
   "NodeName": "sdbserver1:11830",
   "SessionID": 44,
   "TID": 12962,
   "Status": "Running",
   "Type": "Task",
   "Name": "DATASYNC-JOB-D",
   "RelatedID": "c0a81f3d2e36000032a2"
 }....
 // 终止编号为44的会话
 > db.forceSession( 44 )
 ```

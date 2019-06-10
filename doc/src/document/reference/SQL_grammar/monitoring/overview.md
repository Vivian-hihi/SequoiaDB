监控是一种监视当前系统状态的方式。在 SequoiaDB 中，用户可以使用快照（SNAPSHOT）与列表（LIST）命令进行系统监控。

##快照视图##

[快照](reference/SQL_grammar/monitoring/snapshot.md) 是一种得到系统当前状态的命令，主要分为以下类型：

*   [上下文快照](reference/SQL_grammar/monitoring/$SNAPSHOT_CONTEXT.md)
*   [当前会话上下文快照](reference/SQL_grammar/monitoring/$SNAPSHOT_CONTEXT_CUR.md)
*   [会话快照](reference/SQL_grammar/monitoring/$SNAPSHOT_SESSION.md)
*   [当前会话快照](reference/SQL_grammar/monitoring/$SNAPSHOT_SESSION_CUR.md)
*   [集合快照](reference/SQL_grammar/monitoring/$SNAPSHOT_CL.md)
*   [集合空间快照](reference/SQL_grammar/monitoring/$SNAPSHOT_CS.md)
*   [数据库快照](reference/SQL_grammar/monitoring/$SNAPSHOT_DB.md)
*   [系统快照](reference/SQL_grammar/monitoring/$SNAPSHOT_SYSTEM.md)
*   [编目信息快照](reference/SQL_grammar/monitoring/$SNAPSHOT_CATA.md)
*   [事务快照](reference/SQL_grammar/monitoring/$SNAPSHOT_TRANS.md)
*   [当前事务快照](reference/SQL_grammar/monitoring/$SNAPSHOT_TRANS_CUR.md)
*   [访问计划缓存快照](reference/SQL_grammar/monitoring/$SNAPSHOT_ACCESSPLANS.md)
*   [节点健康检测快照](reference/SQL_grammar/monitoring/$SNAPSHOT_HEALTH.md)
*   [配置快照](reference/SQL_grammar/monitoring/$SNAPSHOT_CONFIGS.md)
*   [序列快照](reference/SQL_grammar/monitoring/$SNAPSHOT_SEQUENCES.md)


##列表视图##

[列表](reference/SQL_grammar/monitoring/list.md) 是一种轻量级的得到系统当前状态的命令，主要分为以下类型：

*   [上下文列表](reference/SQL_grammar/monitoring/$LIST_CONTEXT.md)
*   [当前会话上下文列表](reference/SQL_grammar/monitoring/$LIST_CONTEXT_CUR.md)
*   [会话列表](reference/SQL_grammar/monitoring/$LIST_SESSION.md)
*   [当前会话列表](reference/SQL_grammar/monitoring/$LIST_SESSION_CUR.md)
*   [集合列表](reference/SQL_grammar/monitoring/$LIST_CL.md)
*   [集合空间列表](reference/SQL_grammar/monitoring/$LIST_CS.md)
*   [存储单元列表](reference/SQL_grammar/monitoring/$LIST_SU.md)
*   [分区组列表](reference/SQL_grammar/monitoring/$LIST_GROUP.md)
*   [事务列表](reference/SQL_grammar/monitoring/$LIST_TRANS.md)
*   [当前事务列表](reference/SQL_grammar/monitoring/$LIST_TRANS_CUR.md)
*   [序列列表](reference/SQL_grammar/monitoring/$LIST_SEQUENCES.md)

> **Note:**
>
> 单位说明：
>
> - 描述中没有标志单位时：
>   1. 监控信息中关于存储的单位为字节。
>   2. 监控信息中关于数量的单位为条数。
>   3. 监控信息中关于网络的单位为字节。
>
> - 描述中有标志单位时，以描述为准。

##SQL到SequoiaDB映射表##

下表列出了 SQL 快照查询语句的操作在 API 中对应的[快照操作](reference/Sequoiadb_command/Sdb/snapshot.md)：

| SQL 语句 | API 语句        |
| -------- | -------------- |
|  SELECT \<sel\> FROM $\<snapshot\> WHERE \<cond\> ORDER BY \<sort\>  |   db.snapshot( <snapType>, [cond], [sel], [sort] ) |
| db.exec( "select * from $SNAPSHOT_CONTEXT where SessionID = 20" ) | 过滤指定条件的记录。db.snapshot(SDB_SNAP_CONTEXTS, { SessionID: 20 } ) |
| db.exec( " select NodeName from $SNAPSHOT_CONTEXT " ) | 只显示记录的指定字段。db.snapshot(SDB_SNAP_CONTEXTS, {}, { NodeName:""} ) |
| db.exec( " select * from $SNAPSHOT_CONTEXT order by SessionID" ) | 根据指定字段进行排序。db.snapshot(SDB_SNAP_CONTEXTS, {}, {}, { "SessionID": 1 } ) |

下面列出了 SQL 快照查询语句的操作在 API 中使用指定[快照查询参数](reference/Sequoiadb_command/AuxiliaryObjects/SdbSnapshotOption.md)的对应快照操作：

**SELECT \<sel\> FROM $\<snapshot\> WHERE \<cond\> ORDER BY \<sort\> LIMIT \<limit\> OFFSET \<skip\>**

对应

**SdbSnapshotOption[.cond(\<cond\>)]  
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.sel(\<sel\>)]  
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.sort(\<sort\>)]  
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.options(\<options\>)]  
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.skip(\<skipNum\>)]  
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.limit(\<retNum\>)]**

###cond(\<cond\>)###

| SQL 语句 | API 语句        |
| -------- | -------------- |
| db.exec( "select * from $SNAPSHOT_CONTEXT where SessionID = 22" ) | db.snapshot( SDB_SNAP_CONTEXTS, new SdbSnapshotOption().cond( { SessionID: 22 } ) ) |

###sel(\<sel\>)###

| SQL 语句 | API 语句        |
| -------- | -------------- |
| db.exec( "select SessionID from $SNAPSHOT_CONTEXT" ) | db.snapshot( SDB_SNAP_CONTEXTS, new SdbSnapshotOption().cond( {} ).sel( { SessionID: "" } ) ) |

###sort(\<sort\>)###

| SQL 语句 | API 语句        |
| -------- | -------------- |
| db.exec( " select * from $SNAPSHOT_CONTEXT order by SessionID" ) | db.snapshot( SDB_SNAP_CONTEXTS, new SdbSnapshotOption().cond( {} ).sort( { SessionID: 1 } ) ) |

###options(\<options\>)###

| SQL 语句 | API 语句        |
| -------- | -------------- |
| db.exec('select * from $SNAPSHOT_CONFIGS where GroupName = "db1" and SvcName = "20000" /*+use_option(Mode, local) use_option(Expand, false)*/') | db.snapshot( SDB_SNAP_CONFIGS, new SdbSnapshotOption().cond( { GroupName:'db1', SvcName:'20000' } ).options( { "Mode": "local", "Expand": false } ) ) |

###skip(\<skipNum\>)###
| SQL 语句 | API 语句        |
| -------- | -------------- |
| db.exec( " select * from $SNAPSHOT_CONTEXT offset 2" ) | db.snapshot( SDB_SNAP_CONTEXTS, new SdbSnapshotOption().cond( {} ).skip( 2 ) ) |

###limit(\<retNum\>)###
| SQL 语句 | API 语句        |
| -------- | -------------- |
| db.exec( "select * from $SNAPSHOT_CONTEXT limit 1" ) | db.snapshot( SDB_SNAP_CONTEXTS, new SdbSnapshotOption().cond( {} ).limit( 1 ) ) |


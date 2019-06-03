监控是一种监视当前系统状态的方式。在 SequoiaDB 中，用户可以使用快照（SNAPSHOT）与列表（LIST）命令进行系统监控。

##快照视图##

[快照](reference/SQL_grammar/monitoring/snapshot.md) 是一种得到系统当前状态的命令，主要分为以下类型：

*   [上下文快照](reference/SQL_grammar/monitoring/SDB_SNAP_CONTEXTS.md)
*   [当前会话上下文快照](reference/SQL_grammar/monitoring/SDB_SNAP_CONTEXTS_CURRENT.md)
*   [会话快照](reference/SQL_grammar/monitoring/SDB_SNAP_SESSIONS.md)
*   [当前会话快照](reference/SQL_grammar/monitoring/SDB_SNAP_SESSIONS_CURRENT.md)
*   [集合快照](reference/SQL_grammar/monitoring/SDB_SNAP_COLLECTIONS.md)
*   [集合空间快照](reference/SQL_grammar/monitoring/SDB_SNAP_COLLECTIONSPACES.md)
*   [数据库快照](reference/SQL_grammar/monitoring/SDB_SNAP_DATABASE.md)
*   [系统快照](reference/SQL_grammar/monitoring/SDB_SNAP_SYSTEM.md)
*   [编目信息快照](reference/SQL_grammar/monitoring/SDB_SNAP_CATALOG.md)
*   [事务快照](reference/SQL_grammar/monitoring/SDB_SNAP_TRANSACTIONS.md)
*   [当前事务快照](reference/SQL_grammar/monitoring/SDB_SNAP_TRANSACTIONS_CURRENT.md)
*   [访问计划缓存快照](reference/SQL_grammar/monitoring/SDB_SNAP_ACCESSPLANS.md)
*   [节点健康检测快照](reference/SQL_grammar/monitoring/SDB_SNAP_HEALTH.md)
*   [配置快照](reference/SQL_grammar/monitoring/SDB_SNAP_CONFIGS.md)
*   [序列快照](reference/SQL_grammar/monitoring/SDB_SNAP_SEQUENCES.md)


##列表视图##

[列表](reference/SQL_grammar/monitoring/list.md) 是一种轻量级的得到系统当前状态的命令，主要分为以下类型：

*   [上下文列表](reference/SQL_grammar/monitoring/SDB_LIST_CONTEXTS.md)
*   [当前会话上下文列表](reference/SQL_grammar/monitoring/SDB_LIST_CONTEXTS_CURRENT.md)
*   [会话列表](reference/SQL_grammar/monitoring/SDB_LIST_SESSIONS.md)
*   [当前会话列表](reference/SQL_grammar/monitoring/SDB_LIST_SESSIONS_CURRENT.md)
*   [集合列表](reference/SQL_grammar/monitoring/SDB_LIST_COLLECTIONS.md)
*   [集合空间列表](reference/SQL_grammar/monitoring/SDB_LIST_COLLECTIONSPACES.md)
*   [存储单元列表](reference/SQL_grammar/monitoring/SDB_LIST_STORAGEUNITS.md)
*   [分区组列表](reference/SQL_grammar/monitoring/SDB_LIST_GROUPS.md)
*   [事务列表](reference/SQL_grammar/monitoring/SDB_LIST_TRANSACTIONS.md)
*   [当前事务列表](reference/SQL_grammar/monitoring/SDB_LIST_TRANSACTIONS_CURRENT.md)
*   [序列列表](reference/SQL_grammar/monitoring/SDB_LIST_SEQUENCES.md)

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
| db.exec( "select * from $SNAPSHOT_CONTEXT where SessionID = 20" ) | 过滤指定条件的记录。db.snapshot(SDB_SNAP_CONTEXTS, { SessionID: 20 } ) |
| db.exec( " select NodeName from $SNAPSHOT_CONTEXT " ) | 只显示记录的指定字段。db.snapshot(SDB_SNAP_CONTEXTS, {}, { NodeName:""} ) |
| db.exec( " select * from $SNAPSHOT_CONTEXT order by SessionID" ) | 根据指定字段进行排序。db.snapshot(SDB_SNAP_CONTEXTS, {}, {}, { "SessionID": 1 } ) |

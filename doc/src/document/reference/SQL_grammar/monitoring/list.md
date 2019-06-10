在 SequoiaDB 中，列表是一种轻量级的得到系统当前状态的命令，主要分为以下类型：

| 列表标示 | 对应 sdbshell 接口标示 | 列表类型 | 描述 |
| -------- | -------- | -------- | ---- |
| [$LIST_CONTEXT](reference/SQL_grammar/monitoring/$LIST_CONTEXT.md) | [SDB_LIST_CONTEXTS](database_management/monitoring/list/SDB_LIST_CONTEXTS.md) | 上下文列表 | 上下文列表列出当前数据库节点中所有的会话所对应的上下文 |
| [$LIST_CONTEXT_CUR](reference/SQL_grammar/monitoring/$LIST_CONTEXT_CUR.md) | [SDB_LIST_CONTEXTS_CURRENT](database_management/monitoring/list/SDB_LIST_CONTEXTS_CURRENT.md) | 当前会话上下文列表 | 当前上下文列表列出当前数据库节点中当前会话所对应的上下文 |
| [$LIST_SESSION](reference/SQL_grammar/monitoring/$LIST_SESSION.md) | [SDB_LIST_SESSIONS](database_management/monitoring/list/SDB_LIST_SESSIONS.md) | 会话列表 | 会话列表列出当前数据库节点中所有的会话 |
| [$LIST_SESSION_CUR](reference/SQL_grammar/monitoring/$LIST_SESSION_CUR.md) | [SDB_LIST_SESSIONS_CURRENT](database_management/monitoring/list/SDB_LIST_SESSIONS_CURRENT.md) | 当前会话列表 | 当前会话列表列出当前数据库节点中当前的会话 |
| [$LIST_CL](reference/SQL_grammar/monitoring/$LIST_CL.md) | [SDB_LIST_COLLECTIONS](database_management/monitoring/list/SDB_LIST_COLLECTIONS.md) | 集合列表 | 集合列表列出当前数据库节点或集群中所有非临时集合 |
| [$LIST_CS](reference/SQL_grammar/monitoring/$LIST_CS.md) | [SDB_LIST_COLLECTIONSPACES](database_management/monitoring/list/SDB_LIST_COLLECTIONSPACES.md) | 集合空间列表 | 集合空间列表列出当前数据库节点或集群中所有集合空间（编目集合空间除外） |
| [$LIST_SU](reference/SQL_grammar/monitoring/$LIST_SU.md) | [SDB_LIST_STORAGEUNITS](database_management/monitoring/list/SDB_LIST_STORAGEUNITS.md) | 存储单元列表 | 存储单元列表列出当前数据库节点的全部存储单元信息 |
| [$LIST_GROUP](reference/SQL_grammar/monitoring/$LIST_GROUP.md) | [SDB_LIST_GROUPS](database_management/monitoring/list/SDB_LIST_GROUPS.md) | 分区组列表 | 分区组列表列出当前集群中的所有分区信息 |
| [$LIST_TRANS](reference/SQL_grammar/monitoring/$LIST_TRANS.md) | [SDB_LIST_TRANSACTIONS](database_management/monitoring/list/SDB_LIST_TRANSACTIONS.md) | 事务列表 | 事务列表列出数据库中正在进行的事务信息 |
| [$LIST_TRANS_CUR](reference/SQL_grammar/monitoring/$LIST_TRANS_CUR.md) | [SDB_LIST_TRANSACTIONS_CURRENT](database_management/monitoring/list/SDB_LIST_TRANSACTIONS_CURRENT.md) | 当前事务列表 | 当前事务列表列出当前会话正在进行的事务信息 |
| [$LIST_SEQUENCES](reference/SQL_grammar/monitoring/$LIST_SEQUENCES.md) | [SDB_LIST_SEQUENCES](database_management/monitoring/list/SDB_LIST_SEQUENCES.md) | 序列列表 | 序列列表列出当前数据库中所有的序列信息 |


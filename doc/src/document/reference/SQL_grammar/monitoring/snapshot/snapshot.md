在 SequoiaDB 中，快照是一种得到系统当前状态的命令，主要分为以下类型：

| 快照标示 | 快照类型 | 描述 |
| -------- | -------- | ---- |
| [SNAPSHOT_CONTEXT](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_CONTEXTS.md) | 上下文快照 | 上下文快照列出当前数据库节点中所有的会话所对应的上下文 |
| [SNAPSHOT_CONTEXT_CUR](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_CONTEXTS_CURRENT.md) | 当前会话上下文快照 | 当前上下文快照列出当前数据库节点中当前会话所对应的上下文 |
| [SNAPSHOT_SESSION](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_SESSIONS.md) | 会话快照 | 会话快照列出当前数据库节点中所有的会话 |
| [SNAPSHOT_SESSION_CUR](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_SESSIONS_CURRENT.md) | 当前会话快照 | 当前会话快照列出当前数据库节点中当前的会话 |
| [SNAPSHOT_CL](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_COLLECTIONS.md) | 集合快照 | 集合快照列出当前数据库节点或集群中所有非临时集合 |
| [SNAPSHOT_CS](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_COLLECTIONSPACES.md) | 集合空间快照 | 集合空间快照列出当前数据库节点或集群中所有集合空间（编目集合空间除外） |
| [SNAPSHOT_DB](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_DATABASE.md) | 数据库快照 | 数据库快照列出当前数据库节点的数据库监视信息 |
| [SNAPSHOT_SYSTEM](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_SYSTEM.md) | 系统快照 | 系统快照列出当前数据库节点的系统监视信息 |
| [SNAPSHOT_CATA](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_CATALOG.md) | 编目信息快照 | 用于查看编目信息 |
| [SNAPSHOT_TRANS](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_TRANSACTIONS.md) | 事务快照 | 事务快照列出数据库中正在进行的事务信息 |
| [SNAPSHOT_TRANS_CUR](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_TRANSACTIONS_CURRENT.md) | 当前事务快照 | 当前事务快照列出当前会话正在进行的事务信息 |
| [SNAPSHOT_ACCESSPLANS](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_ACCESSPLANS.md) | 访问计划缓存快照 | 访问计划缓存快照列出数据库中缓存的访问计划的信息 |
| [SNAPSHOT_HEALTH](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_HEALTH.md) | 节点健康检测快照 | 节点健康检测快照列出数据库中所有节点的健康信息 |
| [SNAPSHOT_CONFIGS](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_CONFIGS.md) | 配置快照 | 配置快照列出数据库中指定节点的配置信息 |
| [SNAPSHOT_SEQUENCES](reference/SQL_grammar/monitoring/snapshot/SDB_SNAP_SEQUENCES.md) | 序列快照 | 序列快照列出当前数据库的全部序列信息 |


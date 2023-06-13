SequoiaDB 巨杉数据库是一款金融级分布式关系型数据库，产品引擎采用原生分布式架构，100% 兼容 MySQL，支持完整的 ACID 和分布式事务。同时 SequoiaDB 还提供多模（multi-model）数据库存储引擎，原生支持多数据中心容灾机制，是新一代分布式数据库的首选。

本文档中心旨在介绍 SequoiaDB 巨杉数据库的基本概念、数据增删改查的基本语法、数据库运维管理的基本策略，以及性能调优和问题诊断的相关思路。

[快速使用 SequoiaDB][quickstart]

##注意事项##
- 从 3.4.4/3.6/5.0.3 及早期版本滚动升级到 3.4.5/3.6.1/5.0.4 及之后的版本时，从 SQL 引擎执行的 INSERT 操作会存在失败。因此滚动升级的过程中需保证优先完成存储引擎的升级，然后再进行 MySQL/MariaDB 实例的升级。
- 从 3.4.4/3.6/5.0.3 及早期版本升级到 3.4.5/3.6.1/5.0.4 及之后的版本，如果集群会扩展为 X86 和 ARM 架构混合部署，则在升级前版本上创建的、使用 double 类型字段作为 hash 分区键的集合，需要进行重建，否则可能会出现数据无法正确访问的问题。可通过查询 SDB_SNAP_CATALOG 快照，根据集合使用的 hash 算法版本号（InternalV 字段）判断，对于该版本号小于 4 的集合需要进行处理。

##SequoiaDB version 5.6.1 版本说明##

**接口变更：**

- SQL 引擎
  - 新增 sequoiadb_execution_mode 配置参数
  - 新增 information_schema_tables_stats_cache_first 配置参数
  - sdb_sql_ctl 工具端口参数 -p 调整为 -P

**主要特性：**

NA

**性能优化：**

- 存储引擎
  - 优化过滤的记录数超过 CL 总记录数的 10% 时，执行 count 操作的性能

- SQL 引擎
  - 优化从 information_schema.tables 查询统计信息的性能

**工具优化：**

- SQL 引擎
  - 修复某些 auto.cnf 配置下，sdb_sql_ctl 工具添加实例到实例组会报错的问题
  - 实例组在开启 sequoiadb_execute_only_in_mysql 的情况下，SQL 实例的元数据能够同步

**解决重要Bug：**

- 存储引擎
  - 修复指定节点角色为所有节点时，查询回收站快照可能造成节点 crash 的问题
  - 修复执行 split 操作，可能造成节点 crash 的问题
  - 修复 sdbcm 进程创建的子进程可能变成僵尸进程的问题
  - 修复同步日志满，备节点归档日志归档失败的问题
  - 修复回收站项目已满，删除 CS/CL 可能导致报错 -147 的问题

- SQL 引擎
  - 修复 SequoiaDB 回收站已满时不正确的错误处理
  - 修复对同一张表有跨实例的 DDL 与 DQL 并发操作可能导致 crash 的问题
  - 修复异常场景下可能有日志未被实例组回放线程同步的问题
  - 修复存储过程中创建用户使用明文密码，密码内容不正确的问题
  - 修复无事务模式下，执行 INSERT INTO ... SELECT ... 语句可能触发的空指针异常
  - 修复在特定复杂 WHERE 条件下，查询使用 GROUP MIN MAX 访问计划有错误结果的问题
  - 修复有聚集函数时，direct_limit 优化可能错误施加，导致错误结果的问题
  - 修复查询慢查询日志表走 direct_count 优化查询时导致 crash 的问题
  - 修复复合索引首字段匹配条件为 IS NULL 时可能导致 crash 的问题
  - 修复 MySQL 分区表指定分区名并发查询可能结果集缺失的问题
  - 修复若干潜在的变量未初始化、内存非法读取问题

[^_^]:
    本文使用的所有引用及链接
[quickstart]:manual/Quick_Start/quick_deployment.md

SequoiaDB 数据库是一款新型企业级分布式非关系型数据库，帮助企业用户降低 IT 成本，并对大数据的存储与分析提供了一个坚实，可靠，高效与灵活的底层平台。

##SequoiaDB version 2.0 版本说明##

**接口变更：**

- 增加事务锁超时时间配置transactiontimeout
- createIndex接口增加SortBufferSize参数
- createCS接口增加IndexEngineType参数（社区版）
- upsert增加$setOnInsert操作符
- update增加$replace操作符
- 增加findAndRemove和findAndUpdate接口
- 增加createIdIndex和dropIdIndex接口
- 复制组增加对节点的attach及detach接口
- C驱动增加sdbGetCLName 及 sdbGetCLFullName接口

**主要特性：**

- 新版SequoiaDB OM（业务操作管理系统）
- Rocksdb作为可选索引存储引擎（社区版）
- 基于字典的数据压缩算法功能
- 索引重建机制优化
- 支持事务超时机制
- 支持手动触发深度刷盘机制
- 优化数据组节点心跳检测
- 优化sdb_fdw访问性能
- 支持findAndRemove和findAndModify原子操作
- 支持update $replace操作符
- 支持内置SQL使用hint语法选择指定索引
- 支持selector的数学运算，字符串和cast操作
- 支持手工创建删除ID索引功能
- 增加SYSSpare组，用于管理后备节点


**工具优化：**

- 新版高性能多并发导入工具，增加类型autodate和autotimestamp，增加导入选项allowkeydup
- C驱动支持AIX平台

**注意事项：**

- 社区版要求系统安装glibc 2.15以及libstdc++ 6.0.18以上版本

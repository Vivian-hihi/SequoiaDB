## 自定义表配置

在 MySQL 上创建表时，可以在其表选项 COMMENT 中通过指定关键词 "sequoiadb" ，并紧跟一 json 对象以传入自定义的表配置参数。格式如下：

```
COMMENT [=] "[string,] sequoiadb:{ table_options:{...}[, use_partition:<true|false>] }"
```

具体配置参数如下表:

| 参数名 | 类型 | 描述 | 是否必填 |
| ------ | --- | ------ | ------ |
| string | string |用户自定义注释字符串 | 否 |
| table_options | json | 创建集合的相关参数。详见[SequoiaDB创建集合选项](reference/Sequoiadb_command/SdbCS/createCL.md)。| 否 |
| use_partition | bool | 是否创建分区表。取值 false 则显式创建非分区表。| 否 |

示例1：在 SequoiaDB 上创建根据时间进行范围切分的表。

```lang-sql
mysql> CREATE TABLE business_log(ts TIMESTAMP, level INT, content TEXT, PRIMARY KEY(ts))
    -> ENGINE=sequoiadb
    -> COMMENT="Sharding table for example, sequoiadb:{ table_options: { ShardingKey: { ts: 1 }, ShardingType: 'range' } }";
```
示例2：在[引擎配置项](sql_engine/sequoiasql_mysql/config.md#引擎配置)`sequoiadb_use_partition`为 ON 时，指定`use_partition`为 false 显式创建普通表。

```lang-sql
mysql> CREATE TABLE employee(id INT PRIMARY KEY, name VARCHAR(128) UNIQUE KEY)
    -> ENGINE=sequoiadb 
    -> COMMENT='sequoiadb:{ use_partition: false }';
```


## 引擎配置
+ **配置项列表**

   以下表格列出了所有的 SequoiaDB 存储引擎配置项，及它们的简要描述。详细信息参考后文[引擎配置使用说明](sql_engine/sequoiasql_mysql/config.md#引擎配置使用说明)。

   |参数名|类型|默认值|动态生效|作用范围|说明|
   |---|---|---|---|---|---|
   |sequoiadb_conn_addr|string|"localhost:11810"|Yes|Global|SequoiaDB 连接地址。|
   |sequoiadb_user|string|""|Yes|Global|SequoiaDB 鉴权用户。|
   |sequoiadb_password|string|""|Yes|Global|SequoiaDB 鉴权密码。|
   |sequoiadb_use_partition|bool|ON|Yes|Global|是否启用自动分区。|
   |sequoiadb_use_bulk_insert|bool|ON|Yes|Global|是否启用批量插入。|
   |sequoiadb_bulk_insert_size|int|100|Yes|Global|批量插入时每批的插入记录数。|
   |sequoiadb_replica_size|int|1|Yes|Global|写操作需同步的副本数。取值范围为[-1, 7]。|
   |sequoiadb_use_autocommit|bool|ON|Yes|Global|是否启用自动提交模式(已弃用)。|
   |sequoiadb_debug_log|bool|OFF|Yes|Global|是否打印debug日志。|
   |sequoiadb_optimizer_select_count|bool|ON|Yes|Global|是否开启优化select count(*)行为。|
   |sequoiadb_alter_table_overhead_threshold|long|10000000|Yes|Global, Session|更改表开销阈值。当表记录数超过这个阈值，需要全表更新的更改操作将被禁止。|
   |sequoiadb_selector_pushdown_threshold|unsigned int|30|Yes|Global, Session|查询字段下压触发阈值，取值范围[0, 100]，单位：%。|
   |sequoiadb_execute_only_in_mysql|bool|OFF|Yes|Global, Session|DDL 命令只在 MySQL 执行，不下压到 SequoiaDB 执行。|

+ **配置修改方式**

    配置参数有三种修改方式。

   + 使用工具sdb_sql_ctl修改配置

   ```lang-bash
   $ bin/sdb_sql_ctl chconf myinst --sdb-use-partition=OFF
   ```

   + 修改实例数据目录下的配置文件auto.cnf，在[mysqld]下添加/更改对应配置项。示例：

   ```lang-ini
   sequoiadb_use_partition=OFF
   ```

   > **Note:** 修改配置文件后需要重新启动MySQL服务

   + 通过MySQL命令行修改，示例：

   ```lang-sql
   mysql> SET GLOBAL sequoiadb_use_partition=OFF;
   ```

   > **Note:** 通过命令行方式进行的配置为临时有效，当重启MySQL服务后配置将失效。如果需要配置永久生效则必须通过修改配置文件。  

## 引擎配置使用说明

+ **配置 SequoiaDB 连接与鉴权**

   `sequoiadb_conn_addr`可以配置 MySQL 实例所连接的 SequoiaDB 存储集群。可以配置一个或多个协调节点的地址。使用多个时，地址之间要以逗号隔开。如“sdbserver1:11810,sdbserver2:11810”。在配置多个地址时，每次连接会从地址中随机随机选择。在 MySQL 会话数很多时，压力会基本平均地分摊给每个协调节点。
   
   `sequoiadb_user`和`sequoiadb_password`则需设置为所连接的 SequoiaDB 集群的鉴权用户和密码。
   以上的配置在命令行修改后，均在建立新连接时才生效，不影响旧连接。

+ **配置自动分区功能**

   `sequoiadb_use_partition`配置项决定 MySQL 是否使用自动分区功能。自动分区可以普遍提升 SequoiaDB 的性能。自动分区默认启动，启动时，在 MySQL 上创建表将同步在 SequoiaDB 上创建对应的分区表（hash分区，包含所有分区组）。自动分区时，分区键按顺序优先使用主键字段和唯一索引字段。如果两者都没有，则不做分区。
   
   如果开启自动分区后，部分表不希望被分区，可以在[自定义表配置](sql_engine/sequoiasql_mysql/config.md#自定义表配置)中指定`use_partition`为 false。

   > **Note:** 自动分区时，主键或唯一索引只在建表时对应分区键。建表后添加删除主键或唯一索引都不会更改分区键。

+ **配置默认副本数**

   `sequoiadb_replica_size`配置项可以设置表默认的写操作需同步的副本数。副本数多时，数据一致性强度高，但性能会有所下降。副本数少时，则反之。具体可参考 SequoiaDB 的[创建集合的ReplSize参数](reference/Sequoiadb_command/SdbCS/createCL.md#参数)。

+ **配置批量插入**

   `sequoiadb_use_bulk_insert`配置项决定是否开启批量插入功能。批量插入可以提升 SequoiaDB 存储引擎的插入性能。在关闭功能时，MySQL 的批量插入在 SequoiaDB 中是逐条的插入。而开启时，SequoiaDB 存储引擎会把 MySQL 的 1 个批次分解成若干个 sequoiadb_bulk_insert_size 大小的批次进行插入。例如，MySQL 批量插入 1024 条记录，在 sequoiadb_bulk_insert_size 为 100 时，SequoiaDB 存储引擎会进行 10 次记录数为 100 的批量插入，和 1 次记录数为 24 的批量插入。
   
   `sequoiadb_bulk_insert_size`配置项可以配置 SequoiaDB 每次进行批量插入的记录数。在进行插入性能的调优时，可以根据实际适当调整这个值。

+ **性能优化配置**

   `sequoiadb_selector_pushdown_threshold`可以配置查询字段下压的触发阈值。查询字段不下压时，SequoiaDB 集群总是返回完整记录给 MySQL，由 MySQL 过滤有用字段。而在查询字段下压时，SequoiaDB 集群只返回 MySQL 所需字段。在查询字段个数/表总字段个数的百分比小于等于该阈值时，查询字段下压，否则不下压。下压查询字段可以节省了网络传输，但它也会增加 SequoiaDB 工作。可以根据实际适当调整。

   `sequoiadb_optimizer_select_count`决定是否开启优化 SELECT COUNT(*) 行为。未优化时，SELECT COUNT(*) 会请求 SequoiaDB 返回表中的所有记录，由 MySQL 进行计数。开启优化时，SELECT COUNT(*) 会对接到 SequoiaDB 的[SdbCollection.count()](reference/Sequoiadb_command/SdbCollection/count.md)方法，由 SequoiaDB 进行计数。

+ **其它配置**

   `sequoiadb_alter_table_overhead_threshold`配置是更改表开销阈值。当表记录数超过这个阈值，需要全表更新的更改操作将被禁止。这个限制是防止对大表误进行了更改操作。大表的更新可能花费较多的时间。该阈值对添加 DEFAULT NULL 的列、数据类型扩容等无需更新的轻量操作不生效。如确认要对大表结构进行更改，在线上调阈值后，重新执行更改操作即可。
       
   `sequoiadb_execute_only_in_mysql`配置开启后，DDL 语句只在 MySQL 侧执行，即只更改 MySQL 侧表元数据信息，而不会下压到 SDB 侧同步表 DDL 操作。
   
   `sequoiadb_debug_log`配置开启后，MySQL 日志会打印 SequoiaDB 存储引擎有关 debug 信息。
   
   `sequoiadb_use_autocommit`配置项已弃用。请直接使用 MySQL 的`autocommit`配置项。

## MySQL 常用系统配置

| 参数名                 | 类型   | 动态生效 | 动态范围   | 默认值  | 说明 |
| ---------------------- | ----   | -------- | ---------- | ------- | ---- |
| max_connections        | int    | Yes | Global          | 151     | 客户端最大连接数 |
| sql_mode               | set    | Yes | Global, Session | STRICT_TRANS_TABLES,<br>ERROR_FOR_DIVISION_BY_ZERO,<br>NO_AUTO_CREATE_USER,<br>NO_ENGINE_SUBSTITUTION | SQL 模式。取值意义参考[MySQL SQL 模式](https://dev.mysql.com/doc/refman/5.7/en/sql-mode.html) |
| character_set_server   | string | Yes | Global, Session | utf8mb4 | 默认字符集 |
| collation_server       | string | Yes | Global, Session | utf8mb4_bin | 默认校对集 |
| default_storage_engine | string | Yes | Global, Session | SequoiaDB | 默认存储引擎 |
| lower_case_table_names | int    | No  | Global          | 1       | 表名大小写策略。取 0 时，大小写敏感。取 1 时，所有表名均以小写存储。取 2 时，表名以原样存储，但以小写进行比较。 |

> **Note:** 
>
> * 在系统最大文件句柄数不足时，max_connections 可能被自动调整。如果发现修改该配置没有生效，请检查系统 limit 设置和 MySQL 日志。
> * SequoiaDB 不支持大小写敏感的校对集。


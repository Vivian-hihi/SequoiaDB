以下操作均在 MySQL shell 环境下执行。

##连接MySQL实例与数据库分布式存储引擎##

+ **为 SequoiaDB 开启事务**

   修改配置项 transactionon 为 true，然后重启节点。具体配置方式参考[数据库配置](database_management/runtime_configuration.md#参数配置)。

+ **登录 MySQL shell**

 ```lang-bash
 $ export MYSQL_HOME=/opt/sequoiasql/mysql
 $ ${MYSQL_HOME}/bin/mysql --defaults-file=${MYSQL_HOME}/database/3306/auto.cnf -S ${MYSQL_HOME}/database/3306/mysqld.sock -u root -p
 ```

+ **配置 SequoiaDB 连接地址**

   默认的 SequoiaDB 连接地址为“localhost:11810”，如需修改可参考以下两种方式
   
   (1)通过bin/sdb_sql_ctl指定实例名修改

 ```lang-bash
    # bin/sdb_sql_ctl chconf myinst --sdb-conn-addr=192.168.20.37:11810,192.168.20.38:11810
 ```

   (2)通过配置文件修改，参考[配置说明](sql_engine/sequoiasql_mysql/connection.md#配置说明)。

+ **创建数据库实例**

 ```lang-sql
 mysql> create database cs;
 mysql> use cs;
 ```

+ **创建表**

 ```lang-sql
 mysql> create table cl(a int, b int, c text, primary key(a, b) ) ;
 mysql> create table cl1(a int, b int, unique index idx_a(a) ) ;
 ```

+ **基本数据操作**

 ```lang-sql
 mysql> insert into cl values(1, 101, "SequoiaDB test");
 mysql> insert into cl values(2, 102, "SequoiaDB test");
 mysql> insert into cl1 values(1, 99);
 mysql> select * from cl order by b asc limit 1;
 mysql> select * from cl, cl1 where cl.a=cl1.a;
 mysql> update cl set c="My test" where a=1;
 mysql> delete from cl where b=102;
 ```

+ **创建索引**

   使用非索引字段（"c"）执行查询时，访问计划信息如下：

 ```lang-sql
 mysql> explain select * from cl where c="test";
 +----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+---------------------------------------------------------------+
 | id | select_type | table | partitions | type | possible_keys | key  | key_len | ref  | rows | filtered | Extra                                                         |
 +----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+---------------------------------------------------------------+
 |  1 | SIMPLE      | cl    | NULL       | ALL  | NULL          | NULL | NULL    | NULL |    2 |    50.00 | Using where with pushed condition (`cs`.`cl`.`c` = 'test') |
 +----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+---------------------------------------------------------------+
 1 row in set, 1 warning (0.00 sec)

 ```

   对“c”字段创建索引并执行相同查询时将会使用该索引，具体访问计划信息如下：

 ```lang-sql
 mysql> alter table cl add index idx_c(c(20));
 Query OK, 0 rows affected (0.03 sec)
 Records: 0  Duplicates: 0  Warnings: 0

 mysql> explain select * from cl where c="test";
 +----+-------------+-------+------------+------+---------------+-------+---------+-------+------+----------+---------------------------------------------------------------+
 | id | select_type | table | partitions | type | possible_keys | key   | key_len | ref   | rows | filtered | Extra                                                         |
 +----+-------------+-------+------------+------+---------------+-------+---------+-------+------+----------+---------------------------------------------------------------+
 |  1 | SIMPLE      | cl    | NULL       | ref  | idx_c         | idx_c | 23      | const |    1 |   100.00 | Using where with pushed condition (`cs`.`cl`.`c` = 'test') |
 +----+-------------+-------+------------+------+---------------+-------+---------+-------+------+----------+---------------------------------------------------------------+
 1 row in set, 1 warning (0.01 sec)

 ```

+ **存储过程**

 ```lang-sql
 mysql> delimiter //
 mysql> create procedure delete_match()
     -> begin
     -> delete from cl where a=1;
     -> end//
 mysql> delimiter ;
 mysql> call delete_match();
 ```

+ **视图**

 ```lang-sql
 mysql> create view
     -> v1
     -> as select
     -> cl.a,cl.c,cl1.b
     -> from
     -> cl,cl1
     -> where cl.a=cl1.a;
 mysql> select * from v1;
 ```

##配置说明##

###引擎配置###

|参数名|类型|默认值|支持命令行修改|说明|
|------|----|------|----------|----|
|sequoiadb_conn_addr          |字符串|"localhost:11810" |true |SequoiaDB连接地址，可配置多个，之间用逗号隔开。|
|sequoiadb_user               |字符串|""     |true |SequoiaDB鉴权用户。|
|sequoiadb_password           |字符串|""     |true |SequoiaDB鉴权密码。|
|sequoiadb_use_partition      |布尔  |ON     |true |是否启用自动分区。|
|sequoiadb_use_bulk_insert    |布尔  |ON     |true |是否启用批量插入。|
|sequoiadb_bulk_insert_size   |整数  |100    |true |批量插入时每批的插入记录数。|
|sequoiadb_replica_size       |整数  |-1     |true |写操作需同步的副本数。在建表时并且没有自定义表配置时使用。取值范围为[-1, 7]。具体可参考SequoiaDB的[创建集合的ReplSize参数](reference/Sequoiadb_command/SdbCS/createCL.md#参数)。|
|sequoiadb_use_autocommit     |布尔  |ON     |true |是否启用自动提交模式。|
|sequoiadb_debug_log          |布尔  |OFF    |true |是否打印debug日志。|

> **Note:**
> 
> * 配置多个SequoiaDB连接地址时，每次连接会从地址中随机选择。
> * 命令行中修改连接有关的配置，在建立新连接时才生效，不影响旧连接。有关配置包括sequoiadb_conn_addr, sequoiadb_user, sequoiadb_password。
> * 自动分区默认启动，启动时，在mysql上创建表将同步在SequoiaDB上创建对应的分区表（hash分区，包含所有分区组）。
> * 自动分区时，分区键按顺序优先使用主键字段和唯一索引字段。如果两者都没有，则不做分区。
> * 自动分区时，主键或唯一索引只在建表时对应分区键。建表后添加删除主键或唯一索引都不会更改分区键。

配置参数有三种修改方式。

   (1)使用工具sdb_sql_ctl修改配置

 ```lang-bash
 $ bin/sdb_sql_ctl chconf myinst --sdb-use-partition=OFF
 ```

   (2)修改实例数据目录下的配置文件auto.cnf，在[mysqld]下添加/更改对应配置项。示例：

 ```lang-ini
 sequoiadb_use_partition=OFF
 ```

> **Note:**修改配置文件后需要重新启动MySQL服务

   (3)通过MySQL命令行修改，示例：

 ```lang-sql
 mysql> SET GLOBAL sequoiadb_use_partition=OFF;
 ```

> **Note:**通过命令行方式进行的配置为临时有效，当重启MySQL服务后配置将失效。如果需要配置永久生效则必须通过修改配置文件。  

###自定义表配置###

   在mysql上创建表时，可以在 table_option 的 comment 中通过指定关键词 "sequoiadb" ，并紧跟一 json 对象以传入自定义的表配置参数。

```
table_option:
comment [=] "sequoiadb:{table_options:{...}}"
```
具体配置参数如下表:

| 参数名 | 类型 | 描述 | 是否必填 |
| ------ | --- | ------ | ------ |
| table_options | json | 创建集合的相关参数。详见[SequoiaDB创建集合选项](reference/Sequoiadb_command/SdbCS/createCL.md)。| 否	|
| use_partition | bool| 是否创建分区表, 取值 false 即创建非分区表，并过滤 table_options 中与分区相关的参数 | 否 |

 示例1：  
 在SequoiaDB上创建分区键为“{a:1,b:-1}”，分区类型为范围分区的集合cl1  

 ```lang-sql
 mysql> create table cl1(a int, b int, c text) engine = SequoiaDB comment="sequoiadb:{table_options:{ShardingKey:{a:1,b:-1},ShardingType:\"range\"}}";
 ```
示例2：  
使用 use_partition 显示指定创建非分区表时会过滤掉 table_options 中与分区相关的参数并且创建非分区表。

```lang-sql
 mysql> create table cl2(a int, b int, primary key(a), unique key(b)) engine=sequoiadb comment='sequoiadb:{table_options:{"ShardingKey":{a:1}, ShardingType:"hash", Compressed:true, CompressionType:"lzw", AutoSplit: true}, use_partition:false} ';
```

示例3：  
单独指定 "use_partition:false" 时，在配置项 sequoiadb_use_partition = ON 的情况下，也都只会创建普通非分区表。

```lang-sql
 mysql> create table cl3(a int, b int, primary key(a), unique key(b)) engine=sequoiadb comment='sequoiadb:{use_partition:false}';
```

##在线修改 DDL##

MySQL实例组件支持大多数 DDL 的在线修改。在线修改支持原表中（INPLACE）修改表属性，并且允许并发的 DML。可以通过 ALGORITHM 参数控制 ALTER TABLE 语句修改 DDL 时使用的算法。当 ALGORITHM = INPLACE 时，可以在线地修改表属性，而 ALGORITHM = COPY 时，则会把原表内容拷贝到新表，性能会下降。不指定 ALGORITHM 时会自动选择算法。如需在线修改 DDL，一般建议显式地指定 ALGORITHM = INPLACE。例子：

 ```lang-sql
 mysql> alter table cl add index id_idx(id) algorithm=inplace;
 ```

###建表选项###

可以在线修改字符集（CHARACTER SET）和重命名（RENAME）。但不能修改备注（COMMENT），因为它可能包含了建表时的自定义表配置。

###主键与索引###

支持在线添加删除主键和（唯一）索引。

##数据类型映射关系##

| MySQL 实例 | JSON 对象    | 备注                                         |
| ---------- | ------------ | -------------------------------------------- |
| BIT        | int/long     | 超出int范围则按long存储                      |
| BOOL       | int          |                                              |
| TINYINT    | int          |                                              |
| SMALLINT   | int          |                                              |
| MEDIUMINT  | int          |                                              |
| INT        | int/long     | 超出int范围则按long存储                      |
| BIGINT     | long/decimal | 超出long范围则按DECIMAL存储                  |
| FLOAT      | double       |                                              |
| DOUBLE     | double       |                                              |
| DECIMAL    | decimal      |                                              |
| YEAR       | int          |                                              |
| DATE       | date         |                                              |
| TIME       | double       | 'HHMMSS[.fraction]'格式的double值            |
| DATETIME   | string       | 'YYYY-MM-DD HH:MM:SS[.fraction]'格式的字符串 |
| TIMESTAMP  | timestamp    |                                              |
| CHAR       | string       |                                              |
| VARCHAR    | string       |                                              |
| BINARY     | binary       |                                              |
| VARBINARY  | binary       |                                              |
| TINYBLOB   | binary       |                                              |
| BLOB       | binary       |                                              |
| MEDIUMBLOB | binary       |                                              |
| LONGBLOB   | binary       | 最大长度16MB                                 |
| TINYTEXT   | string       |                                              |
| TEXT       | string       |                                              |
| MEDIUMTEXT | string       |                                              |
| LONGTEXT   | string       | 最大长度16MB                                 |
| ENUM       | int          |                                              |
| SET        | int          |                                              |
| JSON       | binary       |                                              |
| GEOMETRY   | 不支持       |                                              |
| NULL       | -            | 不存储                                       |

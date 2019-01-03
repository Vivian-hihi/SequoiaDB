##SequoiaSQL-MySQL使用注意事项##

1. 不支持设置自增字段；

2. 不支持创建外键；

3. 时间戳类型字段取值范围为：1902-01-01 00:00:00.000000 至 2037-12-31 23:59:59.999999；

4. 通过MySQL创建的索引，不可直接在SequoiaDB上对索引执行删除或修改操作；

5. 复合唯一索引仅支持所有字段null值重复，不允许部分字段null值重复，例如：允许出现(null,null)和(null,null)重复值，但不允许出现(1,null)和(1,null)重复值；

6. 建表时默认创建分区表。如在分区表建立新的唯一索引，必须包含其分区键（分区键依次优先选择主键，唯一键及第一个字段）。如需更改创建分区表配置见[配置说明](sql_engine/sequoiasql_mysql/connection.md#配置说明)；

7. 不支持在BINARY、VARBINARY、TINYBLOB、BLOB、MEDIUMBLOB、LONGBLOB、JSON类型的字段上创建索引；

8. 一个MySQL节点仅可与一个SequoiaDB集群对接，不支持跨多个SequoiaDB集群；

9. 默认字符集为utf8mb4，不支持忽略大小写的字符比较规则，字符比较对大小写敏感；

10. VARCHAR、TEXT查询比较时不会忽略尾部空格。

11. DDL操作不支持事务。

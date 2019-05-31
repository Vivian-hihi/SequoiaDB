使用hint显式地控制执行方式或者执行计划。


##语法##
___/*+hint1 hint2 ...*/___

当希望控制某个语句时，只需要在这个语句结尾处增加 hint 即可。属于同一个语句的 hint 使用空格分隔。

hint有多种不同类型：

| hint类型    | 描述                       |
| ----------- | -------------------------- |
| use_hash    | 指定关联方式为哈希关联。   |
| use_index   | 指定集合的扫描方式。       |
| use_flag    | 设置标志位。               |

##类型描述##

###use_hash###

* 语法

  *use_hash()*

* 参数描述

  无

* 支持语句

  select语句

###use_index###

* 语法

  *use_index( \<index_name\> )*

  *use_index( \<cl_name, index_name\> )*

* 参数描述

  | 参数名      | 参数类型 | 描述                                    | 是否必填 |
  | ----------- | -------- | --------------------------------------- | -------- |
  | index_name  | string   | 索引名。                                | 是       |
  | cl_name     | string   | 集合名。                                | 是       |

* 支持语句

  select语句

###use_flag###

* 语法

  *use_flag( \<flag_name\> )*

* 参数描述

  | 参数名    | 参数类型         | 描述                            | 是否必填 |
  | --------- | ---------------- | ------------------------------- | -------- |
  | flag_name | string \| number | 标志位。不同语句的flag取值不同。| 是       |

* 支持语句

  | 语句   | flag取值                                 | 描述                       |
  | ------ | ---------------------------------------- | -------------------------- |
  | update | SQL_UPDATE_KEEP_SHARDINGKEY (0x00008000) | 更新条件中保留分区键字段。 |


##返回值##

无

##示例##

   * 数据库中集合 foo.bar1, foo.bar2, foo.bar3 的情况如下。

   ```lang-json
   // foo.bar1包含索引 "idx_bar1_a"，该索引以 "a" 字段升序排序，其记录如下：
   { "a": 0 }
   { "a": 1 }
   { "a": 2 }
   { "a": 3 }
   { "a": 4 }

   // foo.bar2包含索引 "idx_bar2_b"，该索引以 "b" 字段升序排序，其记录如下：
   { "a": 1, "b": 1 }
   { "a": 2, "b": 1 }
   { "a": 3, "b": 2 }
   { "a": 4, "b": 2 }
   { "a": 5, "b": 2 }

   // foo.bar1包含索引 "idx_bar3_c"，该索引以 "c" 字段升序排序，其记录如下：
   // foo.bar3
   { "c": 0 }
   { "c": 1 }
   { "c": 2 }
   { "c": 3 }
   { "c": 4 }
   ```

###use_hash()###

   * 指定关联方式为哈希关联。

   ```lang-javascript
   > db.exec("select t1.a, t2.b from foo.bar1 as t1 inner join foo.bar2 as t2 on t1.a = t2.b /*+use_hash()*/")
   { "a": 1, "b": 1 }
   { "a": 1, "b": 1 }
   { "a": 2, "b": 2 }
   { "a": 2, "b": 2 }
   { "a": 2, "b": 2 }
   Return 5 row(s).
   Takes 0.16533s.
   ```

###use_index()###

   * 指定集合的扫描方式

   * 使用集合 foo.bar1 中索引 "idx_bar1_a" 进行扫描。

   ```lang-javascript
   > db.exec("select * from foo.bar1 where a = 1 /*+use_index(idx_bar1_a)*/")
   { "_id": { "$oid": "582ae8ea790ce72860000023" }, "a": 1 }
   Return 1 row(s).
   Takes 0.4843s.
   ```

   * 在关联中指定集合 foo.bar1 使用索引 "idx_bar1_a" 进行扫描。

   ```lang-javascript
   > db.exec("select t1.a, t2.b from foo.bar1 as t1 inner join foo.bar2 as t2 on t1.a = t2.b /*+use_index(t1, idx_bar1_a)*/")
   { "a": 1, "b": 1 }
   { "a": 1, "b": 1 }
   { "a": 2, "b": 2 }
   { "a": 2, "b": 2 }
   { "a": 2, "b": 2 }
   Return 5 row(s).
   Takes 0.13962s.
   ```

   * 在一个 select 语句中使用多个 hint。

   ```lang-javascript
   > db.exec("select t1.a, t2.b from foo.bar1 as t1 inner join foo.bar2 as t2 on t1.a = t2.b /*+use_index(t1, idx_bar1_a) use_index(t2, idx_bar2_b) use_hash()*/")
   { "a": 1, "b": 1 }
   { "a": 1, "b": 1 }
   { "a": 2, "b": 2 }
   { "a": 2, "b": 2 }
   { "a": 2, "b": 2 }
   Return 5 row(s).
   Takes 0.18535s.
   ```

   * 指定集合不使用索引。

   ```lang-javascript
   > db.exec("select * from foo.bar1 where a = 1 /*+use_index(NULL)*/")
   { "_id": { "$oid": "582ae8ea790ce72860000023" }, "a": 1 }
   Return 1 row(s).
   Takes 0.4843s.
   ```

###use_flag()###

   * 指定更新记录时保留分区键。

   * 存在切分表foo.bar，落在两个分区组上，分区键为 { b: 1 }

   ```lang-javascript
   > db.execUpdate( "update foo.bar set b = 1 where age < 25 /*+use_flag(SQL_UPDATE_KEEP_SHARDINGKEY)*/" )
   (nofile):0 uncaught exception: -178
   Sharding key cannot be updated
   Takes 0.002696s.
   > db.execUpdate( "update foo.bar set b = 1 where age < 25 /*+use_flag(0x00008000)*/" )
   (nofile):0 uncaught exception: -178
   Sharding key cannot be updated
   Takes 0.002596s.
   ```
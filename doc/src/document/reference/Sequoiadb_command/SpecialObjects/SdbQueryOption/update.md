##语法##
***query.update( \<rule\>, [returnNew], [options] )***

更新查询后的结果集。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | -------- | ---- | -------- |
| rule    | Json 对象 | 更新规则，记录按指定规则更新。 | 是 |
| returnNew | bool      | 是否返回更新后的记录。         | 否 |
| options | Json 对象 | 可选项，详见options选项说明。| 否 |

##options选项##

| 参数名          | 参数类型 | 描述                | 默认值 |
| --------------- | -------- | ------------------- | ------ |
| KeepShardingKey | bool     | 是否保留分区键字段。| false  |

> **Note:**  
> query.update()方法的定义格式包含 rule 参数、 returnNew 参数 options 参数。其中 rule 参数与 [update()](reference/Sequoiadb_command/SdbCollection/update.md)的 rule 参数相同，options 参数与 [update()](reference/Sequoiadb_command/SdbCollection/update.md)的 options 参数相同。returnNew 参数默认为 false，当为 true 时，返回修改后的记录值。

##返回值##

返回 query 自身，类型为 SdbQueryOption。returnNew为false，则查询时返回更新前的查询结果集；returnNew为true，则查询时返回更新后的结果集。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

查询集合 bar 下 age 字段值大于10的记录，并将符合条件的记录的 age 字段加1。

```lang-javascript
> var query = new SdbQueryOption().cond( { age: { $gt: 10 } } ).update( { $inc: { age: 1 } } )
> db.foo.bar.find( { query )
```

> **Note:**  
> 1. 不能与 query.remove()同时使用。  
> 2. 与 query.sort()同时使用时，在单个节点上排序必须使用索引。  
> 3. 在集群中与 query.limit()或 query.skip()同时使用时，要保证查询条件会在单个节点或单个子表上执行。

##语法##
***query.remove()***

删除查询后的结果集。

##返回值##

返回 query 自身，类型为 SdbQueryOption。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

查询集合 bar 下 age 字段值大于10的记录，并将符合条件的记录删除。

```lang-javascript
> var query = new SdbQueryOption().cond( { age: { $gt: 10 } } ).remove()
> db.foo.bar.find( query )
```

> **Note：**  
> 1. 不能与 query.update() 同时使用。  
> 2. 与 query.sort() 同时使用时，在单个节点上排序必须使用索引。  
> 3. 在集群中与 query.limit() 或 query.skip() 同时使用时，要保证查询条件会在单个节点或单个子表上执行。

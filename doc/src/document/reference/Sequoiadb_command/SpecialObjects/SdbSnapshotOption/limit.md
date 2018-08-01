##语法##
***option.limit( \<num\> )***

控制查询返回的记录条数。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | -------- | ---- | -------- |
| num |	int | 自定义返回结果集的记录条数。 | 否 |

> **Note：**  
> option.limit() 方法的定义格式包含 num 参数，它是 int 类型。如果不设定 num 的内容，相当于返回所有的结果集记录。如果想返回结果集的前5条记录，可是设置 num 的值为5。

##返回值##

返回 option 自身，类型为 SdbSnapshotOption。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

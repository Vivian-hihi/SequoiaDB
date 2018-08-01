##语法##
***query.cond( \<cond\> )***

记录匹配条件。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | -------- | ---- | -------- |
|cond|	Json 对象 | 记录匹配条件。为空时，查询所有记录；不为空时，查询符合条件记录。如：{"age":{"$gt":30}}。关于匹配条件的使用，可参考[匹配符](reference/operator/match_operator/overview.md)。 | 否 |


##返回值##

返回 query 自身，类型为 SdbQueryOption。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

1. 查询匹配条件的记录，即设置 cond 参数的内容。如下操作返回集合 bar 中符合
   条件 age 字段值大于25且 name 字段值为"Tom"的记录。

	```lang-javascript
    > var query = new SdbQueryOption().cond( { age: { $gt: 25 }, name: "Tom" } )
 	> db.foo.bar.find( query )
 	```

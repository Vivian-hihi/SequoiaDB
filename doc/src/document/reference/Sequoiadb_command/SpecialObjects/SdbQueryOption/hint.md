##语法##
***query.hint( \<hint\> )***

指定查询使用索引的情况。

##参数描述##

| 参数名 |  参数类型  | 描述 | 是否必填 |
| ------ | ---------  | ---- | -------- |
| hint   |	Json 对象 | 查询是否使用索引及使用哪个索引将由数据库决定；  | 否 |

> **Note：**  

>	* 不指定`hint`：查询是否使用索引及使用哪个索引将由数据库决定；
>	* `hint`为{"":null}：查询走表扫描；
>	* `hint`为单个索引：如：{"":"myIdx"}，表示查询将使用当前集合中名字为"myIdx"的索引进行；
>	* `hint`为多个索引：如：{"1":"idx1","2":"idx2","3":"idx3"}，
                        表示查询将使用上述三个索引之一进行。
                        具体使用哪一个，由数据库评估决定。


##返回值##

返回 query 自身，类型为 SdbQueryOption。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

1. 使用索引 ageIndex 遍历集合 bar 下存在 age 字段的记录，并返回。

	```lang-javascript
    > var query = new SdbQueryOption().cond( {age: {$exists:1} } ).hint( { "": "ageIndex" } )
	> db.foo.test.find( query )
	{
  		"_id": {
    		"$oid": "5812feb6c842af52b6000007"
  		},
  		"age": 10
	}
	{
  		"_id": {
    		"$oid": "5812feb6c842af52b6000008"
  		},
  		"age": 20
	}
	```
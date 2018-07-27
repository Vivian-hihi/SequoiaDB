##语法##
***query.hint( \<hint\> )***

按指定的索引遍历结果集。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | -------- | ---- | -------- |
| hint | Json 对象 | 指定访问计划，加快查询速度。 | 否 |

> **Note：**  
> 1. hint 参数是一个 Json 对象。数据库不关心该对象的字段名，而是通过其字段值来确认需要使用的索引名称。当字段值为 null 时，表示表扫描。使用 hint 参数的格式为：  ```{ "": null }、 { "": "indexname" }、 { "0": "indexname0", "1": "indexname1", "2": "indexname2" }```。  
> 2. v3.0之前，当使用hint()指定索引时，数据库一旦遍历到能够使用的索引（或者表扫描）时，便会停止遍历，进而转向使用该索引（或表扫描）进行数据查找。  
> 3. v3.0开始，数据库在选择索引时，会基于数据和索引的统计模型进行综合分析，最终会选择一个最恰当的索引使用。所以，从v3.0开始，当使用hint()指定多个索引时，数据库将能够选择最合适当前查询的索引。


##返回值##

返回查询结果集的游标，类型为 object 。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

1. 强制要求查询走表扫描。

	```lang-javascript
	> db.foo.test.find( {age: 100 } ).hint( { "": null } )
	```

2. 使用索引 ageIndex 遍历集合 bar 下存在 age 字段的记录，并返回。

	```lang-javascript
	> db.foo.test.find( {age: {$exists:1} } ).hint( { "": "ageIndex" } )
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
3. 提供若干索引，供数据库选择。数据库将基于数据和索引统计，选择最优的索引使用。

	```lang-javascript
	> db.foo.test.find( {age: 100 } ).hint( { "1": "aIndex", "2": "bIndex", "3":"cIndex" } )
	```
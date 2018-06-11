1. 创建全文索引

	创建全文索引时，只需在字段定义中指定索引类型为 "text" 即可，索引的其它选项对全文索引无效，因此无需指定：

	```lang-javascript
	> db.cs.cl.createIndex('idx', {name:"text", address:"text"})
	```
	
	可指定一个或多个字段，需要注意的是在创建时 text 类型不可与其它任何类型混用。每创建一个全文索引，会在数据节点上对应地创建一个固定集合空间及固定集合（集合与集合空间同名，以 SYS_ 开头，与全文索引的对应关系可通过直连数据节点并使用 listIndexes() 进行查询）。文档在 Elasticsearch 中进行索引时，会使用原始集合中文档的 _id 字段的值生成 Elasticsearch 中文档的 _id，支持的原始文档的 _id 类型包括：

	- 32-bit integer
	- 64-bit integer
	- double
	- string
	- ObjectID
	- boolean
	- date
	- timestamp
	- Object

	全文索引在使用时存在以下约束：

	- 一个集合上最多创建 1 个全文索引
	- 数据库中最多创建 64 个全文索引
	- 不包含任何全文索引字段的文档将不会被索引，也无法被全文检索语法查询到（包括 Elasticsearch 中的 match_all 查询）
	- _id 字段类型不在受支持列表的文档不会被索引，也无法被全文检索语法查询到（包括 Elasticsearch 中的 match_all 查询）
	- 不包含全文检索语法中使用到的任何字段的记录无法被查询到

2. 删除全文索引

	删除全文索引使用 dropIndex 语法，指定索引名即可。
	
	```lang-javascript
	> db.cs.cl.dropIndex('idx')
	```
	在索引被删除时，其对应的固定集合空间也会一并删除。
	
3. 全文检索

	SequoiaDB 适配的全文检索引擎为 Elasticsearch，通过在 SequoiaDB 的查询语法中包含 Elasticsearch 的搜索条件来进行全文检索。基本语法结构为：
	
	```lang-javascript
	> db.cs.cl.find( { "" : { $Text : <search command> } } ).[hint({"":<name>})]
	```

	其中的 search command 即 Elasticsearch 的搜索条件。在存在多个全文索引的情况下，需要通过 hint 指定使用的索引。当只有一个全文索引时无需指定。
	search command 部分支持 Elasticsearch 的 DSL（Domain Specific Language 特定领域语言）语句。因此，需要使用全文检索功能的用户，需要掌握 Elasticsearch 的 DSL 语言。
	以下示例程序在集合中查找 name 中包含 "Smith" 的所有记录：
	
	```lang-javascript
	> db.createCS('megacorp').createCL('employee')
	localhost:11810.megacorp.employee
	Takes 1.246399s.
	> db.megacorp.employee.createIndex('idx_1', {first_name:"text", "last_name":"text", "age":"text", "about":"text", "interests": "text"})
	Takes 1.182447s.
	> db.megacorp.employee.insert({"first_name" : "John","last_name" : "Smith","age" : 25,"about" : "I love to go rock climbing","interests": [ "sports", "music" ]})
	Takes 0.009290s.
	> db.megacorp.employee.insert({"first_name" : "Jane","last_name" : "Smith","age" : 32,"about" : "I like to collect rock albums","interests": [ "music" ]})
	Takes 0.001013s.
	> db.megacorp.employee.insert({"first_name" : "Douglas","last_name" : "Fir","age" : 35,"about": "I like to build cabinets","interests": [ "forestry" ]})
	Takes 0.001004s.
	> db.megacorp.employee.find({"":{"$Text":{"query":{"match":{"about" : "rock climbing"}}}}}).hint({"":"idx_1"})
	{
	  "_id": {
		"$oid": "5a8f8d9c89000a0906000000"
	  },
	  "first_name": "John",
	  "last_name": "Smith",
	  "age": 25,
	  "about": "I love to go rock climbing",
	  "interests": [
		"sports",
		"music"
	  ]
	}
	{
	  "_id": {
		"$oid": "5a8f8d9f89000a0906000001"
	  },
	  "first_name": "Jane",
	  "last_name": "Smith",
	  "age": 32,
	  "about": "I like to collect rock albums",
	  "interests": [
		"music"
	  ]
	}
	Return 2 row(s).
	Takes 1.181983s.
	```
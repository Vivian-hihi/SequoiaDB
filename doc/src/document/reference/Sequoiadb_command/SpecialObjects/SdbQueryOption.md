统一指定 cond/sel/sort/hint/skip/limit/update/remove 参数。

##语法##

***SdbQueryOption.cond([cond]).sel([sel]).sort([sort]).hint([hint]).skip([skipNum]).limit([retNum]).update([update]).remove([remove])***

##参数描述##

| 参数名 			| 参数类型 	| 描述 		| 是否必填 |
| ------ 			| ------ 	| ------ 	| ------   |
|cond|	Json 对象 | 记录匹配条件。为空时，查询所有记录；不为空时，查询符合条件记录。如：{"age":{"$gt":30}}。关于匹配条件的使用，可参考[匹配符](reference/operator/match_operator/overview.md)。 | 否 |
|sel     |Json 对象 | 查询返回记录的字段名。为空时，返回记录的所有字段；如果指定的字段名记录中不存在，则按用户设定的内容原样返回。如：{"name":"","age":"","addr":""}。 | 否 |
|sort |	Json 对象 | 对结果集按指定字段排序。字段名的值为1或者-1，1代表升序；-1代表降序。 如果不设定 sort 则表示不对结果集做排序。 | 否 |
| hint   |	Json 对象 | 查询是否使用索引及使用哪个索引将由数据库决定；  | 否 |
| skipNum | int | 自定义结果集从哪条记录返回。 | 否 |
| retNum  |	int | 自定义返回结果集的记录条数。 | 否 |
| update  |	Json 对象 | 更新查询后的结果集，详见 update 参数说明。 | 否 |
| remove  |	 | 删除查询后的结果集。 | 否 |

##update 参数##
| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | -------- | ---- | -------- |
| rule    | Json 对象 | 更新规则，记录按指定规则更新。 | 是 |
| returnNew | bool      | 是否返回更新后的记录。         | 否 |
| options | Json 对象 | 可选项，详见 options 选项说明。| 否 |

##update - options 选项##

| 参数名          | 参数类型 | 描述                | 默认值 |
| --------------- | -------- | ------------------- | ------ |
| KeepShardingKey | bool     | 是否保留分区键字段。| false  |

> **Note：**

>* sel 参数是一个json结构，如：{字段名:字段值}，字段值一般指定为空串。sel中指定的字段名在记录中存在，设置字段值不生效；不存在则返回sel中指定的字段名和字段值。
>* 记录中字段值类型为数组的，我们可以在sel中指定该字段名，用"."操作符加上双引号("")来引用数组元素。

>* 不指定`hint`：查询是否使用索引及使用哪个索引将由数据库决定；
>* `hint`为{"":null}：查询走表扫描；
>* `hint`为单个索引：如：{"":"myIdx"}，表示查询将使用当前集合中名字为"myIdx"的索引进行；
>* `hint`为多个索引：如：{"1":"idx1","2":"idx2","3":"idx3"}，
                        表示查询将使用上述三个索引之一进行。
                        具体使用哪一个，由数据库评估决定。

>* skip() 方法的定义格式包含 skipNum 参数，它是 int 类型。如果不设定 num 的内容或者设定 num 的值为0，相当于返回所有的结果集；如果想从结果集的第3条记录开始返回，可是设置 num 的值等于2。
>* limit() 方法的定义格式包含 retNum 参数，它是 int 类型。如果不设定 num 的内容，相当于返回所有的结果集记录。如果想返回结果集的前5条记录，可是设置 num 的值为5。
>* query.update()方法的定义格式包含 rule 参数、 returnNew 参数和 options 参数。其中 rule 参数与 [update()](reference/Sequoiadb_command/SdbCollection/update.md)的 rule 参数相同，options 参数与 [update()](reference/Sequoiadb_command/SdbCollection/update.md)的 options 参数相同。returnNew 参数默认为 false，当为 true 时，返回修改后的记录值。

##返回值##

返回自身，类型为 SdbQueryOption。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

1. 查询匹配条件的记录，即设置 cond 参数的内容。如下操作返回集合 bar 中符合
   条件 age 字段值大于25且 name 字段值为"Tom"的记录。

	```lang-javascript
    > var query = new SdbQueryOption().cond( { age: { $gt: 25 }, name: "Tom" } )
 	> db.foo.bar.find( query )
 	```

2. 指定返回的字段名，即设置 sel 参数的内容。如有记录{ age: 25, type: "system" }
   和{ age: 20, name: "Tom", type: "normal" }，如下操作返回记录的age字段和name字段。

	```lang-javascript
    > var query = new SdbQueryOption().sel( { age: "", name: "" } )
 	> db.foo.bar.find( query )
 	{
    	"age": 25,
      	"name": ""
 	}
 	{
      	"age": 20,
      	"name": "Tom"
 	}
 	```

3. 返回集合 bar 中 age 字段值大于20的记录（如使用 [$gt](reference/operator/match_operator/gt.md) 查询），设置只返回记录的 name 和 age 字段，并按 age 字段值的升序排序。

	```lang-javascript
	> var query = new SdbQueryOption().cond( { age: { $gt: 20 } } ).sel( { age: "", name: "" } ).sort( { age: 1 } )
	> db.foo.bar.find( query )
	```

	> 通过 [find()](reference/Sequoiadb_command/SdbCollection/find.md) 方法，我们能任意选择我们想要返回的字段名，在上例中我们选择了返回记录的 age 和 name 字段，此时用 sort() 方法时，只能对记录的 age 或 name 字段排序。而如果我们选择返回记录的所有字段，即不设置 find 方法的 sel 参数内容时，那么 sort() 能对任意字段排序。

4. 指定一个无效的排序字段。

	```lang-javascript
	> var query = new SdbQueryOption().cond( { age: { $gt: 20 } } ).sel( { age: "", name: "" } ).sort( { "sex": 1 } )
	> db.foo.bar.find( query )
	```

	> 因为“sex”字段并不存在于 sel() 选项 {age:"",name:""} 中，所以 sort() 指定的排序字段 {"sex":1} 将被忽略。


5. 使用索引 ageIndex 遍历集合 bar 下存在 age 字段的记录，并返回。

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

6. 选择集合 bar 下 age 字段值大于10的记录（如使用 [$gt](reference/operator/match_operator/gt.md) 查询），从第5条记录开始返回，即跳过前面的四条记录

	```lang-javascript
	> var query = new SdbQueryOption().cond( { age: { $gt: 10 } } ).skip( 4 )
	> db.foo.bar.find( query )
	```

	> 如果结果集的记录数小于5，那么无记录返回；如果结果集的记录数大于5，则从第5条开始返回。

7. 选择集合 bar 下 age 字段值大于10的记录（如使用 [$gt](reference/operator/match_operator/gt.md) 查询），并只返回前面2条记录。

	```lang-javascript
	> var query = new SdbQueryOption().cond( { age: { $gt: 10 } } ).limit( 2 )
	> db.foo.bar.find( query )
	{
  	"_id": {
    	"$oid": "5813035cc842af52b6000009"
  	},
  	"name": "Tom",
  	"age": 11
	}
	{
  	"_id": {
  	  "$oid": "58130372c842af52b600000a"
  	},
  	"name": "Jack",
	  "age": 12
	}
	```
 
	> 如果结果集的记录数小于2，按实际的记录数返回，如果结果集的记录数大于2，则只返回前2条记录。

8. 查询集合 bar 下 age 字段值大于10的记录，并将符合条件的记录的 age 字段加1。

	```lang-javascript
	> var query = new SdbQueryOption().cond( { age: { $gt: 10 } } ).update( { $inc: { age: 1 } } )
	> db.foo.bar.find( { query )
	```

	> 1. 不能与 query.remove()同时使用。  
	> 2. 与 query.sort()同时使用时，在单个节点上排序必须使用索引。  
	> 3. 在集群中与 query.limit()或 query.skip()同时使用时，要保证查询条件会在单个节点或单个子表上执行。

9. 查询集合 bar 下 age 字段值大于10的记录，并将符合条件的记录删除。

	```lang-javascript
	> var query = new SdbQueryOption().cond( { age: { $gt: 10 } } ).remove()
	> db.foo.bar.find( query )
	```

	> 1. 不能与 query.update() 同时使用。  
	> 2. 与 query.sort() 同时使用时，在单个节点上排序必须使用索引。  
	> 3. 在集群中与 query.limit() 或 query.skip() 同时使用时，要保证查询条件会在单个节点或单个子表上执行。
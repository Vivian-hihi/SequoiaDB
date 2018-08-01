##语法##
***query.sel( \<sel\> )***

查询返回记录的字段名。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | -------- | ---- | -------- |
|sel     |Json 对象 | 查询返回记录的字段名。为空时，返回记录的所有字段；如果指定的字段名记录中不存在，则按用户设定的内容原样返回。如：{"name":"","age":"","addr":""}。 | 否 |

> **Note:**

>* sel 参数是一个json结构，如：{字段名:字段值}，字段值一般指定为空串。sel中指定的字段名在记录中存在，设置字段值不生效；不存在则返回sel中指定的字段名和字段值。
>* 记录中字段值类型为数组，我们可以在sel中指定该字段名，用"."操作符加上双引号("")来引用数组元素。

##返回值##

返回 query 自身，类型为 SdbQueryOption。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

1. 指定返回的字段名，即设置 sel 参数的内容。如有记录{ age: 25, type: "system" }
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
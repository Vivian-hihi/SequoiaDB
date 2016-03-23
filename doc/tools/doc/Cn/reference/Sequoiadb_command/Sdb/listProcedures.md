## 语法##
***db.listProcedures([cond])***

枚举所有的存储过程函数。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| cond | Json 对象 | 条件为空时，枚举所有的函数，不为空时，枚举符合条件函数。 | 是 |

listProcedures() 方法的定义，只有一个 Json 对象类型的参数名 cond，输入值时返回符合指定值的函数，否则的话返回所有的函数。

## 示例##

* 返回所有的函数信息

<pre class="prettyprint lang-javascript">
> db.listProcedures()

{ "_id" : { "$oid" : "52480389f5ce8d5817c4c353" }, 
  "name" : "sum", 
  "func" : "function sum(x, y) {return x + y;}", 
  "funcType" : 0 
}
{ "_id" : { "$oid" : "52480d3ef5ce8d5817c4c354" }, 
  "name" : "getAll", 
  "func" : "function getAll() {return db.foo.bar.find();}", 
  "funcType" : 0 
}
...</pre>

* 指定返回函数名为 sum 的记录

<pre class="prettyprint lang-javascript">
> db.listProcedures({name:"sum"})

{ "_id" : { "$oid" : "52480389f5ce8d5817c4c353" }, 
  "name" : "sum", 
  "func" : "function sum(x, y) {return x + y;}", 
  "funcType" : 0 
}</pre>

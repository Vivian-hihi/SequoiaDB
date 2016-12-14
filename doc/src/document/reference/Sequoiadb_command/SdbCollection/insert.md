##语法##
***db.collectionspace.collection.insert\(\<doc|docs\>,\[flag\]\)***

向指定集合中插入记录。如果集合空间或集合不存在，首先需要手动创建一个集合空间，如 db.createCS("foo")，再在该集合空间下手动创建集合，如 db.foo.createCL\("bar"\)。然后在集合中插入记录。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | -------- | ---- | -------- |
| doc&#124;docs | Json 对象 | 文档记录。doc 为一条记录，docs 为多条记录。 | 是 |
| flag | Int | 可取 SDB_INSERT_RETURN_ID 或者 SDB_INSERT_CONTONDUP。前者在插入单条记录时有效，表示插入记录后返回记录中“_id”字段内容；后者在插入多条记录时有效，表示在插入的记录中，若存在“_id”字段内容重复的记录时，将跳过这些存在重复“_id”的记录继续插入后面记录。默认情况下，当存在重复“_id”字段内容的记录时，将停止插入后面的记录。 | 否 |

> **Note:**
>
> 如果插入的记录不指定 _id 字段时，SequoiaDB 会自动为记录添加一个 _id 字段来标识记录的唯一性。


##返回值##

无返回值，出错抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误信息码。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

## 示例##

* 不指定 _id 字段，插入一条记录。

 ```lang-javascript
 > db.foo.bar.insert( { name: "Tom", age: 20 } )
 ```

* 插入一条带有 _id 字段的记录。

 ```lang-javascript
 > db.foo.bar.insert( {_id: 10, age: 20 } )
 ```


* 插入多条记录，如下操作会在集合bar中插入两条记录。


 ```lang-javascript
 db.foo.bar.insert( [ { _id: 20, name: "Mike", age: 15 }, { name: "John", age: 25, phone: 123 } ] )
 ```

* 插入拥有重复“_id”键的多条记录，如下操作将会在集合bar中插入两条记录。

 ```lang-javascript
 > db.foo.bar.insert( [ { _id: 1, a: 1 }, { _id: 1, b:2 }, { _id: 3, c: 3 } ],  SDB_INSERT_CONTONDUP )
 ```

 ```lang-javascript
 > db.foo.bar.find()
 {
      "_id": 1,
      "a": 1,
 }
 {
      "_id": 3,
      "c": 3
 }
 ```

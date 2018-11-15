##语法##
**db.collectionspace.collection.insert\(\<doc|docs\>,\[flag\]\)**

**db.collectionspace.collection.insert\(\<doc|docs\>,\[options\]\)**

向指定集合中插入记录。如果集合空间或集合不存在，首先需要手动创建一个集合空间，如 db.createCS("foo")，再在该集合空间下手动创建集合，如 db.foo.createCL\("bar"\)。然后在集合中插入记录。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | -------- | ---- | -------- |
| doc&#124;docs | Json 对象 | 文档记录。doc 为一条记录，docs 为多条记录。 | 是 |
| flag | Int | 可取 SDB_INSERT_RETURN_ID 或者 SDB_INSERT_CONTONDUP。前者在插入单条记录时有效，表示插入记录后返回记录中“_id”字段内容；后者在插入多条记录时有效，表示在插入的记录中，若存在“_id”字段内容重复的记录时，将跳过这些存在重复“_id”的记录继续插入后面记录。默认情况下，当存在重复“_id”字段内容的记录时，将停止插入后面的记录。 | 否 |
| options | Json 对象 | 指定插入的行为及结果。 | 否 |

参数 options 属性选项如下：

| 属性名 | 类型 | 描述 | 格式 |
| --- | --- | --- | --- |
| ContOnDup | Bool | 表示当数据库中遇到 "duplicate key error" 时，数据库是否继续执行插入操作。当设置为 true 时，表示数据库将忽略 "duplicate key error" 错误，而继续执行插入，并正常返回；当设置为 false 时，表示当数据库遇到 "duplicate key error" 时，将停止插入，并报错返回。默认值为 false。| {ContOnDup: true} |
| ReturnOID | Bool | 表示插入是否返回记录的"_id"字段的内容。默认为false，表示不返回。| {ReturnOID: true} |

> **Note:**
>
> 如果插入的记录不指定 _id 字段时，SequoiaDB 会自动为记录添加一个 _id 字段来标识记录的唯一性。


##返回值##

* flag 为 SDB_INSERT_RETURN_ID 时，单条插入返回记录的 “_id” 字段的内容。
* options 参数的 “ReturnOID” 选项可以控制单条插入及批量插入以 Json 对象的方式返回 “_id” 字段的内容。
* 其它情况无返回值，出错抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误信息码。

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
 > db.foo.bar.insert( [ { _id: 20, name: "Mike", age: 15 }, { name: "John", age: 25, phone: 123 } ] )
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

* 插入记录，并以 Json 对象的方式返回结果。

 ```lang-javascript
 > db.foo.bar.insert({a:1}, {ReturnOID:true, ContOnDup:true})
 {
   "_id": {
     "$oid": "5becec3d6404b9295a63caca"
   }
 }
 > db.foo.bar.insert([{a:1}, {b:1}], {ReturnOID:true, ContOnDup:true})
 {
   "_id": [
     {
       "$oid": "5bececdf6404b9295a63cacb"
     },
     {
       "$oid": "5bececdf6404b9295a63cacc"
     }
   ]
 }
 ```

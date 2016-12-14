##语法##
***db.collectionspace.collection.listIndexes\(\)***

枚举[索引](basic_operation/indexes.md)，执行此方法会将指定集合下的索引信息全部显示出来。

##参数##

无

##返回值##

返回游标。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

## 示例##

* 返回集合 bar 下的所有索引信息

 ```lang-javascript
 > db.foo.bar.listIndexes()
 ```

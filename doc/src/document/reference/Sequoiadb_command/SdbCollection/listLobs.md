##名称##

listLobs - 列取集合中的大对象。

##语法##
***db.collectionspace.collection.listLobs([SdbQueryOption])***

##类别##

Collection

##描述##

获取集合中的大对象列表，并通过游标（cursor）的方式返回。

##参数##

* `SdbQueryOption`( *Object*， *选填* )
    
    使用一个对象来指定记录查询参数。使用方法可参考[SdbQueryOption](reference/Sequoiadb_command/AuxiliaryObjects/SdbQueryOption.md)。
    
    >**Note:**
    >
    > 特殊的，当使用SdbQueryOption指定hint为{"ListPieces": 1}时，可以列取lob的分片详细信息
    >

##返回值##

成功：返回DBCursor对象。  

失败：抛出异常。

##错误##

`listLobs()`函数常见异常如下：

| 错误码 | 错误类型 | 描述 | 解决方法 |
| ------ | ------ | --- | ------ |
| -6 | SDB_INVALIDARG | 参数错误。 | 查看参数是否填写正确。|
| -34 | SDB_DMS_CS_NOTEXIST | 集合空间不存在。| 检查集合空间是否存在。|
| -23 | SDB_DMS_NOTEXIST| 集合不存在。 | 检查集合是否存在。|

当异常抛出时，可以通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取[错误码](reference/Sequoiadb_error_code.md)，
或通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息。
可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)了解更多内容。

## 示例##

* 列取 foo.bar 中的所有大对象

 ```lang-javascript
 > db.foo.bar.listLobs()
 {
  "Size": 2,
  "Oid": {
    "$oid": "00005d36c8a5350002de7edc"
  },
  "CreateTime": {
    "$timestamp": "2019-07-23-16.43.17.360000"
  },
  "ModificationTime": {
    "$timestamp": "2019-07-23-16.43.17.508000"
  },
  "Available": true,
  "HasPiecesInfo": false
 }
 {
  "Size": 51717368,
  "Oid": {
    "$oid": "00005d36cae8370002de7edd"
  },
  "CreateTime": {
    "$timestamp": "2019-07-23-16.52.56.278000"
  },
  "ModificationTime": {
    "$timestamp": "2019-07-23-16.52.56.977000"
  },
  "Available": true,
  "HasPiecesInfo": false
 }
 Return 2 row(s).
 Takes 0.002045s.
 ```

* 列取 foo.bar 中的大小大于10的大对象

 ```lang-javascript
 > db.foo.bar.listLobs( SdbQueryOption().cond( { "Size": { $gt: 10 } } ) )
 {
  "Size": 51717368,
  "Oid": {
    "$oid": "00005d36cae8370002de7edd"
  },
  "CreateTime": {
    "$timestamp": "2019-07-23-16.52.56.278000"
  },
  "ModificationTime": {
    "$timestamp": "2019-07-23-16.52.56.977000"
  },
  "Available": true,
  "HasPiecesInfo": false
 }
 Return 1 row(s).
 Takes 0.003665s.
 ```
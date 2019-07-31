##语法##

***query.flags( \<flag\> )***

##类别##

SdbQuery

##描述##

按指定的标志位遍历结果集。

##参数##

| 参数名 | 参数类型 | 默认值 | 描述   | 是否必填 |
| ------ | -------- | ------ | ------ | -------- |
| flag   | int      | ---    | 标志位 | 是       |

flag 参数的可选值如下表：

| 可选值 | 描述                         |
| ------ | ---------------------------- |
| 128    | 强制使用指定的索引进行查询，如果数据库没有通过提示指定的索引，则无法查询 |
| 256    | 启用并行子查询，每个子查询将完成扫描不同部分的数据 |
| 512    | 一般来说，查询不会返回数据，直到从数据库中获取游标，当添加此标志位时，将在查询响应中返回数据 |
| 16384  | 在查询时，启用准备更多数据 |
| 65536  | 当事务处于开启状态且事务隔离级别为 “RC” 时，默认情况下读取记录后将释放事务锁定。但是，在设置此标志位时，事务锁将不会在事务提交或回滚之前释放。当事务处于关闭状态或者事务隔离级别为 “RU” 时，该标志位不起作用 |
 
##返回值##

返回查询结果集的游标。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 指定标志位遍历结果集。

   ```lang-javascript
   > db.foo.bar.find().flags( 256 )
   {
      "_id": {
        "$oid": "5d412cfa614afb5557b2b41d"
      },
      "name": "fang",
      "age": 18
   }
   {
      "_id": {
        "$oid": "5d412cfa614afb5557b2b41c"
      },
      "name": "alice",
      "age": 19
   }
   {
      "_id": {
        "$oid": "5d412cfa614afb5557b2b41b"
      },
      "name": "ben",
      "age": 21
   }
   ```
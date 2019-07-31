##NAME##

flags - Specify flag bit to traverse the result set.

##SYNOPSIS##

***query.flags( \<flag\> )***

##CATEGORY##

SdbQuery

##DESCRIPTION##

Specify flag bit to traverse the result set.

##PARAMETERS##

| Name | Type | Default | Description | Required or not |
| ---- | ---- | ------- | ----------- | --------------- |
| flag | int  | ---     | flag bit    | yes             |

The optional values of the 'flag' parameter are as follows：

| Optional values | Description                                                       |
| --------------- | ----------------------------------------------------------------- |
| 128             | Force to use specified hint to query, if database have no index assigned by the hint, fail to query |
| 256             | Enable parallel sub query, each sub query will finish scanning different part of the data |
| 512             | In general, query won't return data until cursor gets from database, when add this flag, return data in query response, it will be more high-performance |
| 16384           | Enable prepare more data when query |
| 65536           | When the transaction is turned on and the transaction isolation level is "RC", the transaction lock will be released after the record is read by default. However, when setting this flag, the transaction lock will not released until the transaction is committed or rollback. When the transaction is turned off or the transaction isolation level is "RU", the flag does not work |

##RETURN VALUE##

On success, returns the cursor of the query result set.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Specify flag bit to traverse the result set.

```lang-javascript
> db.foo.bar.find().flags(256)
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
{
  "_id": {
    "$oid": "5d412cfa614afb5557b2b41a"
  },
  "name": "tom",
  "age": 20
}
```

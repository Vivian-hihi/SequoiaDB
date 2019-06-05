##描述##

集合空间列表 $LIST_CS 列出当前数据库节点中所有的集合空间，每个集合空间为一条记录。

##标示##

$LIST_CS

##字段信息##

| 字段名 | 类型   | 描述       |
| ----- | ----- | ----- |
| CLUniqueHWM | 长整型 | 集合空间最近创建的集合的UniqueID |
| Collection | 数组 | 包含集合的信息 |
| LogicalID | 字符串 | 集合空间逻辑 ID，为递增顺序 |
| Name   | 字符串 | 集合空间名 |
| PageSize | 整型 | 集合空间数据页大小 |
| UniqueID | 整型   | 集合空间的UniqueID，在集群上全局唯一 |

##示例##

```lang-javascript
> db.exec( "select * from $LIST_CS" )
{
  "CLUniqueHWM": 2469606195201,
  "Collection": [
    {
      "Name": "bar",
      "UniqueID": 2469606195201
    }
  ],
  "LobPageSize": 262144,
  "Name": "foo",
  "PageSize": 65536,
  "Type": 0,
  "UniqueID": 575,
  "_id": {
    "$oid": "5cf4b69607c2e1754b77c5ee"
  }
}
...
```

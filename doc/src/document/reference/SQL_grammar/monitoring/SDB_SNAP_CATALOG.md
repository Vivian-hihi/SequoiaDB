##描述##

编目信息快照 $SNAPSHOT_CATA 列出当前数据库中所有集合的编目信息，每个集合一条记录。

##标示##

$SNAPSHOT_CATA

>   **Note:**
>
>   只能在协调节点执行。

##字段信息##

| 字段名              | 类型   | 描述                         |
| ------------------- | ------ | ---------------------------- |
| Name                | 字符串 | 集合完整名                   |
| UniqueID            | 长整型 | 集合的UniqueID，在集群上全局唯一 |
| EnsureShardingIndex | 布尔   | 是否自动为分区键字段创建索引 |
| ReplSize            | 整型   | 执行修改操作时需要同步的副本数<br>当执行更新、插入、删除记录等操作时，仅当指定副本数的节点都完成操作时才返回操作结果 |
| ShardingKey         | 对象   | 数据分区类型：<br>- range：数据按分区键值的范围进行分区存储<br>- hash：数据按分区键的哈希值进行分区存储 |
| Version             | 整型   | 集合版本号，当对集合的元数据执行修改操作时递增该版本号（例如数据切分） |
| Attribute           | 整型   | 集合属性                     |
| AttributeDesc       | 字符串 | 集合属性描述                 |
| CompressionType     | 整型   | 压缩算法类型                 |
| CompressionTypeDesc | 字符串 | 压缩算法类型描述             |
| CataInfo.GroupID    | 整型   | 分区组 ID                    |
| CataInfo.GroupName  | 字符串 | 分区组名                     |
| CataInfo.LowBound   | 对象   | 数据分区区间的上限           |
| CataInfo.UpBound    | 对象   | 数据分区区间的下限           |
| AutoIncrement.Field | 字符串 | 自增字段名称                 |
| AutoIncrement.Generated | 字符串 | 自增字段生成方式         |
| AutoIncrement.SequenceName | 字符串 | 自增字段对应序列名    |
| AutoIncrement.SequenceID | 长整型 | 自增字段对应序列ID      |

##示例##

```lang-javascript
> db.exec( "select * from $SNAPSHOT_CATA" )
{
  "Attribute": 1,
  "AttributeDesc": "Compressed",
  "CataInfo": [
    {
      "ID": 1,
      "SubCLName": "year2015.month01",
      "LowBound": {
        "a": 0
      },
      "UpBound": {
        "a": 100
      }
    }
  ],
  "CompressionType": 1,
  "CompressionTypeDesc": "lzw",
  "EnsureShardingIndex": true,
  "IsMainCL": true,
  "Name": "newmaincs.newmaincl",
  "ShardingKey": {
    "a": 1
  },
  "ShardingType": "range",
  "UniqueID": 502511173633,
  "Version": 6,
  "_id": {
    "$oid": "5cf08d3007c2e1754b77ba97"
  }
}
{
  "Attribute": 1,
  "AttributeDesc": "Compressed",
  "CataInfo": [
    {
      "GroupID": 1001,
      "GroupName": "db2",
      "LowBound": {
        "": 0
      },
      "UpBound": {
        "": 1024
      }
    }
  ],
  "CompressionType": 1,
  "CompressionTypeDesc": "lzw",
  "EnsureShardingIndex": true,
  "InternalV": 3,
  "MainCLName": "newmaincs.newmaincl",
  "Name": "year2015.month01",
  "Partition": 1024,
  "ShardingKey": {
    "a": 1
  },
  "ShardingType": "hash",
  "UniqueID": 532575944705,
  "Version": 4,
  "_id": {
    "$oid": "5cf08d3a07c2e1754b77baa8"
  }
}
{
  "Attribute": 1,
  "AttributeDesc": "Compressed",
  "CataInfo": [
    {
      "GroupID": 1000,
      "GroupName": "db1",
      "LowBound": {
        "": 0
      },
      "UpBound": {
        "": 1024
      }
    }
  ],
  "CompressionType": 1,
  "CompressionTypeDesc": "lzw",
  "EnsureShardingIndex": true,
  "InternalV": 3,
  "Name": "year2015.month02",
  "Partition": 1024,
  "ShardingKey": {
    "a": 1
  },
  "ShardingType": "hash",
  "UniqueID": 532575944706,
  "Version": 3,
  "_id": {
    "$oid": "5cf08d5107c2e1754b77babf"
  }
}
{
  "Attribute": 1,
  "AttributeDesc": "Compressed",
  "CataInfo": [],
  "CompressionType": 1,
  "CompressionTypeDesc": "lzw",
  "EnsureShardingIndex": true,
  "IsMainCL": true,
  "Name": "local_test_csmcs.local_test_cl_mcl",
  "ReplSize": 7,
  "ShardingKey": {
    "a": 1
  },
  "ShardingType": "range",
  "UniqueID": 717259538433,
  "Version": 9,
  "_id": {
    "$oid": "5cf08fa807c2e1754b77bd52"
  }
}
{
  "_id": {
    "$oid": "5cf4bcdf07c2e1754b77c5f2"
  },
  "Name": "foo.bar2",
  "UniqueID": 2469606195203,
  "Version": 2,
  "Attribute": 1,
  "AttributeDesc": "Compressed",
  "CompressionType": 1,
  "CompressionTypeDesc": "lzw",
  "CataInfo": [
    {
      "GroupID": 1000,
      "GroupName": "db1"
    }
  ]
}
{
  "Attribute": 1,
  "AttributeDesc": "Compressed",
  "CataInfo": [],
  "CompressionType": 1,
  "CompressionTypeDesc": "lzw",
  "EnsureShardingIndex": true,
  "IsMainCL": true,
  "Name": "local_test_cs.local_test_cl_mcl",
  "ShardingKey": {
    "a": 1
  },
  "ShardingType": "range",
  "UniqueID": 2418066587684,
  "Version": 56,
  "_id": {
    "$oid": "5cf098b707c2e1754b77c5b0"
  }
}
{
  "Attribute": 1,
  "AttributeDesc": "Compressed",
  "AutoIncrement": [
    {
      "SequenceName": "SYS_2469606195201_studentID_SEQ",
      "Field": "studentID",
      "Generated": "default",
      "SequenceID": 168
    }
  ],
  "CataInfo": [
    {
      "GroupID": 1001,
      "GroupName": "db2"
    }
  ],
  "CompressionType": 1,
  "CompressionTypeDesc": "lzw",
  "Name": "foo.bar",
  "UniqueID": 2469606195201,
  "Version": 2,
  "_id": {
    "$oid": "5cf4b6a307c2e1754b77c5ef"
  }
}
```

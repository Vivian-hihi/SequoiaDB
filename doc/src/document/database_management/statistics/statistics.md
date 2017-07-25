SequoiaDB在查询优化中使用的统计信息，包含集合和索引的数据分布信息。查询优化器可以根据这些统计信息来估计查询结果中的基数，从而创建高质量的查询计划。

在SequoiaDB中有两种统计信息，集合的统计信息和索引的统计信息。

##集合的统计信息##

集合的统计信息存放在数据节点的SYSSTAT.SYSCOLLECTIONSTAT集合中，具体的字段如下：

| 字段名 | 数据类型 | 默认值 | 必须 | 说明 |
| ---- | ---- | ---- | ---- | ---- |
| CollectionSpace | String | | 是 | 统计的Collection所在Collection Space的名称 |
| Collection | String | | 是 | 统计的Collection的名称（不带Collection Space名字） |
| CreateTime | NumberLong | 0 | 是 | 统计收集的时间戳 |
| SampleRecords | NumberLong | 0 | 是 | 统计收集时抽样的文档个数 |
| TotalRecords | NumberLong | 10 | 是 | 统计收集时的文档个数 |
| TotalDataPages | NumberInt | 1 | 是 | 统计收集时的数据页个数 |
| TotalDataSize | NumberLong | | 是 | 统计收集时的数据总大小（字节数） |
| AvgNumFields | NumberInt | 10 | 否 | 文档的平均字段数 |

例子：

```lang-javascript
{
  "Collection": "foo",
  "CollectionSpace": "bar",
  "CreateTime": 1496910925978,
  "SampleRecords": 200,
  "TotalDataPages": 1284,
  "TotalDataSize": 65929411,
  "TotalRecords": 600000,
  "AvgNumFields" : 10
}
```

##索引的统计信息##

索引的统计信息存放在数据节点的SYSSTAT.SYSCOLLECTIONSTAT集合中，具体的字段如下：

| 字段名 | 数据类型 | 默认值 | 必须 | 说明 |
| ---- | ---- | ---- | ---- | ---- |
| CollectionSpace | String | | 是 | 统计的Collection所在Collection Space的名称 |
| Collection | String | | 是 | 统计的Collection的名称（不带Collection Space名字） |
| CreateTime | NumberLong | 0 | 是 | 统计收集的时间戳 |
| Index | String | | 是 | 统计Index的名称 |
| KeyPattern | BSONObj | | 是 | 统计索引的字段定义，例如：{a:1, b:-1} |
| SampleRecords | NumberLong | 0 | 是 | 统计收集时抽样的文档个数 |
| TotalRecords | NumberLong | 10 | 是 | 统计收集时的文档个数 |
| IndexPages | NumberInt | 1 | 是 | 统计收集时索引的页个数 |
| IndexLevels | NumberInt | 1 | 是 | 统计收集时索引的层数 |
| IsUnique | BOOL | FALSE | 是 | Index是否唯一索引 |
| MCV | Object | undefined | 否 | 频繁数值集合(Most Common Values) <br/>如：MCV: { Values: [ {a:1,b:1}, {a:2, b:2}, ... ], Frac: [ 1000, 1000, ... ] } |
| MCV.Values | Array | | 是(如有MCV) | 频繁数值的值 |
| MCV.Frac | Array | | 是(如有MCV) | 频繁数值的比例，每个值的取值 0 ~ 10000，最终比例为 (Frac / 10000) * 100% |

```lang-javascript
{

  "Collection": "foo",
  "CollectionSpace": "bar",
  "CreateTime": 1496910926035,
  "Index": "index",
  "IndexLevels": 2,
  "IndexPages": 256,
  "IsUnique": false,
  "KeyPattern": {
    "a": 1
  },
  "MCV": {
    "Values": [
      {
        "a": 2358
      },
      {
        "a": 7074
      },
      {
        "a": 11790
      },
      ...
    ],
    "Frac": [
      50,
      50,
      50,
      ...
    ]
  },
  "SampleRecords": 200,
  "TotalRecords": 600000
}
```

##统计信息的收集##

请参考[db.analyze\(\)](reference/Sequoiadb_command/Sdb/analyze.md)。

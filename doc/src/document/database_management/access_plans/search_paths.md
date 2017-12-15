##访问计划评估##

当查询的集合有多个索引时，SequoiaDB 需要选取合适的索引，或者全表扫描来执行查询。数据节点上的查询优化器会基于代价对候选的访问计划进行评估，选取合适的访问计划来完成查询。

估算出每个候选访问计划执行的以下指标：

1.  基于规则的估算选取候选访问计划

    1.  索引的选择率 < 0.1（即索引过滤剩下的记录个数为集合记录个数的 10%）
    2.  索引完全匹配排序字段及排序方向
    3.  全表扫描

2.  符合指标 1 的候选访问计划，再基于代价的进行估算，最终选出总代价最小的访问计划执行查询。

**示例**

集合 foo.bar 上的有索引：

1. "index_a" : ```{ a : 1 }```
2. "index_b" : ```{ b : 1 }```
3. "index_c" : ```{ c : 1 }```

查询 ```db.foo.bar.find( { a : 1, b : 2 } ).sort( { c : 1 } )``` 可以有以下的访问计划：

1.  IXSCAN( "index_a" ) ==> SORT( { c : 1 } )
2.  IXSCAN( "index_b" ) ==> SORT( { c : 1 } )
3.  IXSCAN( "index_c" )
4.  TBSCAN() ==> SORT( { c : 1 } )

根据指标 1 可以确定 4 个都是候选的访问计划，其中访问计划 1 和 2 满足指标 1.1，访问计划 3 满足指标 1.2，访问计划 4 满足指标 1.3。

然后通过代价估算确定总代价最小的访问计划，并选取执行查询。假设估算出 4 个候选访问计划的总代价分别为 1000，800，12000 和 1000，则最终选择访问计划 2 执行查询。

##索引选择率的估算##

索引选择率的估算有两种方式：

1.  [使用统计信息进行估算](database_management/statistics/statistics.md#统计信息的使用)
2.  使用默认值进行估算

使用默认值进行估算：

1.  数值

    1. 在 ```[ -99999999.9, 99999999.9 ]``` 的区间中选取
    2. 如 ```{ $lt : 0 }``` 的选择率为：```( 0 - ( -99999999.9 ) ) / ( 99999999.9 - ( -99999999.9 ) ) = 0.5```

2.  字符串

    1. 逐个字符计算在 ' ' （空格 ASCII 码：32）至 ASCII 码 127 之间的比例
    2. 计算前 20 个字符

3. 其他数据类型

   1. 相等比较：0.005
   2. 大于、小于比较：0.333333
   3. 范围比较：0.05

>   **Note:**
>
>   参考[使用统计信息进行估算](database_management/statistics/statistics.md#统计信息的使用)

##访问计划的搜索过程##

使用 [SdbQuery.explain\(\)](reference/Sequoiadb_command/SdbQuery/explain.md) 可以查看查询的访问计划。

当 SdbQuery.explain() 的 Search 选项为 true 时，将会展示查询优化器搜索过的访问计划，并查看查询优化器选择的结果。
当 SdbQuery.explain() 的 Evaluate 选项为 true 时，将会展示查询优化器估算访问计划的总代价的演算过程。

>   **Note:**
>  
>   1.  搜索过的访问计划不在访问计划缓存中，因此 Search 选项不使用缓存，重新估算
>   2.  搜索过程将嵌套展示在数据节点每个集合的访问计划中

###访问计划的搜索过程的信息###

Search 选项为 true 时，将展示以下信息：

| 字段名                      | 类型      | 描述                                                                                 |
| --------------------------- | --------- | ------------------------------------------------------------------------------------ |
| Constants                   | BSON 对象 | 生成访问计划使用的常量<br>Evaluate 选项为 true 时显示                                |
| Constants.RandomIOCostUnit  | 整型      | 随机读取 IO 的代价，默认值为 10                                                      |
| Constants.SeqIOCostUnit     | 整型      | 顺序读取 IO 的代价，默认值为 1                                                       |
| Constants.SeqWrtIOCostUnit  | 整型      | 顺序写入 IO 的代价，默认值为 2                                                       |
| Constants.PageUnit          | 整型      | 数据页的单位，默认值为 4096 （单位：字节）                                           |
| Constants.RecExtractCPUCost | 整型      | 从数据页中提取数据的 CPU 代价，默认值为 4                                            |
| Constants.IXExtractCPUCost  | 整型      | 从索引页中提取索引项的 CPU 代价，默认值为 2                                          |
| Constants.OptrCPUCost       | 整型      | 操作符的 CPU 代价单位，默认值为 1                                                    |
| Constants.IOCPURate         | 整型      | IO 代价与 CPU 代价的比例，默认值为 2000                                              |
| Constants.TbScanStartCost   | 整型      | 全表扫描的启动代价，默认值为 0                                                       |
| Constants.IxScanStartCost   | 整型      | 索引扫描的启动代价，默认值为 0                                                       |
| Options                     | BSON      | 生成访问计划使用的配置项，即 SequoiaDB 的配置                                        |
| Options.optcostthreshold    | 整型      | SequoiaDB 的 --optcostthreshold 选项，查询优化器忽略 IO 影响的最小的页数<br>数据页数大于阈值时，估算访问计划代价时需要计算 IO 的代价<br>默认值为 20，0 表示一直需要计算 IO 代价，-1 表示从不计算代价 |
| Options.sortbuf             | 整型      | SequoiaDB 的 --sortbuf 选项<br>排序缓存大小（单位：MB），默认值为 256 ，最小值为 128 |
| Inputs                      | BSON 对象 | 生成访问计划使用的输入项，集合的统计信息<br>Evaluate 选项为 true 时显示              |
| Inputs.Pages                | 整型      | 集合的数据页个数                                                                     |
| Inputs.Records              | 长整型    | 集合的数据个数                                                                       |
| Inputs.NeedEvalIO           | 布尔型    | 根据 Input.Pages 和 Options.optcostthreshold 判断是否需要计算 IO 代价                |
| Inputs.CLEstFromStat        | 布尔型    | 是否使用集合的统计信息进行估算                                                       |
| Inputs.CLStatTime           | 时间戳    | 使用的集合的统计信息的生成时间                                                       |
| SearchPaths                 | 数组      | 每个搜索过的访问计划的估算过程                                                       |

SearchPaths 数组的每项表示一个搜索过的访问计划，将展示以下信息：

| 字段名         | 类型      | 描述                                             |
| -------------- | --------- | ------------------------------------------------ |
| ScanType       | 字符串    | 访问计划的扫描方式<br>1. "tbscan" 表示全表扫描<br>2. "ixscan" 表示索引扫描 |
| IndexName      | 字符串    | 访问计划使用的索引的名称<br>全表扫描时为 ""      |
| UseExtSort     | 布尔型    | 访问计划是否使用非索引排序                       |
| Query          | BSON 对象 | 访问计划解析后的用户查询条件                     |
| IXBound        | BSON 对象 | 访问计划使用索引的查找范围<br全>表扫描为 null    |
| NeedMatch      | 布尔型    | 访问计划获取记录时是否需要根据匹配符进行过滤<br>NeedMatch 为 false 的情况有：<br>1. 没有查询条件<br>2. 查询条件可以被索引覆盖 |
| IXEstFromStat  | 布尔型    | 是否使用索引的统计信息进行估算（索引扫描时显示） |
| IXStatTime     | 时间戳    | 使用的索引的统计信息的生成时间（索引扫描时显示） |
| Score          | 浮点型    | 评分：<br>1. 索引扫描为索引的选择率（< 0.1时为候选计划）<br>2. 全表扫描为匹配符的选择率 |
| IsCandidate    | 布尔型    | 是否候选访问计划，不是候选计划不进行估算<br>1. 索引扫描选择率 < 0.1<br>2. 索引扫描完全匹配排序字段<br>3. 全表扫描 |
| IsUsed         | 布尔型    | 是否最终选择的访问计划                           |
| TotalCost      | 浮点型    | 估算的代价（内部表示 单位约为 1/2000000 秒）<br>该代价不包括选择符、skip() 和 limit() 的影响 |
| ScanNode       | BSON 对象 | [TBSCAN 的推演公式](database_management/access_plans/search_paths.md#TBSCAN的推演公式) 或 [IXSCAN 推演公式](database_management/access_plans/search_paths.md#IXSCAN的推演公式)<br>Evaluate 选项为 true 时显示 |
| SortNode       | BSON 对象 | [SORT 的推演公式](database_management/access_plans/search_paths.md#SORT的推演公式)<br>Evaluate 选项为 true 且需要进行排序时显示 |


Evaluate 选项为 true 时将展示查询优化器的推演公式，每个需要计算的变量将以数组形式展示：

```
变量: [
  公式,
  代入数据的计算公式,
  计算结果
]
```
>   **Note:**
>
>   推演公式中的代价均为内部表示，单位约为 1/2000000 秒


**示例**

```
{
  ...,
  "Search": {
    "Options": {
      "sortbuf": 256,
      "optcostthreshold": 20
    },
    "Constants": {
      "RandomReadIOCostUnit": 10,
      "SeqReadIOCostUnit": 1,
      "SeqWrtIOCostUnit": 2,
      "PageUnit": 4096,
      "RecExtractCPUCost": 4,
      "IXExtractCPUCost": 2,
      "OptrCPUCost": 1,
      "IOCPURate": 2000,
      "TBScanStartCost": 0,
      "IXScanStartCost": 0
    },
    "Input": {
      "Pages": 1,
      "Records": 10,
      "NeedEvalIO": false,
      "CLEstFromStat": false
    },
    "SearchPaths": [
      {
        "IsUsed": false,
        "IsCandidate": false,
        "Score": 1,
        "ScanType": "ixscan",
        "IndexName": "$id",
        "UseExtSort": false,
        "Direction": 1,
        "IXBound": {
          "_id": [
            [
              {
                "$minElement": 1
              },
              {
                "$maxElement": 1
              }
            ]
          ]
        },
        "NeedMatch": false,
        "IXEstFromStat": false
      },
      ...
    ]
  }
}
```

###TBSCAN的推演公式###

TBSCAN 的推演公式将展示以下信息：

| 字段名         | 类型   | 描述 |
| -------------- | ------ | ---- |
| MthSelectivity | 浮点型 | 估算的 TBSCAN 使用匹配符进行过滤的选择率 |
| MthCPUCost     | 整型   | 估算的 TBSCAN 使用匹配符过滤一个记录的 CPU 代价 |
| IOCost         | 数组   | 估算的 TBSCAN 的 IO 代价的公式及计算过程<br>NeedEvalIO 为 false 不需要计算<br>即各个数据页进行顺序扫描的代价总和<br>公式为：```SeqReadIOCostUnit * Pages * ( PageSize / PageUnit )``` |
| CPUCost        | 数组   | 估算的 TBSCAN 的 CPU 代价的公式及计算过程<br>即各个记录从数据页中提取并进行匹配符过滤的代价总和<br>公式为：```Records * ( RecExtractCPUCost + MthCPUCost )``` |
| StartCost      | 数组   | 估算的 TBSCAN 的启动代价（内部表示）<br>公式为：```TBScanStartCost``` |
| RunCost        | 数组   | 估算的 TBSCAN 的运行代价（内部表示）<br>公式为：```IOCPURate * IOCost + CPUCost``` |
| TotalCost      | 数组   | 估算的 TBSCAN 的总代价（内部表示）<br>公式为：```StartCost + RunCost``` |

**示例**

```
"ScanNode": {
  "MthSelectivity": 1,
    "MthCPUCost": 0,
    "IOCost": [
      "SeqReadIOCostUnit * Pages * ( PageSize / PageUnit )",
      "1 * 74 * ( 65536 / 4096 ) ",
      1184
  ],
  "CPUCost": [
    "Records * ( RecExtractCPUCost + MthCPUCost )",
    "100000 * ( 4 + 0 ) ",
    400000
  ],
  "StartCost": [
    "TBScanStartCost",
    "0",
    0
  ],
  "RunCost": [
    "IOCPURate * IOCost + CPUCost",
    "2000 * 1184 + 400000",
    2768000
  ],
  "TotalCost": [
    "StartCost + RunCost",
    "0 + 2768000",
    2768000
  ]
}
```

###IXSCAN的推演公式###

IXSCAN 的推演公式将展示以下信息：

| 字段名           | 类型   | 描述 |
| ---------------- | ------ | ---- |
| IndexPages       | 整型   | 估算的 IXSCAN 输入的索引页数 |
| IndexLevels      | 整型   | 估算的 IXSCAN 输入的索引层数 |
| MthSelectivity   | 浮点型 | 估算的 IXSCAN 使用匹配符进行过滤的选择率 |
| MthCPUCost       | 整型   | 估算的 IXSCAN 使用匹配符过滤一个记录的 CPU 代价 |
| ScanSelectivity  | 浮点型 | 估算的 IXSCAN 使用索引时需要扫描索引的比例 |
| PredSelectivity  | 浮点型 | 估算的 IXSCAN 使用索引进行过滤的选择率 |
| PredCPUCost      | 整型   | 估算的 IXSCAN 使用索引进行过滤一个记录的 CPU 代价 |
| IndexReadPages   | 数组   | 估算的 IXSCAN 需要读取的索引页个数<br>NeedEvalIO 为 false 不需要计算<br>公式为：```max( 1, ceil( IndexPages * ScanSelectivity ) )``` |
| IndexReadRecords | 数组   | 估算的 IXSCAN 需要读取的索引记录个数<br>公式为：```max( 1, ceil( Records * ScanSelectivity ) )``` |
| ReadPages        | 数组   | 估算的 IXSCAN 需要读取的数据页个数<br>NeedEvalIO 为 false 不需要计算<br>公式为：```max( 1, ceil( Pages * PredSelevtivity ) )``` |
| ReadRecords      | 数组   | 估算的 IXSCAN 需要读取的记录个数<br>公式为：```max( 1, ceil( Records * PredSelectivity ) ) |
| IOCost           | 数组   | 估算的 IXSCAN 的 IO 代价的公式及计算过程<br>NeedEvalIO 为 false 不需要计算<br>即各个数据页进行随机扫描的代价总和<br>公式为：```RandomReadIOCostUnit * ( IndexReadPages + ReadPages ) * ( PageSize / PageUnit )``` |
| CPUCost          | 数组   | 估算的 IXSCAN 的 CPU 代价的公式及计算过程<br>即各个记录从索引页和数据页中提取并进行匹配符过滤的代价总和<br>如果需要进行匹配符过滤，公式为：```IndexReadRecords * ( IXExtractCPUCost + PredCPUCost ) + ReadRecords * ( RecExtractCPUCost + MthCPUCost )```<br>如果不需要进行匹配符过滤，公式为：```IndexReadRecords * ( IXExtractCPUCost + PredCPUCost ) + ReadRecords * RecExtractCPUCost``` |
| StartCost        | 数组   | 估算的 IXSCAN 的启动代价（内部表示）<br>公式为：```IXScanStartCost + PredCPUCost * IndexLevels``` |
| RunCost          | 数组   | 估算的 IXSCAN 的运行代价（内部表示）<br>公式为：```IOCPURate * IOCost + CPUCost``` |
| TotalCost        | 数组   | 估算的 IXSCAN 的总代价（内部表示）<br>公式为：```StartCost + RunCost``` |

**示例**

```
"ScanNode": {
  "IndexPages": 49,
  "IndexLevels": 1,
  "MthSelectivity": 0.00001,
  "MthCPUCost": 2,
  "IXScanSelectivity": 0.00001,
  "IXPredSelectivity": 0.00001,
  "PredCPUCost": 1,
  "IndexReadPages": [
    "max( 1, ceil( IndexPages * IXScanSelectivity ) )",
    "max( 1, ceil( 49 * 1e-05 ) )",
    1
  ],
  "IndexReadRecords": [
    "max( 1, ceil( Records * IXScanSelectivity ) )",
    "max( 1, ceil( 100000 * 1e-05 ) )",
    1
  ],
  "ReadPages": [
    "max( 1, ceil( Pages * IXPredSelectivity ) )",
    "max( 1, ceil( 49 * 1e-05 ) )",
    1
  ],
  "ReadRecords": [
    "max( 1, ceil( Records * IXPredSelectivity ) )",
    "max( 1, ceil( 100000 * 1e-05 ) )",
    1
  ],
  "IOCost": [
    "RandomReadIOCostUnit * ( IndexReadPages + ReadPages ) * ( PageSize / PageUnit )",
    "10 * ( 1 + 1 ) * ( 65536 / 4096 ) ",
    320
  ],
  "CPUCost": [
    "IndexReadRecords * ( IXExtractCPUCost + PredCPUCost ) + ReadRecords * RecExtractCPUCost",
    "1 * ( 2 + 1 ) + 1 * 4",
    7
  ],
  "StartCost": [
    "IXScanStartCost + PredCPUCost * IndexLevels",
    "0 + 1 * 1",
    1
  ],
  "RunCost": [
    "IOCPURate * IOCost + CPUCost",
    "2000 * 320 + 7",
    640007
  ],
  "TotalCost": [
    "StartCost + RunCost",
    "1 + 640007",
    640008
  ]
}
```

###SORT的推演公式###

SORT 的推演公式将展示以下信息：

| 字段名          | 类型   | 描述 |
| --------------- | ------ | ---- |
| Records         | 长整型 | 估算的 SORT 输入的记录个数 |
| RecordTotalSize | 长整型 | 估算的 SORT 输入的记录总大小 |
| Pages           | 整型   | 估算的 SORT 输入的记录页数（输入记录个数的总大小存放入 4K 页面中的页数） |
| SortFields      | 整型   | SORT 进行排序的字段个数 |
| SortType        | 字符串 | SORT 估算的排序类型<br>RecordTotalSize 小于 sortbuff 时，"InMemory" 为内存排序<br>RecordTotalSize 大于 sortbuff 时，"External" 为外存排序 |
| IOCost          | 数组   | 估算的 SORT 的 IO 代价的公式及计算过程<br>SortType 为 "InMemory" 时不需要计算<br>各个数据页需要写出磁盘，并进行归并排序，假设归并排序中 75% 为顺序读，25% 为随机读<br>公式为：```Pages * ( SeqWrtIOCostUnit + SeqReadIOCostUnit * 0.75 + RandomReadIOCostUnit * 0.25 )``` |
| CPUCost         | 数组   | 估算的 SORT 的 CPU 代价的公式及计算过程<br>即各个记录进行排序的代价<br>公式为：```2 * OptrCPUCost * SortFields * max( 2, Records ) * log2( max( 2, Records ) )``` |
| StartCost       | 数组   | 估算的 SORT 的启动代价<br>需要计算子操作的总代价和排序的代价<br>公式为：```ChildTotalCost + IOCPURate * IOCost + CPUCost``` |
| RunCost         | 数组   | 估算的 SORT 的运行代价（内部表示）<br>即从排序缓存中提取各个记录的代价<br>公式为：```OptrCPUCost * Records``` |
| TotalCost       | 数组   | 估算的 SORT 的总代价（内部表示）<br>公式为：```StartCost + RunCost``` |

**示例**

```
"SortNode": {
  "Records": 100000,
  "RecordTotalSize": 2900000,
  "Pages": 708,
  "SortFields": 1,
  "SortType": "InMemory",
  "IOCost": 0,
  "CPUCost": [
    "2 * OptrCPUCost * SortFields * max( 2, Records ) * log2( max( 2, Records ) )",
    "2 * 1 * 1 * max( 2, 100000 ) * log2( max( 2, 100000 ) )",
    3321929
  ],
  "StartCost": [
    "ChildTotalCost + IOCPURate * IOCost + CPUCost",
    "2768000 + 2000 * 0 + 3321929",
    6089929
  ],
  "RunCost": [
    "OptrCPUCost * Records",
    "1 * 100000",
    100000
  ],
  "TotalCost": [
    "StartCost + RunCost",
    "6089929 + 100000",
    6189929
  ]
}
```

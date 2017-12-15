使用 [SdbQuery.explain\(\)](reference/Sequoiadb_command/SdbQuery/explain.md) 可以查看查询的访问计划。

当 SdbQuery.explain() 的 Detail 选项为 true 时，将会展示详细的访问计划。在协调节点和数据节点上展示的详细访问计划略有不同。

**协调节点上的详细访问计划**

协调节点上的详细访问计划包括以下内容：

1.  协调节点上的访问计划信息
2.  涉及数据节点上的访问计划信息

```
{
  { 协调节点的访问计划信息 },
  "PlanPath": {
    "Operator": "COORD-MERGE",
    { 协调节点查询上下文的访问计划信息 },
    "ChildOperators": [
      {
        { 数据节点的访问计划信息 },
        ...
      },
      ...
    ]
  }
}
```

**数据节点上的详细访问计划**

数据节点上的详细访问计划包括以下内容：

1.  数据节点上的访问计划信息
2.  访问计划的缓存使用情况
3.  涉及的垂直分区中主子表的访问计划信息

主表的详细访问计划：

```
{
  { 主表的访问计划信息 },
  "PlanPath": {
    "Operator": "MERGE",
    { 主表查询上下文的访问计划信息 },
    "ChildOperators": [
      {
        { 子表的访问计划信息 },
        ...
      },
      ...
    ]
  }
}
```

普通集合或者子表的详细访问计划：

```
{
  { 集合的访问计划信息 },
  "PlanPath": {
    { 查询上下文的访问计划信息 }
    ...
  }
}
```

###协调节点的访问计划###

协调节点上的访问计划包括以下信息：

| 字段名      | 类型      | 描述                                    |
| ----------- | --------- | --------------------------------------- |
| NodeName    | 字符串    | 访问计划所在的节点的名称                |
| GroupName   | 字符串    | 访问计划所在的节点属于的复制组的名称    |
| Role        | 字符串    | 访问计划所在的节点的角色<br>"coord" 表示协调节点 |
| Collection  | 字符串    | 访问计划访问的集合的名称                |
| Query       | BSON 对象 | 访问计划解析后的用户查询条件            |
| Sort        | BSON 对象 | 访问计划中的排序字段                    |
| Selector    | BSON 对象 | 访问计划执行的选择符                    |
| Hint        | BSON 对象 | 访问计划中指定查询使用索引的情况        |
| Skip        | 长整型    | 访问计划需要跳过的记录个数              |
| Limit       | 长整型    | 访问计划最多返回的记录个数              |
| Flags       | 整型      | 访问计划中指定的执行标志，默认值为 0    |
| ReturnNum   | 长整型    | 访问计划返回记录的个数                  |
| ElapsedTime | 浮点型    | 访问计划查询耗时（单位：秒）            |
| IndexRead   | 长整型    | 访问计划扫描索引记录的个数              |
| DataRead    | 长整型    | 访问计划扫描数据记录的个数              |
| UserCPU     | 浮点型    | 访问计划用户态 CPU 使用时间（单位：秒） |
| SysCPU      | 浮点型    | 访问计划内核态 CPU 使用时间（单位：秒） |
| PlanPath    | BSON 对象 | 访问计划的具体执行操作 [COORD-MERGE](database_management/access_plans/detailed_access_plan.md#COORD-MERGE的信息) |

**示例**

```
{
  "NodeName": "hostname:11810",
  "GroupName": "SYSCoord",
  "Role": "coord",
  "Collection": "foo.bar",
  "Query": {
    "a": 1
  },
  "Sort": {},
  "Selector": {},
  "Hint": {
    "": ""
  },
  "Skip": 0,
  "Return": -1,
  "Flag": 0,
  "ReturnNum": 10,
  "ElapsedTime": 0.050839,
  "IndexRead": 0,
  "DataRead": 0,
  "UserCPU": 0,
  "SysCPU": 0,
  "PlanPath": {
    "Operator": "COORD-MERGE",
    ...
  }
}
```

###主表的访问计划###

垂直分区中的主表可以展示详细的访问计划，主表的访问计划包括以下信息：

| 字段名      | 类型      | 描述                                    |
| ----------- | --------- | --------------------------------------- |
| NodeName    | 字符串    | 访问计划所在的节点的名称                |
| GroupName   | 字符串    | 访问计划所在的节点属于的复制组的名称    |
| Role        | 字符串    | 访问计划所在的节点的角色<br>"data" 表示协调节点 |
| Collection  | 字符串    | 访问计划访问的集合的名称                |
| Query       | BSON 对象 | 访问计划解析后的用户查询条件            |
| Sort        | BSON 对象 | 访问计划中的排序字段                    |
| Selector    | BSON 对象 | 访问计划执行的选择符                    |
| Hint        | BSON 对象 | 访问计划中指定查询使用索引的情况        |
| Skip        | 长整型    | 访问计划需要跳过的记录个数              |
| Limit       | 长整型    | 访问计划最多返回的记录个数              |
| Flags       | 整型      | 访问计划中指定的执行标志，默认值为 0    |
| ReturnNum   | 长整型    | 访问计划返回记录的个数                  |
| ElapsedTime | 浮点型    | 访问计划查询耗时（单位：秒）            |
| IndexRead   | 长整型    | 访问计划扫描索引记录的个数              |
| DataRead    | 长整型    | 访问计划扫描数据记录的个数              |
| UserCPU     | 浮点型    | 访问计划用户态 CPU 使用时间（单位：秒） |
| SysCPU      | 浮点型    | 访问计划内核态 CPU 使用时间（单位：秒） |
| PlanPath    | BSON 对象 | 访问计划的具体执行操作 [MERGE](database_management/access_plans/detailed_access_plan.md#MERGE的信息) |

**示例**

```
{
  "NodeName": "hostname:11820",
  "GroupName": "group",
  "Role": "data",
  "Collection": "maincs.maincl",
  "Query": {},
  "Sort": {
    "a": 1
  },
  "Selector": {},
  "Hint": {},
  "Skip": 0,
  "Return": -1,
  "Flag": 2048,
  "ReturnNum": 50000,
  "ElapsedTime": 1.225226,
  "IndexRead": 0,
  "DataRead": 50000,
  "UserCPU": 0.5399999999999991,
  "SysCPU": 0.02000000000000002,
  "PlanPath": {
    "Operator": "MERGE",
    ...
  }
}
```

###数据节点的访问计划###

数据节点上的访问计划（包括垂直分区中的子表）包括以下信息：

| 字段名                    | 类型      | 描述                                    |
| ------------------------- | --------- | --------------------------------------- |
| NodeName                  | 字符串    | 访问计划所在的节点的名称                |
| GroupName                 | 字符串    | 访问计划所在的节点属于的复制组的名称    |
| Role                      | 字符串    | 访问计划所在的节点的角色<br>"data" 表示数据节点 |
| Collection                | 字符串    | 访问计划访问的集合的名称                |
| Query                     | BSON 对象 | 访问计划解析后的用户查询条件            |
| Sort                      | BSON 对象 | 访问计划中的排序字段                    |
| Selector                  | BSON 对象 | 访问计划执行的选择符                    |
| Hint                      | BSON 对象 | 访问计划中指定查询使用索引的情况        |
| Skip                      | 长整型    | 访问计划需要跳过的记录个数              |
| Limit                     | 长整型    | 访问计划最多返回的记录个数              |
| Flags                     | 整型      | 访问计划中指定的执行标志，默认值为 0    |
| ReturnNum                 | 长整型    | 访问计划返回记录的个数                  |
| ElapsedTime               | 浮点型    | 访问计划查询耗时（单位：秒）            |
| IndexRead                 | 长整型    | 访问计划扫描索引记录的个数              |
| DataRead                  | 长整型    | 访问计划扫描数据记录的个数              |
| UserCPU                   | 浮点型    | 访问计划用户态 CPU 使用时间（单位：秒） |
| SysCPU                    | 浮点型    | 访问计划内核态 CPU 使用时间（单位：秒） |
| CacheStatus               | 字符串    | 访问计划的缓存状态：<br>1. "NoCache" 为没有加入缓存<br>2. "NewCache" 为新建的缓存<br>3. "HitCache" 为命中的缓存 |
| IsMainCLPlan              | 布尔型    | 访问计划是否主表共享的查询计划          |
| CacheLevel                | 字符串    | 访问计划的缓存级别：<br>1. "OPT_PLAN_NOCACHE" 为不进行缓存<br>2. "OPT_PLAN_ORIGINAL" 为缓存原查询计划<br>3. "OPT_PLAN_NORMALZIED" 为缓存泛化后的查询计划<br>4. "OPT_PLAN_PARAMETERIZED" 为缓存参数化的查询计划<br>"OPT_PLAN_FUZZYOPTR" 为缓存参数化并带操作符模糊匹配的查询计划 |
| Parameters                | 数组类型  | 参数化的访问计划使用的参数列表          |
| MatchConfig               | BSON 对象 | 访问计划中的匹配符的配置                |
| MatchConfig.EnableMixCmp  | 布尔型    | 访问计划的匹配符是否使用混合匹配模式    |
| MatchConfig.Parameterized | 布尔型    | 访问计划的匹配符是否支持参数化          |
| MatchConfig.FuzzyOptr     | 布尔型    | 访问计划的匹配符是否支持模糊匹配        |
| PlanPath                  | BSON 对象 | 访问计划的具体执行操作 [SORT](database_management/access_plans/detailed_access_plan.md#SORT的信息)、[TBSCAN](database_management/access_plans/detailed_access_plan.md#TBSCAN的信息) 或 [IXSCAN](database_management/access_plans/detailed_access_plan.md#IXSCAN的信息) |
| Search                    | BSON 对象 | 查询计划优化器搜索过的访问计划<br>Search 选项为 true 时显示 |

**示例**

```
{
  "NodeName": "hostname:11820",
  "GroupName": "group",
  "Role": "data",
  "Collection": "foo.bar",
  "Query": {
    "a": {
      "$gt": 100
    }
  },
  "Sort": {},
  "Selector": {},
  "Hint": {},
  "Skip": 0,
  "Return": -1,
  "Flag": 2048,
  "ReturnNum": 0,
  "ElapsedTime": 0.000093,
  "IndexRead": 0,
  "DataRead": 0,
  "UserCPU": 0,
  "SysCPU": 0,
  "CacheStatus": "HitCache",
  "MainCLPlan": false,
  "CacheLevel": "OPT_PLAN_PARAMETERIZED",
  "Parameters": [
    100
  ],
  "MatchConfig": {
    "EnableMixCmp": false,
    "Parameterized": true,
    "FuzzyOptr": false
  },
  "PlanPath": {
    ...
  }
```

##详细的访问计划操作的信息##

详细的访问计划中有以下操作：

1.  [COORD-MERGE](database_management/access_plans/detailed_access_plan.md#COORD-MERGE的信息)：对应一个协调节点上的查询上下文对象。
2.  [MERGE](database_management/access_plans/detailed_access_plan.md#MERGE的信息)：对应一个数据节点上的主表查询上下文对象。
3.  [SORT](database_management/access_plans/detailed_access_plan.md#SORT的信息)：对应一个数据节点上的排序上下文对象。
4.  [TBSCAN](database_management/access_plans/detailed_access_plan.md#TBSCAN的信息)：对应一个数据节点上的全表扫描上下文对象。
5.  [IXSCAN](database_management/access_plans/detailed_access_plan.md#IXSCAN的信息)：对应一个数据节点上的索引扫描上下文对象。

###COORD-MERGE的信息###

详细的访问计划中，COORD-MERGE 对象对应一个协调节点上的查询上下文对象，其中展示的信息如下：

| 字段名                      | 类型      | 描述                                                                     |
| --------------------------- | --------- | ------------------------------------------------------------------------ |
| Operator                    | 字符串    | 操作符的名称： "COORD-MERGE"                                             |
| Sort                        | 字符串    | COORD-MERGE 需要保证输出结果有序的排序字段                               |
| NeedReorder                 | 布尔型    | COORD-MERGE 是否需要根据排序字段对多个数据组的记录进行排序合并           |
| DataNodeNum                 | 整型      | COORD-MERGE 涉及查询的数据节点个数                                       |
| DataNodeList                | 数组      | COORD-MERGE 涉及查询的数据节点，按查询的执行顺序列出                     |
| DataNodeList.Name           | 字符串    | COORD-MERGE 发送查询的数据节点名称                                       |
| DataNodeList.EstTotalCost   | 浮点数    | COORD-MERGE 发送的查询在该数据节点上查询的估算时间（单位：秒）           |
| DataNodeList.QueryTimeSpent | 浮点数    | 在数据节点上查询的执行时间（单位：秒）<br>Run 选项为 true 时显示         |
| DataNodeList.WaitTimeSpent  | 浮点数    | COORD-MERGE 发送的查询在数据节点上查询的等待时间（单位：秒）<br>Run 选项为 true 时显示 |
| Selector                    | BSON 对象 | COORD-MERGE 执行的选择符                                                 |
| Skip                        | 长整型    | 指定 COORD-MERGE 需要跳过的记录个数                                      |
| Return                      | 长整型    | 指定 COORD-MERGE 最多返回的记录个数                                      |
| Estimate                    | BSON 对象 | 估算的 COORD-MERGE 代价信息<br>Estimate 选项为 true 时显示               |
| Estimate.StartCost          | 浮点型    | 估算的 COORD-MERGE 的启动时间（单位：秒）                                |
| Estimate.RunCost            | 浮点型    | 估算的 COORD-MERGE的运行时间（单位：秒）                                 |
| Estimate.TotalCost          | 浮点型    | 估算的 COORD-MERGE 的结束时间（单位：秒）                                |
| Estimate.Output             | BSON 对象 | 估算的 COORD-MERGE 输出结果的统计信息<br>Filter 选项包含 "Output" 时显示 |
| Estimate.Output.Records     | 长整形    | 估算的 COORD-MERGE 输出的记录个数                                        |
| Estimate.Output.RecordSize  | 整型      | 估算的 COORD-MERGE 输出的记录平均字节数                                  |
| Estimate.Output.Sorted      | 布尔型    | COORD-MERGE 输出结果是否有序                                             |
| Run                         | BSON 对象 | 实际执行 COORD-MERGE 的代价信息<br>Run 选项为 true 时显示                |
| Run.ContextID               | 长整型    | COORD-MERGE 执行的上下文 ID                                              |
| Run.StartTimestamp          | 字符串    | COORD-MERGE 执行启动的时间戳                                             |
| Run.QueryTimeSpent          | 浮点型    | COORD-MERGE 执行耗时（单位：秒）                                         |
| Run.GetMoreWithData         | 长整型    | 请求 COORD-MERGE 返回结果集的次数                                        |
| Run.ReturnNum               | 长整型    | COORD-MERGE 返回记录个数                                                 |
| Run.WaitTimeSpent           | 浮点型    | COORD-MERGE 等待数据返回的时间（单位：秒，以秒为单位的粗略统计信息）     |
| ChildOperators              | 数组      | COORD-MERGE 的子操作（每个数据组返回的查询的访问计划结果）               |

**示例**

```
{
  ...,
  "PlanPath": {
    "Operator": "COORD-MERGE",
    "Sort": {},
    "NeedReorder": false,
    "DataNodeNum": 2,
    "DataNodeList": [
      {
        "Name": "hostname:11820",
        "EstTotalCost": 0.4750005,
        "QueryTimeSpent": 0.045813,
        "WaitTimeSpent": 0.000124
      },
      {
        "Name": "hostname:11830",
        "EstTotalCost": 0.4750005,
        "QueryTimeSpent": 0.045841,
        "WaitTimeSpent": 0.000108
      }
    ],
    "Selector": {},
    "Skip": 0,
    "Return": -1,
    "Estimate": {
      "StartCost": 0,
      "RunCost": 0.4750015,
      "TotalCost": 0.4750015,
      "Output": {
        "Records": 2,
        "RecordSize": 43,
        "Sorted": false
      }
    },
    "Run": {
      "ContextID": 9,
      "StartTimestamp": "2017-12-09-13.51.14.749863",
      "QueryTimeSpent": 0.046311,
      "GetMores": 3,
      "ReturnNum": 10,
      "WaitTimeSpent": 0
    },
    "ChildOperators": [
      {
        ...
      },
      {
        ...
      }
    ]
  }
}
```

###MERGE的信息###

详细的访问计划中，MERGE 对象对应一个数据节点上的主表查询上下文对象，其中展示的信息如下：

| 字段名                           | 类型      | 描述                                                                         |
| -------------------------------- | --------- | ---------------------------------------------------------------------------- |
| Operator                         | 字符串    | 操作符的名称： "MERGE"                                                       |
| Sort                             | 字符串    | MERGE 需要保证输出结果有序的排序字段                                         |
| NeedReorder                      | 布尔型    | MERGE 是否需要根据排序字段对多个子表的记录进行排序合并                       |
| SubCollectionNum                 | 整型      | MERGE 涉及查询的子表个数                                                     |
| SubCollectionList                | 数组      | MERGE 涉及查询的子表，按查询的执行顺序列出                                   |
| SubCollectionList.Name           | 字符串    | MERGE 发送查询的子表名称                                                     |
| SubCollectionList.EstTotalCost   | 浮点数    | MERGE 发送的查询在子表上查询的估算时间（单位：秒）                           |
| SubCollectionList.QueryTimeSpent | 浮点数    | MERGE 发送的查询在子表上查询的执行时间（单位：秒）<br>Run 选项为 true 时显示 |
| SubCollectionList.WaitTimeSpent  | 浮点数    | MERGE 发送的查询在数据节点上查询的等待时间（单位：秒）<br>Run 选项为 true 时显示） |
| Selector                         | BSON 对象 | MERGE 执行的选择符                                                           |
| Skip                             | 长整型    | 指定 MERGE 需要跳过的记录个数                                                |
| Limit                            | 长整型    | 指定 MERGE 最多返回的记录个数                                                |
| Estimate                         | BSON 对象 | 估算的 MERGE 代价信息<br>Estimate 选项为 true 时显示                         |
| Estimate.StartCost               | 浮点型    | 估算的 MERGE 的启动时间（单位：秒）                                          |
| Estimate.RunCost                 | 浮点型    | 估算的 MERGE 的运行时间（单位：秒）                                          |
| Estimate.TotalCost               | 浮点型    | 估算的 MERGE 的结束时间（单位：秒）                                          |
| Estimate.Output                  | BSON 对象 | 估算的 MERGE 输出结果的统计信息<br>Filter 选项包含 "Output" 时显示           |
| Estimate.Output.Records          | 长整形    | 估算的 MERGE 输出的记录个数                                                  |
| Estimate.Output.RecordSize       | 整型      | 估算的 MERGE 输出的记录平均字节数                                            |
| Estimate.Output.Sorted           | 布尔型    | MERGE 输出结果是否有序                                                       |
| Run                              | BSON 对象 | 实际执行 MERGE 的代价信息<br>Run 选项为 true 时显示                          |
| Run.ContextID                    | 长整型    | MERGE 执行的上下文 ID                                                        |
| Run.StartTimestamp               | 字符串    | MERGE 执行启动的时间戳                                                       |
| Run.QueryTimeSpent               | 浮点型    | MERGE 执行耗时（单位：秒）                                                   |
| Run.GetMores                     | 长整型    | 请求 MERGE 返回结果集的次数                                                  |
| Run.ReturnNum                    | 长整型    | MERGE 返回记录个数                                                           |
| SubCollections                   | 数组      | MERGE 的子操作（每个子表返回的查询的访问计划结果）                           |

**示例**

```
{
  ...,
  "PlanPath": {
    "Operator": "MERGE",
    "Sort": {
      "h": 1
    },
    "NeedReorder": true,
    "SubCollectionNum": 2,
    "SubCollectionList": [
      {
        "Name": "subcs.subcl1",
        "EstTotalCost": 0.8277414999999999,
        "QueryTimeSpent": 1.080046,
        "WaitTimeSpant": 0.000234
      },
      {
        "Name": "subcs.subcl2",
        "EstTotalCost": 0.8277414999999999,
        "QueryTimeSpent": 0.946832,
        "WaitTimeSpant": 0.000182
      }
    ],
    "Selector": {},
    "Skip": 0,
    "Return": -1,
    "Estimate": {
      "StartCost": 1.630483,
      "RunCost": 0.09999999999999999,
      "TotalCost": 1.730483,
      "Output": {
        "Records": 50000,
        "RecordSize": 43,
        "Sorted": true
      }
    },
    "Run": {
      "ContextID": 63121,
      "StartTimestamp": "2017-12-11-16.18.00.789234",
      "QueryTimeSpent": 1.203218,
      "GetMores": 3,
      "ReturnNum": 50000
    },
    "SubCollections": [
      {
        ...
      },
      {
        ...
      }
    ]
  }
}
```

###SORT的信息###

详细的访问计划中，SORT 对象对应一个数据节点上的排序上下文对象，其中展示的信息如下：

| 字段名                     | 类型      | 描述                                                                          |
| -------------------------- | --------- | ----------------------------------------------------------------------------- |
| Operator                   | 字符串    | 操作符的名称： "SORT"                                                         |
| Sort                       | BSON 对象 | SORT 执行的排序字段                                                           |
| Selector                   | BSON 对象 | SORT 执行的选择符                                                             |
| Skip                       | 长整型    | 指定 SORT 需要跳过的记录个数                                                  |
| Limit                      | 长整型    | 指定 SORT 最多返回的记录个数                                                  |
| Estimate                   | BSON 对象 | 估算的 SORT 代价信息<br>Estimate 选项为 true 时显示                           |
| Estimate.StartCost         | 浮点型    | 估算的 SORT 的启动时间（单位：秒）                                            |
| Estimate.RunCost           | 浮点型    | 估算的 SORT 的运行时间（单位：秒）                                            |
| Estimate.TotalCost         | 浮点型    | 估算的 SORT 的结束时间（单位：秒）                                            |
| Estimate.SortType          | 字符串    | SORT 估算的排序类型：<br>1. "InMemory" 为内存排序<br>2. "External" 为外存排序 |
| Estimate.Output            | BSON 对象 | 估算的 SORT 输出的统计信息<br>Filter 选项包含 "Output" 时显示                 |
| Estimate.Output.Records    | 长整形    | 估算的 SORT 输出的记录个数                                                    |
| Estimate.Output.RecordSize | 整型      | 估算的 SORT 输出的记录平均字节数                                              |
| Estimate.Output.Sorted     | 布尔型    | SORT 输出是否有序，对 SORT 为 true                                            |
| Run                        | BSON 对象 | 实际查询的 SORT 代价信息<br> Run 选项为 true 时显示                           |
| Run.ContextID              | 长整型    | SORT 执行的上下文 ID                                                          |
| Run.StartTimestamp         | 字符串    | SORT 启动的时间                                                               |
| Run.QueryTimeSpent         | 浮点型    | SORT 耗时（单位：秒）                                                         |
| Run.GetMores               | 长整型    | 请求 SORT 返回结果集的次数                                                    |
| Run.ReturnNum              | 长整型    | SORT 返回记录个数                                                             |
| Run.SortType               | 字符串    | SORT 执行的排序类型：<br>1. "InMemory" 为内存排序<br>2. "External" 为外存排序 |
| ChildOperators             | 数组      | SORT 的子操作（[TBSCAN](database_management/access_plans/detailed_access_plan.md#TBSCAN的信息) 或 [IXSCAN](database_management/access_plans/detailed_access_plan.md#IXSCAN的信息)） |

**示例**

```
{
  ...,
  "PlanPath": {
    "Operator": "SORT",
    "Sort": {
      "c": 1
    },
    "Selector": {},
    "Skip": 0,
    "Return": -1,
    "Estimate": {
      "StartCost": 0.475,
      "RunCost": 5e-7,
      "TotalCost": 0.4750005,
      "SortType": "InMemory",
      "Output": {
        "Records": 1,
        "RecordSize": 43,
        "Sorted": true
      }
    },
    "Run": {
      "ContextID": 8,
      "StartTimestamp": "2017-11-29-14.02.38.108504",
      "QueryTimeSpent": 0.050564,
      "GetMores": 1,
      "ReturnNum": 5,
      "SortType": "InMemory"
    },
    "ChildOperators": [
      {
        ...
      }
    ]
  }
}
```

###TBSCAN的信息###

详细的访问计划中，TBSCAN 对应一个使用全表扫描的上下文对象，展示的信息如下：

| 字段名                         | 类型      | 描述                                                            |
| ------------------------------ | --------- | --------------------------------------------------------------- |
| Operator                       | 字符串    | 操作符的名称： "TBSCAN"                                         |
| Collection                     | 字符串    | TBSCAN 访问的集合名字                                           |
| Query                          | BSON 对象 | TBSCAN 执行的匹配符                                             |
| Selector                       | BSON 对象 | TBSCAN 执行的选择符                                             |
| Skip                           | 长整型    | 指定 TBSCAN 需要跳过的记录个数                                  |
| Return                         | 长整型    | 指定 TBSCAN 最多返回的记录个数                                  |
| Estimate                       | BSON 对象 | 估算的 TBSCAN 代价信息<br>Estimate 选项为 true 时显示           |
| Estimate.StartCost             | 浮点型    | 估算的 TBSCAN 的启动时间（单位：秒）                            |
| Estimate.RunCost               | 浮点型    | 估算的 TBSCAN 的运行时间（单位：秒）                            |
| Estimate.TotalCost             | 浮点型    | 估算的 TBSCAN 的结束时间（单位：秒）                            |
| Estimate.CLEstFromStat         | 布尔型    | TBSCAN 是否使用集合的统计信息进行估算                           |
| Estimate.CLStatTime            | 字符串    | TBSCAN 使用的集合的统计信息的生成时间                           |
| Estimate.Input                 | BSON 对象 | 估算的 TBSCAN 输入的统计信息<br>Filter 选项包含 "Input" 时显示  |
| Estimate.Input.Pages           | 长整型    | 估算的 TBSCAN 输入的数据页数                                    |
| Estimate.Input.Records         | 长整形    | 估算的 TBSCAN 输入的记录个数                                    |
| Estimate.Input.RecordSize      | 整形      | 估算的 TBSCAN 输入的记录平均字节数                              |
| Estimate.Filter                | BSON 对象 | 估算的 TBSCAN 进行过滤的信息<br>Filter 选项包含 "Filter" 时显示 |
| Estimate.Filter.MthSelectivity | 浮点型    | 估算的 TBSCAN 使用匹配符进行过滤的选择率                        |
| Estimate.Output                | BSON 对象 | 估算的 TBSCAN 输出的统计信息<br>Filter 选项包含 "Output" 时显示 |
| Estimate.Output.Records        | 长整形    | 估算的 TBSCAN 输出的记录个数                                    |
| Estimate.Output.RecordSize     | 整型      | 估算的 TBSCAN 输出的记录平均字节数                              |
| Estimate.Output.Sorted         | 布尔型    | TBSCAN 输出是否有序，对 TBSCAN 为 false                         |
| Run                            | BSON 对象 | 实际执行 TBSCAN 的代价信息<br>Run 选项为 true 时显示            |
| Run.ContextID                  | 长整型    | TBSCAN 执行的上下文标识                                         |
| Run.StartTimestamp             | 字符串    | TBSCAN 执行启动的时间戳                                         |
| Run.QueryTimeSpent             | 浮点型    | TBSCAN 执行耗时（单位：秒）                                     |
| Run.GetMoreWithData            | 长整型    | 请求 TBSCAN 返回结果集的次数                                    |
| Run.ReturnNum                  | 长整型    | TBSCAN 返回记录个数                                             |
| Run.ReadRecords                | 长整型    | TBSCAN 扫描记录个数                                             |

**示例**

```
{
  ...,
  "PlanPath": {
    "Operator": "TBSCAN",
    "Collection": "foo.bar",
    "Query": {
      "$and": []
    },
    "Selector": {},
    "Skip": 0,
    "Return": -1,
    "Estimate": {
      "StartCost": 0,
      "RunCost": 0.45,
      "TotalCost": 0.45,
      "CLEstFromStat": false,
      "Input": {
        "Pages": 25,
        "Records": 25000,
        "RecordSize": 43
      },
      "Filter": {
        "MthSelectivity": 1
      },
      "Output": {
        "Records": 25000,
        "RecordSize": 43,
        "Sorted": false
      }
    },
    "Run": {
      "ContextID": 63123,
      "StartTimestamp": "2017-12-11-16.18.00.789831",
      "QueryTimeSpent": 0.040438,
      "GetMores": 25,
      "ReturnNum": 25000,
      "ReadRecords": 25000
    }
  }
}
```

###IXSCAN的信息###

详细的访问计划中，IXSCAN 对应一个使用索引扫描的上下文对象，展示的信息如下：

| 字段名                          | 类型      | 描述                                                                                       |
| ------------------------------- | --------- | ------------------------------------------------------------------------------------------ |
| Operator                        | 字符串    | 操作符的名称： "IXSCAN"                                                                    |
| Collection                      | 字符串    | IXSCAN 访问集合的名字                                                                      |
| Index                           | 字符串    | IXSCAN 访问索引的名字                                                                      |
| IXBound                         | BSON 对象 | IXSCAN 访问索引的查找范围                                                                  |
| Query                           | BSON 对象 | IXSCAN 执行的匹配符                                                                        |
| NeedMatch                       | 布尔型    | IXSCAN 是否需要在数据上执行匹配符进行过滤                                                  |
| Selector                        | BSON 对象 | IXSCAN 执行的选择符                                                                        |
| Skip                            | 长整型    | 指定 IXSCAN 需要跳过的记录个数                                                             |
| Limit                           | 长整型    | 指定 IXSCAN 最多返回的记录个数                                                             |
| Estimate                        | BSON 对象 | 估算的 IXSCAN 代价信息<br>Estimate 选项为 true 时显示                                      |
| Estimate.StartCost              | 浮点型    | 估算的 IXSCAN 的启动时间（单位：秒）                                                       |
| Estimate.RunCost                | 浮点型    | 估算的 IXSCAN 的运行时间（单位：秒）                                                       |
| Estimate.TotalCost              | 浮点型    | 估算的 IXSCAN 的结束时间（单位：秒）                                                       |
| Estimate.CLEstFromStat          | 布尔型    | IXSCAN 是否使用集合的统计信息进行估算                                                      |
| Estimate.CLStatTime             | 时间戳    | IXSCAN 使用的集合的统计信息的生成时间                                                      |
| Estimate.IXEstFromStat          | 布尔型    | IXSCAN 是否使用索引的统计信息进行估算                                                      |
| Estimate.IXStatTime             | 时间戳    | IXSCAN 使用的索引的统计信息的生成时间                                                      |
| Estimate.Input                  | BSON 对象 | 估算的 IXSCAN 输入的统计信息<br>Filter 选项包含 "Input" 时显示                             |
| Estimate.Input.Pages            | 长整型    | 估算的 IXSCAN 输入的数据页数                                                               |
| Estimate.Input.Records          | 长整形    | 估算的 IXSCAN 输入的记录个数                                                               |
| Estimate.Input.RecordSize       | 整形      | 估算的 IXSCAN 输入的记录平均字节数                                                         |
| Estimate.Filter                 | BSON 对象 | 估算的 IXSCAN 进行过滤的信息<br>Filter 选项包含 "Filter" 时显示                            |
| Estimate.Filter.MthSelectivity  | 浮点型    | 估算的 IXSCAN 使用匹配符进行过滤的选择率                                                   |
| Estimate.Filter.ScanSelectivity | 浮点型    | 估算的 IXSCAN 使用索引时需要扫描索引的比例                                                 |
| Estimate.Filter.PredSelectivity | 浮点型    | 估算的 IXSCAN 使用索引进行过滤的选择率                                                     |
| Estimate.Output                 | BSON 对象 | 估算的 IXSCAN 输出的统计信息<br>Filter 选项包含 "Output" 时显示                            |
| Estimate.Output.Records         | 长整形    | 估算的 IXSCAN 输出的记录个数                                                               |
| Estimate.Output.RecordSize      | 整型      | 估算的 IXSCAN 输出的记录平均字节数                                                         |
| Estimate.Output.Sorted          | 布尔型    | IXSCAN 输出是否有序<br>如果索引包含 Sort 的所有字段并且匹配顺序，该项为 true，否则为 false |
| Run                             | BSON 对象 | 实际查询的 IXSCAN 代价信息<br>Run 选项为 true 时显示                                       |
| Run.ContextID                   | 长整型    | IXSCAN 执行的上下文 ID                                                                     |
| Run.StartTimestamp              | 字符串    | IXSCAN 启动的时间                                                                          |
| Run.QueryTimeSpent              | 浮点型    | IXSCAN 耗时（单位：秒）                                                                    |
| Run.GetMores                    | 长整型    | 请求 IXSCAN 返回结果集的次数                                                               |
| Run.ReturnNum                   | 长整型    | IXSCAN 返回记录个数                                                                        |
| Run.ReadRecords                 | 长整型    | IXSCAN 扫描数据记录个数                                                                    |
| Run.IndexReadRecords            | 长整型    | IXSCAN 扫描索引项个数                                                                      |

**示例**

```
{
  ...,
  "PlanPath": {
    "Operator": "IXSCAN",
    "Collection": "foo.bar",
    "Index": "index",
    "IXBound": {
      "a": [
        [
          1,
          1
        ]
      ]
    },
    "Query": {
      "$and": [
        {
          "a": {
            "$et": 1
          }
        }
      ]
    },
    "NeedMatch": false,
    "Selector": {},
    "Skip": 0,
    "Return": -1,
    "Estimate": {
      "StartCost": 5e-7,
      "RunCost": 0.3200035,
      "TotalCost": 0.320004,
      "CLEstFromStat": false,
      "IXEstFromStat": false,
      "Input": {
        "Pages": 25,
        "Records": 25000,
        "RecordSize": 43,
        "IndexPages": 15
      },
      "Filter": {
        "MthSelectivity": 0.00004,
        "IXScanSelectivity": 0.00004,
        "IXPredSelectivity": 0.00004,
      },
      "Output": {
        "Records": 1,
        "RecordSize": 43,
        "Sorted": false
      }
    },
    "Run": {
      "ContextID": 36136,
      "StartTimestamp": "2017-12-11-16.11.34.518111",
      "QueryTimeSpent": 0.935198,
      "GetMores": 1,
      "ReturnNum": 5,
      "ReadRecords": 5,
      "IndexReadRecords": 6
    }
  }
}
```

##语法##

```
{ <字段名>: { $cast: <类型> } }
```

##说明##

将字段转化为指定类型。原始值为数组类型时对每个数组元素执行该操作。
其中指定的类型可以使用字符串表示(大小写不敏感)，也可以使用数字表示，如下表：

| 目标类型  | 字符串表示  | 数字表示 |
| --------- | ----------- | -------- |
| MinKey    | "minkey"    | -1       |
| Double    | "double"    | 1        |
| String    | "string"    | 2        |
| Object    | "object"    | 3        |
| ObjectId  | "oid"       | 7        |
| Bool      | "bool"      | 8        |
| Date      | "date"      | 9        |
| Null      | "null"      | 10       |
| Int32     | "int32"     | 16       |
| Timestamp | "timestamp" | 17       |
| Int64     | "int64"     | 18       |
| Decimal   | "decimal"   | 100      |
| MaxKey    | "maxkey"    | 127      |

转换关系如下：

* MinKey

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | ALL      | 任何类型都能转换成MinKey | 无 |

* Double

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | String   | 数字字符串: 对应的数值，其他: 0.0 | 0.0 |
  | Bool     | true: 1.0，false: 0.0 | 0.0 |
  | Int32    | | 0.0 |
  | Int64    | | 0.0 |
  | Decimal  | | 0.0 |

* String

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | Int32    | | null |
  | Int64    | | null |
  | Double   | | null |
  | Decimal  | | null |
  | Date     | | null |
  | Timestamp| | null |
  | Oid      | | null |
  | Object   | | null |
  | Array    | | null |
  | Bool     | | null |

* Object

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | String   | 标准Json     | null     |

* ObjectId

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | String   | 24个16进制字符组成 | null |

* Bool

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | Int32    | 0: false, 其他: true | 无 |
  | Int64    | 0: false, 其他: true | 无 |
  | Double   | 0: false, 其他: true | 无 |
  | Decimal  | 0: false, 其他: true | 无 |

* Date

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | Int32    | 绝对毫秒 | null |
  | Int64    | 绝对毫秒 | null |
  | Double   | 绝对毫秒 | null |
  | Decimal  | 绝对毫秒 | null |
  | String   | 形如“2015-08-19” | null |
  | Timestamp| | null |

  > **Note:**  
  > Date 类型字段转换为Int32类型可能会发生溢出。

* Null

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | ALL      | | 无 |

* Int32

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | String   | | 0 |
  | Bool     | | 0 |
  | Int64    | | 0 |
  | Double   | | 0 |
  | Decimal  | | 0 |
  | Timestamp| 绝对毫秒 | 0 |
  | Date     | 绝对毫秒 | 0 |

* Timestamp

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | Int32    | 绝对毫秒 | null |
  | Int64    | 绝对毫秒 | null |
  | Double   | 绝对毫秒 | null |
  | Decimal  | 绝对毫秒 | null |
  | String   | 形如“2015-08-19-17.59.05.918488” | null |
  | Date     | | null |

  > **Note:**  
  > Timestamp 类型字段转换为Int32类型可能会发生溢出。

* Int64

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | String   | | 0 |
  | Bool     | | 0 |
  | Int32    | | 0 |
  | Double   | | 0 |
  | Decimal  | | 0 |
  | Timestamp| 绝对毫秒 | 0 |
  | Date     | 绝对毫秒 | 0 |

* Decimal

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | String   | 数字字符串: 对应的数值，其他: 0 | 0 |
  | Bool     | true: 1, false: 0 | 0 |
  | Int32    | | 0 |
  | Int64    | | 0 |
  | Double   | | 0 |

* MaxKey

  | 源类型   | 转换格式备注 | 异常返回 |
  | -------- | ------------ | -------- |
  | ALL      | 任何类型都能转换成MaxKey | 无 |

##示例##

在集合 foo.bar 插入1条记录：

```lang-javascript 
> db.foo.bar.insert( { "a": "123" } )
```

> **Note:**  
> 字段“a”的值是字符串

SequoiaDB shell 运行如下：

* 作为选择符使用，返回字段“a”转换“int32”类型之后的记录：

  ```lang-javascript
  > db.foo.bar.find( {}, { "a": { "$cast": "int32" } } )
  {
      "_id": {
        "$oid": "582527402b4c38286d000019"
      },
      "a": 123
  }
  Return 1 row(s).
  ```

  > **Note:**  
  > 返回的结果为int32类型。

* 与匹配符配合使用，匹配字段“a”转换为“int32”之后值为123的记录：

  ```lang-javascript
  > db.foo.bar.find( { a: { $cast: "int32", $et: 123 } } )
  {
      "_id": {
        "$oid": "582527402b4c38286d000019"
      },
      "a": 123
  }
  Return 1 row(s).
  ```

  > **Note:**  
  > 返回原始记录，字段“a”的值是字符串类型。






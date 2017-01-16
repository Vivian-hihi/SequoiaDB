##语法##

```
{ <字段名>: { $slice: <值> } }
```

##说明##

截取数组中的子数组。字段非数组时，返回原值。

##示例##

在集合 foo.bar 插入1条记录：

```lang-javascript 
> db.foo.bar.insert( { "a": [ 1, 2, 3, 4, 5 ] } )
```

SequoiaDB shell 运行如下：

1. 作为选择符使用：

  返回下标为0，长度为2的子数组：

  ```lang-javascript
  > db.foo.bar.find( {}, { "a": { "$slice": 2 } } )
  {
      "_id": {
        "$oid": "58257889ec5c9b3b7e000000"
      },
      "a": [
        1,
        2
      ]
  }
  Return 1 row(s).
  ```

  返回倒数第二个元素开始的子数组：

  ```lang-javascript
  > db.foo.bar.find( {}, { "a": { "$slice": -2 } } )
  {
      "_id": {
        "$oid": "58257889ec5c9b3b7e000000"
      },
      "a": [
        4,
        5
      ]
  }
  Return 1 row(s).
  ```

  返回下标为2，长度为3的子数组：
  
  ```lang-javascript
  > db.foo.bar.find( {}, { "a": { "$slice": [ 2, 3 ] } } )
  {
      "_id": {
        "$oid": "58257889ec5c9b3b7e000000"
      },
      "a": [
        3,
        4,
        5
      ]
  }
  Return 1 row(s).
  ```

  返回倒数第二个元素开始，长度为3的子数组：
  
  ```lang-javascript
  > db.foo.bar.find( {}, { "a": { "$slice": [ -2, 3 ] } } )
  {
      "_id": {
        "$oid": "58257889ec5c9b3b7e000000"
      },
      "a": [
        4,
        5
      ]
  }
  Return 1 row(s).
  ```

  > **Note:**  
  > 子串长度不足3。

2. 与匹配符配合使用：

  匹配字段“a”下标为2，长度为1的子数组为[3]的记录：

  ```lang-javascript
  > db.foo.bar.find( { "a": { "$slice": [ 2, 1 ], "$et": [ 3 ] } } )
  {
      "_id": {
        "$oid": "58257889ec5c9b3b7e000000"
      },
      "a": [
        1,
        2,
        3,
        4,
        5
      ]
  }
  Return 1 row(s).
  ```



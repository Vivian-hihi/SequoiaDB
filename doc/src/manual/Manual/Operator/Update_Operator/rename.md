## 语法

```lang-json
{ $rename: { <字段名1>:<字段名2>,<字段名3>:<字段名4>, ... } }
```

## 描述

$rename 操作是将指定的 “<字段名1>” 重命名为指定的 “<字段名2>”。

## 示例

* 集合 sample.employee 存在如下记录：

 ```lang-json
 { "a": 5,"b": 10}
 ```

 将字段名 a 重命名为字段名 c：

 ```lang-javascript
 > db.sample.employee.update({$rename:{'a':'c'}})
 ```

 此操作后，记录更新为：

 ```lang-json
 { "c": 5,"b": 10}
 ```

* 集合 sample.employee 存在如下记录：

 ```lang-json
 { "a": 5,"b": 10}
 ```

 将字段名 a 重命名为字段名 b：

 ```lang-javascript
 > db.sample.employee.update({$rename:{'a':'b'}})
 ```

 此操作后，记录更新为：

 ```lang-json
 { "b": 5 }
 ```

 >**Note**
 >
 > json 格式中存在相同名字的列只显示一个，因此另一个字段 b 不显示，但服务端仍然存在。

* 集合 sample.employee 存在如下记录：

 ```lang-json
 { "a": 5,"b": 10}
 ```

 将字段名 a 重命名为字段名 b,将字段名 b 重命名为字段名 c：

 ```lang-javascript
 > db.sample.employee.update({$rename:{'a':'b'}})
 ```

 此操作后，记录更新为：

 ```lang-json
 { "a": 10,"b": 5}
 ```
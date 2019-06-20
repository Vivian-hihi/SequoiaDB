##语法##

***System.addGroup( \<groupObj\> )***

##类别##

System

##描述##

添加用户组

##参数##

| 参数名  | 参数类型 | 默认值       | 描述             | 是否必填 |
| ------- | -------- | ------------ | ---------------- | -------- |
| groupObj     | JSON   | ---          | 用户组信息       | 是       |

groupObj 参数详细说明如下：

| 属性     | 值类型 | 是否<br>必填 | 格式 | 描述 |
| -------- | ------ | -------- | -------------------- | ----------------- |
| name    | string |     是   | { "name": newGroup }     | 用户组名      |


##返回值##

无返回值。

##错误##

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 增加一个用户组

  ```lang-javascript
  > System.addGroup( { "name": "newGroup" } )
  ```
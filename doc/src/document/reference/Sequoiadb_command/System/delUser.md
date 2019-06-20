##语法##

***System.delUser( \<userObj\> )***

##类别##

System

##描述##

删除系统用户

##参数##

| 参数名  | 参数类型 | 默认值       | 描述             | 是否必填 |
| ------- | -------- | ------------ | ---------------- | -------- |
| userObj     | JSON   | ---          | 用户信息       | 是       |

userObj 参数详细说明如下：

| 属性     | 值类型 | 是否<br>必填 | 格式 | 描述 |
| -------- | ------ | -------- | -------------------- | ---------------------------------- |
| name    | string |     是   | { "name": newUser }     | 用户名                        |
| group    | string |     否   | { "group": newUser }     | 用户组                        |


##返回值##

无返回值。

##错误##

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 删除系统用户

  ```lang-javascript
  > System.delUser( { "name": "newUser2" } )
  ```
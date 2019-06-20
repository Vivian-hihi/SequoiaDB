##语法##

***System.delGroup( \<name\> )***

##类别##

System

##描述##

删除系统用户组

##参数##

| 参数名  | 参数类型 | 默认值       | 描述             | 是否必填 |
| ------- | -------- | ------------ | ---------------- | -------- |
| name     | string   | ---          | 用户组名       | 是       |

##返回值##

无返回值。

##错误##

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 删除一个用户组

  ```lang-javascript
  > System.delGroup( "newGroup" )
  ```
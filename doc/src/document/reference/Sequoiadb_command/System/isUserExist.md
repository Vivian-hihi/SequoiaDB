##语法##

***System.isUserExist( \<name\> )***

##类别##

System

##描述##

判断指定用户是否存在

##参数##

| 参数名    | 参数类型 | 默认值 | 描述         | 是否必填 |
| --------- | -------- | ------ | ------------ | -------- |
| name  | string   | ---    | 用户名     | 是       |

##返回值##

存在指定用户返回true，否则返回false

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 判断用户是否存在：

  ```lang-javascript
  > System.isUserExist("root" )
  true
  ```
##语法##

***System.sniffPort( \<port\> )***

##类别##

System

##描述##

判断指定端口是否可用

##参数##

| 参数名    | 参数类型 | 默认值 | 描述         | 是否必填 |
| --------- | -------- | ------ | ------------ | -------- |
| port  | int   | ---    | 端口号     | 是       |

##返回值##

指定端口可用返回true，否则返回false

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 测试一个端口是否可用

  ```lang-javascript
  > System.sniffPort(50000)
  {
  	"Usable": false
  }
  ```
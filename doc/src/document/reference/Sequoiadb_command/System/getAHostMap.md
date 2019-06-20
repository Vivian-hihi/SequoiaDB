##语法##

***System.getAHostMap( \<hostname\> )***

##类别##

System

##描述##

获得指定主机名在host文件中对应的IP地址

##参数##

| 参数名    | 参数类型 | 默认值 | 描述         | 是否必填 |
| --------- | -------- | ------ | ------------ | -------- |
| hostname  | string   | ---    | 主机名称     | 是       |

##返回值##

如果有则返回对应的IP地址，否则出错

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获得指定主机名在host文件中对应的IP地址

  ```lang-javascript
  > System.getAHostMap( "localhost" )
  127.0.0.1
  ```
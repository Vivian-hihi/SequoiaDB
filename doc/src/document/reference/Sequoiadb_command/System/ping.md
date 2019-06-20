##语法##

***System.ping( \<hostname\> )***

##类别##

System

##描述##

判断到达该主机的网络是否连通

##参数##

| 参数名    | 参数类型 | 默认值 | 描述         | 是否必填 |
| --------- | -------- | ------ | ------------ | -------- |
| hostname  | string   | ---    | 主机名称     | 是       |

##返回值##

能连通返回true，否则返回false

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 测试到达一个主机的网络是否连通

  ```lang-javascript
  > System.ping("hostname")
  {
    "Target": "hostname",
    "Reachable": true
  }
  ```
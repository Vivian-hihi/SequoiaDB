##语法##

***System.addAHostMap( \<hostname\>, \<ip\>, \[isReplace\] )***

##类别##

System

##描述##

往host文件添加一条主机到IP地址的映射关系

##参数##

| 参数名  | 参数类型 | 默认值       | 描述             | 是否必填 |
| ------- | -------- | ------------ | ---------------- | -------- |
| hostname     | string   | ---          | 主机名       | 是       |
| ip     | string   | ---          | IP地址     | 是       |
| replace | boolean  | true        | 是否替换映射关系 | 否       |

##返回值##

无返回值。

##错误##

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 添加一条映射关系

  ```lang-javascript
  > System.addAHostMap( "hostname", "1.1.1.1" )
  ```
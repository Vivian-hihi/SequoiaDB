##语法##

***System.delAHostMap( \<hostname\> )***

##类别##

System

##描述##

删除host文件中的一条指定主机的映射关系

##参数##

| 参数名  | 参数类型 | 默认值       | 描述             | 是否必填 |
| ------- | -------- | ------------ | ---------------- | -------- |
| hostname     | string   | ---          | 主机名       | 是       |

##返回值##

无返回值。

##错误##

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 删除一条映射关系

  ```lang-javascript
  > System.delAHostMap( "hostname" )
  ```
##语法##

***System.setProcUlimitConfigs( \<configsObj\> )***

##类别##

System

##描述##

修改进程资源限制值

##参数##

| 参数名  | 参数类型 | 默认值       | 描述             | 是否必填 |
| ------- | -------- | ------------ | ---------------- | -------- |
| configsObj  | JSON   | ---    | 新的限制值    | 是       |

options 参数见[getProcUlimitConfigs](reference/Sequoiadb_command/System/getProcUlimitConfigs.md)

##返回值##

无

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 修改进程最大内存大小

  ```lang-javascript
  > System.setProcUlimitConfigs( { "max_memory_size": -1, "open_files": 1024} )
  ```
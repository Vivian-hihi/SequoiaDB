##语法##

***System.isProcExist( \<optionObj\> )***

##类别##

System

##描述##

判断指定进程是否存在

##参数##

| 参数名    | 参数类型 | 默认值 | 描述         | 是否必填 |
| --------- | -------- | ------ | ------------ | -------- |
| optionObj  | JSON   | ---    | 进程信息     | 是       |

options 参数详细说明如下：

| 属性     | 值类型 | 是否<br>必填 | 格式 | 描述 |
| -------- | ------ | -------- | -------------------- | ----------------- |
| value    | string |     是   | { value: "31831" }     | 指定类型的值     |
| type    | string |     否   | { type: "pid" }     | 查询类型      |

##返回值##

存在指定进程返回true，否则返回false

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 通过指定pid判断：

  ```lang-javascript
  > System.isProcExist( { value: "31831", type: "pid" } )
  true
  ```

* 通过指定服务名判断：

  ```lang-javascript
  > System.isProcExist( { value: "sdbcm(11790)", type: "name" } )
  true
  ```
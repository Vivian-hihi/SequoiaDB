##语法##

***System.snapshotCpuInfo()***

##类别##

System

##描述##

获取CPU的基本信息

##参数##

无

##返回值##

返回CPU的基本信息

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取CPU的基本信息

  ```lang-javascript
  > System.snapshotCpuInfo()
  {
    "User": 47223380,
    "Sys": 46662920,
    "Idle": 3513293040,
    "Other": 3023840
  }
  ```
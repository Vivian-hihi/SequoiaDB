##语法##

***System.getReleaseInfo()***

##类别##

System

##描述##

操作系统发行版本信息

##参数##

无

##返回值##

返回操作系统的发行版本信息

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取操作系统的发行版本信息

  ```lang-javascript
  > System.getReleaseInfo()
  {
    "Distributor": "Ubuntu",
    "Release": "16.04",
    "Description": "Ubuntu 16.04.6 LTS",
    "KernelRelease": "4.4.0-116-generic",
    "Bit": 64
  }
  ```
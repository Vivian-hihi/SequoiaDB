##语法##

***System.getMemInfo()***

##类别##

System

##描述##

获取内存信息

##参数##

无

##返回值##

返回内存信息

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取内存信息

  ```lang-javascript
  > System.getMemInfo()
  {
    "Size": 5967,
    "Used": 2919,
    "Free": 384,
    "Unit": "M"
  }
  ```
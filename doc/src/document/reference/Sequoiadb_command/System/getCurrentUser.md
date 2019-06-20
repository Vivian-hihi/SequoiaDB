##语法##

***System.getCurrentUser()***

##类别##

System

##描述##

获取当前用户信息

##参数##

无

##返回值##

返回当前用户信息

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取当前用户信息

  ```lang-javascript
  > System.getCurrentUser()
  {
    "user": "root",
    "gid": "0",
    "dir": "/root"
  }
  ```
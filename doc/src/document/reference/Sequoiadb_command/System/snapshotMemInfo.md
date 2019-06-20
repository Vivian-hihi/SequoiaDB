##语法##

***System.snapshotMemInfo()***

##类别##

System

##描述##

获取内存的一些基本信息

##参数##

无

##返回值##

返回内存的一些基本信息

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取内存的基本信息

	```lang-javascript
  > System.snapshotMemInfo()
  {
    "Size": 5967,
    "Used": 2919,
    "Free": 384,
    "Unit": "M"
  }
	```
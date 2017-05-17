##名称##

getLastErrMsg - 获取当前操作的详细错误信息。

##语法##

***getLastErrMsg()***

##类别##

Global

##描述##

获取当前操作的详细错误信息。

##参数##

无

##返回值##

当存在错误信息时，返回为 String 类型的描述信息；当不存在错误信息时，无返回值（即为 void）。

##版本信息##

v2.6及以上版本

##示例##

1. 获取当前操作的详细错误信息

  	```lang-javascript
  	> db = new Sdb()
  	(nofile):0 uncaught exception: -15
  	> getLastErrMsg()
  	Network error
  	```

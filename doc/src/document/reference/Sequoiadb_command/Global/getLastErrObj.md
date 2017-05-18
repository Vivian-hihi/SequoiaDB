##名称##

getLastErrObj - 获取当前操作的详细错误信息。

##语法##

**getLastErrObj()**

##类别##

Global

##描述##

获取当前操作的详细错误信息。

##参数##

无

##返回值##

当存在错误时，返回为bson对象，当不存在错误时，无返回值（即void）。
bson对象有3个固定的字段：

* errno: (Int32) 错误码
* description: (String) 错误码对应的描述
* detail: (String) 详细的错误描述信息

对于操作协调节点发生错误时，当某些数据节点发生错误时，还会有扩展字段：

* ErrNodes: [ bson object ] 描述发生错误的节点的详细信息。

##版本##

v2.8及以上版本。

##示例##

1. 获取当前操作的详细错误信息

  	```lang-javascript
  	> db = new Sdb()
  	(nofile):0 uncaught exception: -15
  	> getLastErrObj()
  	{
    	"errno": -15,
    	"description": "Network error",
    	"detail": ""
  	}
  ```

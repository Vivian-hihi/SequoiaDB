##名称##

getLastErrObj - 以 bson 对象的方式，返回前一次操作的详细错误信息。

##语法##

**getLastErrObj()**

##类别##

Global

##描述##

获取前一次操作的详细错误信息。

##参数##

无。

##返回值##

若前一次操作发生错误，该函数返回以 BSON 对象的形式返回错误信息。否则，无返回值（即void）。BSON 对象有3个固定的字段：

* errno: (Int32) 错误码。
* description: (String) 错误码对应的描述。
* detail: (String) 详细的错误描述信息。

当操作协调节点发生错误，若该错误由某些数据节点产生，会有扩展字段：

* ErrNodes: (BSON object) 描述发生错误的节点的详细信息。

##版本##

v2.6及以上版本。

##示例##

1. 获取前一次操作的详细错误信息。

  	```lang-javascript
  	> db = new Sdb()
  	(nofile):0 uncaught exception: -15
  	> var err = getLastErrObj()
	> var obj = err.toObj()
	> println( obj.toString() )
  	{
    	"errno": -15,
    	"description": "Network error",
    	"detail": ""
  	}
  ```

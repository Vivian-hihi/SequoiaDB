##NAME##

getLastErrObj - Get the error object of last operation.

##SYNOPSIS##

**getLastErrObj()**

##CATEGORY##

Global

##DESCRIPTION##

##PARAMETERS##

##RETURN VALUE##

##ERRORS##

##HISTORY##

Since v2.6.

##EXAMPLES##

1. 获取当前操作的详细错误信息

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
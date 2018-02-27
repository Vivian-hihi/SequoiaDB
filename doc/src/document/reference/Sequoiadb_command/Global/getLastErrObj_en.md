##NAME##

getLastErrObj - Get the error object of last operation.

##SYNOPSIS##

**getLastErrObj()**

##CATEGORY##

Global

##DESCRIPTION##

Get the error object of last operation.

##PARAMETERS##

NULL.

##RETURN VALUE##

The error object of last operation.

The error object contains the follow 3 fields:

* errno: (Int32) error code.
* description: (String) the description of error code.
* detail: (String) error detail.

When error happen in data nodes, the error object will also contains the follow fields:

* ErrNodes: The error detail of the data nodes.

##ERRORS##

NULL.

##HISTORY##

Since v2.6.

##EXAMPLES##

1. Get the error object of last operation.

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

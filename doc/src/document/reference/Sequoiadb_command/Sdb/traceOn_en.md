##NAME##

traceOn -  Turn on the database engine program tracking.

##SYNOPSIS##

***db.traceOn( \<bufferSize\>, [strComp], [strBreakPoint], [tids] )***

##CATEGORY##

Sdb

##DESCRIPTION##

Turn on the database engine program tracking.

##PARAMETERS##

| Name 		    | Type   | Description                                       | Required or not |
| ------------- | ------ | ------------------------------------------------- | --------------- |
| bufferSize    | int    | The size of the tracked file we turn on. Uint: MB. Ranges: [1,1024] | requiresd |
| strComp	    | string | The module we specified. Default to all modules.  | not 	       	   |
| strBreakPoint | string | Breake point                                      | not 		       |
| tids		    | array  | Specify one or multiple threads                   | not 		       |

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Turn on the database engine program tracking.

  > **Note:** Track only the node that db connected. 

```lang-javascript
> db.traceOn( 256 )
```

* Turn on the database engine program tracking and specify a tracked module and break point. 

```lang-javascript
> db.traceOn( 256, "cls, dms, mth", "_dmsTempSUMgr::init", 12712 )
```

* Or turn on the database engine program tracking and specify multiple threads.

```lang-javascript
> db.traceOn( 256, "cls, dms, mth", "_dmsTempSUMgr::init", [12712, 12713, 12714] )
```

* When the tracked module was blocked because of the breakpoint, you can use [traceResume()](reference/Sequoiadb_command/Sdb/traceResume.md) to wake up the module which was tracked and blocked. 

```lang-javascript
> db.traceResume()
```

* Using [traceStatus()](reference/Sequoiadb_command/Sdb/traceStatus.md) to view the tracking status of the current program. 

```lang-javascript
> db.traceStatus()
```

* Using [traceOff()](reference/Sequoiadb_command/Sdb/traceOff.md) to turn off the database engine program tracking and export tracking results to binary files.

```lang-javascript
> db.traceOff("/opt/sequoiadb/trace.dump")
```

* Using [traceFmt()](reference/Sequoiadb_command/Global/traceFmt.md) to analysis the binary file.

```lang-javascript
> traceFmt( 0, "/opt/sequoiadb/trace.dump", "/opt/sequoiadb/trace_output" )
```
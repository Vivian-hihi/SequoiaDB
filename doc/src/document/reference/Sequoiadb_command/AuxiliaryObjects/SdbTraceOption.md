指定 traceOn 监控参数。

包括指定模块、断点、线程号、函数以及线程类型等参数。

##语法##

**SdbTraceOption() [.components( \<component1\> [,component2...] )]
</br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.breakPoints( \<breakPoint1\> [,breakPoint2...] )]
</br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.tids( \<tid1\> [,tid2...] )]
</br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.functionNames( \<functionName1\> [,functionName2...] )]
</br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.threadTypes( \<threadType1\> [,threadType2...] )]**

**SdbTraceOption() [.components( [ \<component1\>, \<component2\>, ... ] )]
</br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.breakPoints( [ \<breakPoint1\>, \<breakPoint2\>, ... ] )]
</br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[ .tids( [ \<tid1\>, \<tid2\>, ... ] )]
</br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.functionNames( [ \<functionName1\>, \<functionName2\>, ... ] )]
</br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[.threadTypes( [ \<threadType1\>,  \<threadType2\>, ... ] )]**

##方法##

###components(\<component\>)###

模块方法

| 参数名       | 参数类型              | 默认值   | 描述     | 是否必填 |
| ------------ | --------------------- | -------- | -------- | -------- |
| conponent    | string / string array | 所有模块 | 指定模块 | 否       |

conponent 参数的可选值如下表：

| 可选值 | 描述                      | 
| ------ | ------------------------- | 
| auth   | Authentication            | 
| bps    | BufferPool Services       | 
| cat    | Catalog Services          | 
| cls    | Cluster Services          | 
| dps    | Data Protection Services  | 
| mig    | Migration Services        | 
| msg    | Messaging Services        | 
| net    | Network Services          | 
| oss    | Operating System Services | 
| pd     | Problem Determination     | 
| rtn    | RunTime                   | 
| sql    | SQL Parser                | 
| tools  | Tools                     | 
| bar    | Backup And Recovery       | 
| client | Client                    | 
| coord  | Coord Services            | 
| dms    | Data Management Services  | 
| ixm    | Index Management Services | 
| mon    | Monitoring Services       | 
| mth    | Methods Services          | 
| opt    | Optimizer                 | 
| pmd    | Process Model             | 
| rest   | RESTful Services          | 
| spt    | Scripting                 | 
| util   | Utilities                 | 

###breakPoints(\<breakPoint\>)###

断点方法

| 参数名       | 参数类型	           |  默认值   | 描述                    | 是否必填 |
| ------------ | --------------------- | --------- | ----------------------- | -------- |
| breakPoint   | string / string array | ---       |于函数处打断点进行跟踪   | 否       |

###tids(\<tid\>)###

线程方法

| 参数名       | 参数类型	           |  默认值  | 描述     | 是否必填 |
| ------------ | --------------------- | -------- | -------- | -------- |
| tid          | int / int array       | 所有线程 | 指定线程 | 否       |

###functionNames(\<functionName\>)###

函数方法

| 参数名       | 参数类型	           |  默认值   | 描述       | 是否必填 |
| ------------ | --------------------- | --------- | ---------- | -------- |
| functionName | string / string array | ---       | 指定函数名 | 否       |

###threadTypes(\<threadType\>)###

线程类型方法

| 参数名       | 参数类型	           | 默认值   | 描述       | 是否必填 |
| ------------ | --------------------- | -------- | ---------- | -------- |
| threadType   | string / string array | ---      | 指定线类型 | 否       |

threadType 参数的可选值详见[线程类型](database_management/EDU.md)

> **Note：**

>1. SdbTraceOption 可同时连续指定多个方法，其中模块方法的参数和函数方法的参数是并集关系，线程类型方法的参数和线程方法的参数也是并集关系；

>2. 当同时不指定 components 方法和 functionNames 方法，默认监控所有模块，但是如果指定 functionNames 方法，不指定 components 方法，则监控指定的函数；

>3. 以上各个方法中的参数最多只能指定 10 个参数。

##返回值##

返回自身，类型为 SdbTraceOption。

##错误##

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 开启监控程序

	```lang-javascript
    > db = new Sdb( "localhost", 50000 )
    > var option = new SdbTraceOption().component( "dms", "rtn" ).breakPoints( "_coordCMDEval::execute", "_dmsStorageUnit::insertRecord" ).tid( [ 15923, 35712 ] ).threadType( "RestListener", "LogWriter" ).functionName( "_coordCMDEval::execute", "_dmsStorageUnit::insertRecord" )
	> db.traceOn( 1000, option )
	```

* 以上 SdbTraceOption 各个方法的参数还可以以字符串数组的形式给出

	```lang-javascript
    > db = new Sdb( "localhost", 50000 )
    > var option = new SdbTraceOption().component( [ "dms", "rtn" ] ).breakPoints( [ "_coordCMDEval::execute", "_dmsStorageUnit::insertRecord" ] ).tid( [ 15923, 35712 ] ).threadType( [ "RestListener", "LogWriter" ] ).functionName( [ "_coordCMDEval::execute", "_dmsStorageUnit::insertRecord" ] )
	> db.traceOn( 1000, option )
	```

* 对于方法中存在多个参数的情况，可以通过多次调用该方法指定参数

	```lang-javascript
    > db = new Sdb( "localhost", 50000 )
    > var option = new SdbTraceOption().component( [ "dms", "rtn" ] ).component( [ "dps", "cls" ].component( "pd" ) )
	> db.traceOn( 1000, option )
	```


##语法##
***db.traceOn(&lt;bufferSize&gt;,[strComp],[strBreakPoint])***

开启数据库引擎跟踪功能。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| bufferSize | int | 开启追踪的文件大小。 | 是 |
| strComp | string | 指定模块。 | 否 |
| strBreakPoint | string | 于函数处打断点进行跟踪。 | 否 |

## 示例##

* 开启数据库引擎程序跟踪的功能，默认为所有模块：

<pre class="prettyprint lang-javascript">
> db.traceOn(10000000);</pre>

* 开户数据库引擎程序跟踪功能，指定跟踪的模块名称和指定断点进行跟踪：

<pre class="prettyprint lang-javascript">
> db.traceOn(10000000, "cls, dms, mth", "_dmsTempCB::init") ;</pre>

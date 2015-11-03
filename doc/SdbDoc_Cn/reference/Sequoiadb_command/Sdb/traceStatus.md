##语法##
***db.traceStatus()***

查看当前程序跟踪的状态。

## 参数描述

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| - | - | - | - |

## 示例##

* 查看当前程序跟踪的状态：

<pre class="prettyprint lang-javascript">
> db.traceStatus()
{
  "TraceStarted": true,
  "Wrapped": false,
  "Size": 524288,
  "Mask": 
  [
    "auth",
    "bps",
    "cat",
    "cls",
    "dps",
    "mig",
    "msg",
    "net",
    "oss",
    "pd",
    "rtn",
    "sql",
    "tools",
    "bar",
    "client",
    "coord",
    "dms",
    "ixm",
    "mon",
    "mth",
    "opt",
    "pmd",
    "rest",
    "spt",
    "util",
    "aggr",
    "spd",
    "qgm"
  ],
  "BreakPoint": []
}</pre>

##语法##
***db.traceOff(&lt;dumpFile&gt;)***

关闭数据库引擎跟踪功能，并将跟踪情况导出二进制文件，如：/opt/sequoiadb/trace.dump

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| dump file | string | dump 的文件名称| 是 |

## 示例##

* 关闭数据库引擎跟踪 /opt/sequoiadb/trace.dump

<pre class="prettyprint lang-javascript">
> db.traceOff("/opt/sequoiadb/trace.dump");</pre>

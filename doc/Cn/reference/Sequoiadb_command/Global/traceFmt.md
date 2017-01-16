##语法##
***traceFmt(&lt;formatType&gt;,&lt;input&gt;,&lt;output&gt;)***

将 db.traceOff() 导出来的二进制文件格式化为另外文件输出。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| formatType | int | 格式类型 | 是 |
| input | string | 输入文件 | 是 |
| output | string | 输出文件 | 是 |

##示例##

**格式化输出文件**：

<pre class="prettyprint lang-javascript">
> traceFmt(0, "/opt/sequoiadb/trace.dump", "/opt/sequoiadb/trace.flw")</pre>

##语法##
***db.cancelTask(&lt;id&gt;,[ isAsync ])***

取消任务

##参数描述##

| 参数名 | 参数类型 | 描述   | 是否必填 |
|--------|----------|--------|----------|
| id     | 整数     |任务ID  | 是       |
| isAsync| 布尔     |是否异步| 否       |

##示例##

**停止切分任务**
<pre class="prettyprint lang-javascript">
> var taskid1 = db.test.test.splitAsync("db1", "db2", 50);
> db.cancelTask( taskid1, true )</pre>

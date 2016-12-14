##语法##
***db.waitTasks(&lt;id1&gt;,[id2],...)***

同步等待指定任务结束或取消

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| id1, id2… | 整数 | 任务 ID | 是 |

## 示例##

* 同步等待数据切分任务完成

<pre class="prettyprint lang-javascript">
> var taskid1 = db.test.test.splitAsync("db1", "db2", 50);
> var taskid2 = db.my.my.splitAsync("db3", "db4", 50) ;
> db.waitTasks( taskid1, taskid2 )</pre>

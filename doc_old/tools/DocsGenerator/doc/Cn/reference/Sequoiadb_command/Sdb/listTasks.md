##语法##
***db.listTasks([cond],[sel],[orderBy],[hint])***

查看数据库所有后台任务

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| cond | Json 对象 | 任务过滤条件 | 否 |
| sel | Json 对象 | 任务选择字段 | 否 |
| hint | Json 对象 | 保留项 | 否 |

## 示例##

* 列出系统所有后台任务

<pre class="prettyprint lang-javascript">
> db.listTasks()</pre>

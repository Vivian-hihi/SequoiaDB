## 描述##

\$project 类似 SQL 中的 select 语句，通过使用 \$project 操作可以从记录中筛选出所需字段，字段名的值如果为1，表示选出，为0表示不选；还可以实现字段的重命名。

**Note:**

如果记录不存在所选字段，则以如下格式输出："field":null，field 为不存在的字段名。对嵌套对象使用点操作符（.）引用字段名。

## 示例##

* 使用 $project 快速地从结果集中选取所需字段

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({ $project : {title: 0,author: 1}})</pre>

此操作是选出 author 字段，而 title 字段在结果集中不输出。

* 使用 $project 重命名字段名，如下：

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({ $project : {author: 1,name:"$studentName",dep:"$info.department"}})</pre>

此操作将字段名 studentName 重命名为 name 输出，将 info 对象中的子对象 department 字段重命名为 dep。对嵌套对象，字段引用使用点操作符（.）指向。

* 下面的示例使用 $project 选择输出字段，然后使用 $match 按条件匹配记录

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({ $project: {score:1,author:1}},{$match:{score:{$gt:80}}})</pre>

此操作使用 \$project 输出所有记录的 score 和 author 字段值，然后按 $match 输出匹配条件的记录。

**Note:**

由于 \$project 选取了输出字段名，所以 $match 中字段名必须是 $project 中选出的字段名。

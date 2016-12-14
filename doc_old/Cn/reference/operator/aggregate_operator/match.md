## 描述##

\$match 与 find() 方法中的 cond 参数完全相同，通过 \$match 可以实现从集合中选择匹配条件的记录。

$match 的语法规则请参考读取操作 find() 方法的 cond 参数介绍。

## 示例

* 下面的示例使用 \$match 执行简单的匹配

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$match:{$and:[{score:80},{"info.name":{$exists:1}}]}})</pre>

该操作表示从集合 collection 中返回符合条件 score 等于80且 info 对象中的子对象 name 字段存在的记录。

* 下面的示例使用 \$match 匹配符合条件的记录，然后使用 \$group 对结果集分组，最后使用 $project 输出结果集中指定的字段名

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$match:{$and:[{score:80},{"info.name":{$exists:1}}]}},{$group:{_id:"$major"}},{$project:{major:1,dep:1}})</pre>

该操作首先集合 collection 中返回符合条件 score 等于80且 info 对象中的子对象 name 字段存在的记录，然后按 major 字段进行分组，最后选择输出结果集中的 major 和 dep 字段。

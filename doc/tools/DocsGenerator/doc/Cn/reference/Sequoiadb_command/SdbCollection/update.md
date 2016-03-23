##语法##
***db.collectionspace.collection.update(&lt;rule&gt;,[cond],[hint])***

更新集合记录。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| rule | Json 对象 | 更新规则。记录按 rule 的内容更新。 | 是 |
| cond | Json 对象 | 选择条件。为空时，更新所有记录，不为空时，更新符合条件的记录。 | 否 |
| hint | Json 对象 | 指定访问计划。 | 否 |

## 格式##

update() 方法的定义必须包含 rule 字段，rule 是一个 Json 对象。cond 和 hint 字段可选。hint 参数是一个包含一个单一字段的 Json 对象，字段名会被忽略，而其字段值则指定为需要访问索引的名称，当字段值为 null 时，则遍历集合中所有的记录，它的格式为{"":null}或者{"":"&lt;indexname&gt;"}。

<pre class="prettyprint lang-diy">
{&lt;{""更新符1"":{字段名1:"值"},"更新符2":{"字段名2":"值2"},...}&gt;,[{"字段名1":{"匹配符1":"值1"},"字段名2":{"匹配符2":"值2"},...}],[{"":"索引名"|null}]}</pre>

**Note:**

update 本版本暂不支持对表分区键（ShardingKey）字段更新，如果包含对分区键的更新操作，将自动剔除掉对分区键的更新，但其他字段更新生效，且不会发生错误。

## 示例##

* 按指定的更新规则更新集合中所有记录，即设置 rule 参数，不设定 cond 和 hint 参数的内容

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$inc:{age:1}})</pre>

此操作更新集合 bar 下的 age 字段，将 age 字段的值增加1。

* 选择符合匹配条件的记录，对这些记录按更新规则更新，即设定 rule 和 cond 参数

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{age:""}},{age:{$exists:1},name:{$exists:0}})</pre>

此操作更新集合 bar 中存在 age 字段而不存在 name 字段的记录，将这些记录的 age 字段删除。

* 按访问计划更新记录，假设集合中存在指定的索引名

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$inc:{age:1}},{age:{$gt:20}},{"":"testIndex"})></pre>

此操作使用索引名为 testIndex 的索引访问集合 bar 中 age 字段值大于20的记录，将这些记录的 age 字段名加1。

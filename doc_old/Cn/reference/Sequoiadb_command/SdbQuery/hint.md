##语法##
***query.hint(&lt;hint&gt;)***

按指定的索引遍历结果集。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| hint | Json 对象 | 指定访问计划，加快查询速度。 | 否 |

## 格式##

query.hint() 的方法定义包含 hint 参数，如果不设定 hint 参数的内容相当于不使用索引遍历结果集。hint 参数是一个包含一个单一字段的 Json 对象，字段名会被忽略，而其字段值则指定为需要访问索引的名称，当字段值为 null 时，则遍历集合中所有的记录。

格式如下：
<pre class="prettyprint lang-diy">
{"":null} 或者 {"":"&lt;indexname&gt;"}</pre>

##示例##

* 使用索引 ageIndex 遍历集合 bar 下存在 age 字段的记录，并返回。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$exists:1}}).hint({"":"ageIndex"})</pre>

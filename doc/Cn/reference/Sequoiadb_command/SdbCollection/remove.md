## 语法##
***db.collectionspace.collection.remove([cond],[hint])***

删除集合中的记录。

## 参数描述##
| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| cond | Json 对象 | 选择条件。为空时，删除所有记录，不为空时，删除符合条件的记录。 | 否 |
| hint | Json 对象 | 指定访问计划。 | 否 |

## 格式##

cond 参数是一个Json 的对象。当它的内容为空（例如{}）时，删除集合下所有的记录。hint 参数是一个包含一个单一元素的 Json 对象，该元素的字段名会被忽略，而其字段值则指定为需要访问索引的名称，当字段值为 null 时，则遍历集合中所有的记录。

<pre class="prettyprint lang-diy">
{[{"字段名1":{"匹配符1":"值1","字段名2":{"匹配符2":"值2"},...}],[{"":"索引名"|null}]}</pre>

## 示例##

* 删除集合所有记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.remove()</pre>

* 按访问计划删除匹配 cond 条件的记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.remove({age:{$gte:20}},{"":"myIndex"})</pre>

此操作按照索引名为“myIndex”的索引遍历集合中的记录，在遍历得到的记录中删除符合条件 age 字段值大于等于20的记录。

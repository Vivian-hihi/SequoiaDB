##语法##
***db.collectionspace.collection.dropIndex(&lt;name&gt;)***

删除集合中指定的索引。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 索引名，同一个集合中的索引名必须唯一。 | 是 |

## 格式##

dropIndex() 方法的定义格式必须包含 name 字段。其中 name 的值必须为字符串。
<pre class="prettyprint lang-diy">
{"name":"<索引名>"}</pre>

**Note:**

* 做删除索引操作时，索引名必须在集合中存在。
* 索引名不能是空串，含点（.）或者美元符号（$），且长度不超过127B。

## 示例

* 删除集合 bar 下名为 ageIndex 的索引，假设 ageIndex 已存在。

<pre class="prettyprint lang-javascript">
> db.foo.bar.dropIndex("ageIndex")</pre>

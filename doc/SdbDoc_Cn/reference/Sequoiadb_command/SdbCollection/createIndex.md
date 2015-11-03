## 语法##
***db.collectionspace.collection.createIndex(&lt;name&gt;,&lt;indexDef&gt;,[isUnique],[enforced])***

为集合创建索引，提高查询速度。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 索引名，同一个集合中的索引名必须唯一。 | 是 |
| indexDef | Json 对象 |  索引键，包含一个或多个指定索引字段与方向的对象。其中方向为1代表从小到大排序，-1则为从大到小排序。 | 是 |
| isUnique | Boolean | 索引是否唯一，默认 false。设置为 true 时代表该索引为唯一索引。 | 否 |
| enforced | Boolean | 索引是否强制唯一，可选参数，在 isUnique 为 true 时生效，默认 false。设置为 true 时代表该索引在 isUnique 为 true 的前提下，不可存在一个以上全空的索引键。 | 否 |

## 格式##

createIndex() 方法定义包含 name，indexDef，isUnique 三个参数， 其中 name 的值必须为字符串；indexDef 则为一个 JSON 对象，indexDef 的对象必须包含至少一个字段，其中字段名为用户需要索引的字段名，其值为1或者-1。其中1代表升序，-1代表降序；isUnique 为布尔类型，默认 false。

<pre class="prettyprint lang-diy">
{"name":"&lt;索引名&gt;","indexDef":{"&lt;索引字段1&gt;":&lt;1|-1&gt; [,"&lt;索引字段2&gt;":&lt;1|-1&gt;...] },["isUnique":&lt;true|false&gt;],["enforced":&lt;true|false&gt;]}</pre>

**Note:**

* 在唯一索引所指定的索引键字段上，集合中不可存在一条以上的记录完全重复
* 索引名不能是空串，含点（.）或者美元符号（$）。且长度不超过127B

## 示例##

* 在集合 bar 下为字段名 age 创建名为 ageIndex 的唯一索引，记录按 age 字段值的升序排序。

<pre class="prettyprint lang-javascript">
db.foo.bar.createIndex("ageIndex",{age:1},true)</pre>

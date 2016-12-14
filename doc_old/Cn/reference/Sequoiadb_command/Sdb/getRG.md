## 语法##
***db.getRG(&lt;name&gt;|&lt;id&gt; )***

返回分区组的引用。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 分区组名。同一个数据库对象中，分区组名唯一| name 和 id 任选 |
| id | int | 分区组 id，创建分区组时系统自动分配 | id 和 name 任选 |

## 格式##

getRG() 方法定于格式包含 name 或 id 字段，name 为字符串型，id 为 int 型。指定的分区组名或 id 值要在数据库对象中存在，否则出现操作异常。

<pre class="prettyprint lang-diy">
("&lt;分区组名&gt;"|&lt;id&gt;)</pre>

**Note:**

* name 字段的值不能使空串，含点（.），或者美元符号（$）。且长度不超过127B。

##示例##

* 指定 name 值，返回分区组 rg1 的引用

<pre class="prettyprint lang-javascript">
> db.getRG("rg1")</pre>

* 指定 id 值，返回分区组 rg1 的引用

<pre class="prettyprint lang-javascript">
> db.getRG(1000)</pre>

## 语法##
***db.dropCS(&lt;name&gt;)***

在数据库对象中删除指定的集合空间。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 集合空间名。同一个数据库对象中集合空间名唯一。 | 是 |

## 格式##

删除集合空间的定义格式只有 name 字段，name 的值为 string 类型，指定的集合空间名必须要在数据库对象中存在，否则操作异常。

<pre class="prettyprint lang-diy">
("<集合空间名>")</pre>

**Note:**

* name字段的值不能使空串，含点（.），或者美元符号（$）。且长度不超过127B。
* 集合空间在数据库对象中存在。

## 示例##

* 删除名为 foo 的集合空间，假定 foo 已存在

<pre class="prettyprint lang-javascript">
> db.dropCS("foo")</pre>

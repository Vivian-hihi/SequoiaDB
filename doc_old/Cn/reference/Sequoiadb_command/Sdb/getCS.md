## 语法##
***db.getCS(&lt;name&gt;)***

返回指定集合空间对象的引用。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 集合空间名。同一个数据库对象中集合空间名唯一。 | 是 |

## 格式##

getCS() 方法的定义格式只有 name 字段，name 的值是字符串型 。

<pre class="prettyprint diy">
("<集合空间名>")</pre>

**Note:**

* name 字段的值不能使空串，含点（.），或者美元符号（$）。且长度不超过127B。
* 集合空间在数据库对象中存在

## 示例##

* 返回集合空间 foo 的引用，假定 foo 已存在。

<pre class="prettyprint lang-javascript">
> db.getCS("foo")</pre>

## 语法##
***db.collectionspace.dropCL(&lt;name&gt;)***

删除指定集合空间下指定的集合。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ |------ |
| name | string | 集合名，在同一个集合空间中，集合名必须唯一。 | 是 |

## 格式##

dropCL() 方法的定义格式必须指定 name 参数，并且 name 的值在集合空间中存在，否则操作异常。

<pre class="prettyprint lang-diy">
{"name":"<集合名>"}</pre>

**Note:**

* name 的值不能是空串，含点（.）或者美元符号（$），并且长度不能超过127B，否则操作失败
* 集合名必须在集合空间中存在，否则操作异常

## 示例##

* 删除集合空间 foo 下的集合 bar。假定集合存在

<pre class="prettyprint lang-javascript">
> db.foo.dropCL("bar")</pre>

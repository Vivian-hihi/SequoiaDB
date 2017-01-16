## 语法##
***db.removeRG(&lt;name&gt;)***

删除数据库中指定的分区组。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 分区组名，同一个数据库对象中，分区组名唯一。 | 是 |

## 格式##

removeRG() 方法的定义格式只有 name 字段，name 的值是字符串型，必填。

<pre class="prettyprint lang-diy">
(<"分区组名">)</pre>

**Note:**

* 分区组名必须存在。

## 示例##

* 删除名为“group”的分区组

<pre class="prettyprint lang-javascript">
> db.removeRG("group")</pre>

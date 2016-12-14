##语法##
***db.createRG(&lt;name&gt;)***

新建一个分区组。创建后系统自动为分区组分配一个 GroupId。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 分区组名，同一个数据库对象中，分区组名唯一。 | 是 |

##格式##

createRG() 方法的定义格式只有 name 字段，name 的值是字符串型，必填。
<pre class="prettyprint diy">
(<"分区组名">)</pre>

**Note:**

* 分区组名不能是空串，含点（.）或者美元符号（$），并且长度不能超过127B。

## 示例

* 新建名为“group”的分区组

<pre class="prettyprint lang-javascript">
> db.createRG("group")</pre>

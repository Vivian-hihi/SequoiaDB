## 语法##
***db.resetSnapshot([cond])***

重置快照。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| con | Json 对象 | 选择条件，只重置符合 cond 条件的快照记录，为 null 时，重置所有。 | 否 |

## 格式##

resetSnapshot() 方法定义格式有 cond 参数，它是一个 Json 对象。

<pre class="prettyprint lang-diy">
{["cond":"{"字段名1":{"匹配符1":"值1"},"字段名2":{"匹配符2":"值2"}...}"]}</pre>

## 示例##

* 重置 SessionID 大于1的快照。

<pre class="prettyprint lang-javascript">
> db.resetSnapshot({SessionID:{$gt:1}})</pre>

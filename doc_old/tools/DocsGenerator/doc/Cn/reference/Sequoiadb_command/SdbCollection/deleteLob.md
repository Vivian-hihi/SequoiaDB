##语法##
***db.collectionspace.collection.deleteLob(&lt;oid&gt;)***

删除集合中的大对象。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| oid | string | 大对象的唯一描述符。 | 是 |

## 示例##

* 删除一个描述符为 5435e7b69487faa663000897 的大对象

<pre class="prettyprint lang-javascript">
> db.foo.bar.deleteLob('5435e7b69487faa663000897')</pre>

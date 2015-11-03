##语法##
***db.collectionspace.collection.listLobs()***

枚举集合中的大对象。

## 参数描述##
| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| - | - | - | - |

**Note:**

此方法暂不支持排序等查询操作。

## 示例##

* 枚举 foo.bar 中的所有大对象

<pre class="prettyprint lang-javascript">
> db.foo.bar.listLobs()</pre>

## 语法##
***db.collectionspace.collection.detachCL(&lt;subCLFullName&gt;)***

从主分区集合中分离出子分区集合。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| partitionName | string | 子分区名（原子分区集合名） | 是 |

## 示例##

* 从主分区集合中分离指定子分区

<pre class="prettyprint lang-javascript">
> db.foo.year.detachCL("foo2.January")</pre>

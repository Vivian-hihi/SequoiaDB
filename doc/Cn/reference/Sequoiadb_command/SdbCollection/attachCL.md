## 语法##
***db.collectionspace.collection.attachCL(&lt;subCLFullName&gt;, &lt;options&gt;)***

在主分区集合下挂载子分区集合。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| subCLFullName | string | 子分区集合名（包含集合空间名） | 是 |
| options | Json 对象 |  分区范围，包含两个字段“LowBound”（区间左值）以及“UpBound”（区间右值），例如：{LowBound:{a:0},UpBound:{a:100}}表示取字段“a”的范围区间：[0, 100) | 是 |

## 示例##

* 在主分区集合的指定区间下挂载子分区集合

<pre class="prettyprint lang-javascript">
> db.foo.year.attachCL("foo2.January",{LowBound:{date:"20130101"},UpBound:{date:"20130131"}}</pre>

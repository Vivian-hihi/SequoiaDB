## 描述##

$sort 用来指定结果集的排序规则。对嵌套对象使用点操作符（.）引用字段名。

## 示例##

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$sort:{score:-1,name:1}});</pre>

该操作表示从集合 collection 中读取记录，并以 score 的字段值进行降序排序（1表示升序，-1表示降序）；

当记录间 score 字段值相同时，则以 name 字段值进行升序排序。

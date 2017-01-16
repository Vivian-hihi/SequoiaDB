## 描述##

$skip 参数控制结果集的开始点，即跳过结果集中指定条数的记录。如果跳过的记录数大于总记录数，返回0条记录。

## 示例##

* 跳过10条记录返回

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate( { $skip : 10 } ) ;</pre>

该操作表示从集合 collection 中读取记录，并跳过前面10条，从第11条记录开始返回。

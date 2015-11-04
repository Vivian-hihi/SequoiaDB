##语法##
***db.collectionspace.collection.createIdIndex()***

在 SequoiaDB 中创建集合时可以根据需要将 AutoIndexId 置为 true。这样集合将不会创建默认的“$id”索引，同时数据的更新、删除操作将被禁止。本方法可以恢复“&#36;id”索引，同时开放更新和删除功能。

##参数描述##

参数名          参数类型       解决方法                                                                         是否必填
--------------- -------------- -------------------------------------------------------------------------------- --------
SortBufferSize  int            创建索引时使用的排序缓存的大小，单位为MB。取值为0时表示不使用排序缓存。默认为64。否

##常见的错误##

错误码   可能的原因               解决方法
-------- ------------------------ --------------------
-247     $id 索引已经存在         -
-291     存在一个相同定义的索引   删除定义冲突的索引

##示例##

* 使用默认参数构建 $id 索引:

<pre class="prettyprint lang-javascript">
> db.foo.bar.createIdIndex();</pre>

* 使用离线方式构建 $id 索引:

<pre class="prettyprint lang-javascript">
> db.foo.bar.createIdIndex({SortBufferSize:128})</pre>

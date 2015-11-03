##语法##
***db.collectionspace.collection.createIdIndex()***

在 SequoiaDB 中创建集合时可以根据需要将 AutoIndexId 置为 true。这样集合将不会创建默认的“$id”索引，同时数据的更新、删除操作将被禁止。本方法可以恢复“&#36;id”索引，同时开放更新和删除功能。

##参数描述##

参数名   参数类型       解决方法                                                                                                          是否必填
-------- -------------- ----------------------------------------------------------------------------------------------------------------- --------
Offline  Bool           true表示使用离线模式创建索引。当离线创建进行时，无法对集合进行写操作，但会大大提高创建索引的速度。缺省为 false。  否

##常见的错误##

错误码   可能的原因               解决方法
-------- ------------------------ --------------------
-247     $id 索引已经存在         -
-291     存在一个相同定义的索引   删除定义冲突的索引

##示例##

* 使用默认参数构建 $id 索引:

<pre class="prettyprint lang-javascript">
> db.foo.bar.createIdIdnex();</pre>

* 使用离线方式构建 $id 索引:

<pre class="prettyprint lang-javascript">
> db.foo.bar.createIdIndex({Offline:true})</pre>

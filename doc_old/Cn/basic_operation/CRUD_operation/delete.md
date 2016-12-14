  删除操作即移除集合中的记录。SequoiaDB中使用[remove()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/remove.html)方法做删除操作。

  **Note:** 本文档的所有例子都是使用 SequoiaDB 的 shell 接口。

##remove()##

remove() 方法是删除集合中记录主要方法，它的语法结构为：

<pre class="prettyprint lang-javascript">
db.collectionspace.collection.remove([cond],[hint])</pre>

在 SQL 中对应的操作：remove() 的方法与 DELETE 语句相似：

-  [cond] 参数对应 where 语句

-  [hint] 参数是对应索引表里的名称

##删除集合记录##

-   删除集合中的所有记录

    以下操作会删除集合 bar 中所有的记录。

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.remove()</pre>

-   删除集合中匹配条件的记录

    以下操作会删除集合 bar 中所有匹配 name 字段值为“Tom”的记录。

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.remove({name:"Tom"})</pre>

-   hint 参数

    以下操作会通过索引遍历快速删除匹配条件的记录，“textIndex”为索引名称。

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.remove({name:"Tom"},{"":"testIndex"})</pre>

更新操作即修改集合中已存在的记录。SequoiaDB中使用 [update()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/update.html)方法做更新操作。

**Note:**
本文档的所有例子都是使用 SequoiaDB 的 shell 接口。

##update()##

update() 方法是修改集合中记录的主要方法，它的语法结构为：

<pre class="prettyprint lang-javascript">
db.collectionspace.collection.update(&lt;rule&gt;,[cond],[hint])</pre>

在 SQL中 对应的操作：update() 的方法与 update...set 语句相似：

-  &lt;rule&gt;参数对应 set 语句

-  [cond] 参数对应 where 语句

-  [hint] 参数是对应索引表里的名称

##使用 update 操作修改记录##

如果 update() 方法只有 rule 参数的表达式（例如使用 $set 更新表达式），那么 update 方法会修改集合记录中所有指定的字段；更新嵌套对象 SequoiaDB 使用点（.）操作符。

-   更新记录字段

    使用 $set 更新记录字段的值。下面的操作修改集合 bar 中符合条件 \_id 字段值等于1的记录，使用 $set 修改 name 字段的嵌套元素 first字段的值，将它的值修改为“Mike”：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.update({$set:{"name.first":"Mike"}},{_id:1})</pre>

    **Note: **

    如果 rule 参数包含的字段名没有在当前的记录中，update()方法会添加 rule 参数包含的字段到记录中。

-   删除记录字段

    使用 $unset 删除记录的字段名。下面的操作是删除集合 bar 中所有含有 age 字段的记录，如果记录中没有 age 字段，跳过。

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.update({$unset:{age:""}})</pre>

-   数组元素更新

    如果需要更新数组中的元素，SequoiaDB使用点操作符（.），数组下标从0开始。下面的操作是修改数组字段 arr 的第二个元素的值，将它的值添加为5：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.update({$inc:{"arr.1":5}})</pre>

-   hint 参数

    下面操作会通过索引遍历对所有记录的 name 字段内容修改为Tom，“textIndex”为索引名称。

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.update({$set:{name:"Tom"}},null,{"":"textIndex"})</pre>

##语法##
***db.collectionspace.collection.count([cond])***

统计指定集合空间下指定集合的记录总数。

##参数描述##

参数名   参数类型    描述                                                                          是否必填
-------- ----------- ----------------------------------------------------------------------------- ----------
cond     Json 对象   选择条件。为空时，统计集合下所有的记录总数；不为空时，统计符合条件的记录总数。   否

##格式##

count() 方法的定义格式包含 cond 字段，它是一个 JSON 对象。

<pre class="prettyprint lang-diy">
{[{"字段名1":{"匹配符1":"值1"},"字段名2":{"匹配符2":"值2"},...}]}</pre>

##示例##

-   统计集合 bar 所有的记录数，即不指定参数 cond

<pre class="prettyprint lang-javascript">
> db.foo.bar.count()</pre>

-   统计符合条件 name 字段的值为“Tom”且 age 字段的值大于25的记录数

<pre class="prettyprint lang-javascript">
> db.foo.bar.count({name:"Tom",age:{$gt:25}})</pre>

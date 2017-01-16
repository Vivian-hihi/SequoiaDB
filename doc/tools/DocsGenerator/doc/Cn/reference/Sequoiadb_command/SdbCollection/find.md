##语法##
***db.collectionspace.collection.find([cond],[sel])***

选择集合记录，对选择的记录返回一个游标（cursor）。在 SequoiaDB中 游标是一个指针，指向一个查询结果集，客户端可以遍历检索结果。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| cond | Json 对象 | 选择条件。为空时，查询所有记录，不为空时，查询符合条件记录。 | 否 |
| sel | Json 对象 | 控制返回字段名。为空时，返回记录的所有字段，如果指定的字段名不在记录中，返回。 | 否 |

## 格式##

find() 的定义格式包含 cond 和 sel 两个参数，都是 JSON 对象类型。cond 控制符合条件的记录，sel 控制返回记录的字段名。

<pre class="prettyprint lang-diy">
{[{"字段名1":{"匹配符1":"值1","字段名2":{"匹配符2":"值2"},...}],[{"字段名1":"","字段名2":"",..}]}</pre>

**Note:**

* sel 是一个 Json 对象，字段的值一般设定为空。而如果指定值：{"字段名1":"值1","字段名2":"值2",...}，如果记录中存在所选字段，设定的值（值1，值2...）不生效；如果记录中不存在所选字段，则按指定的值输出。

## 示例##

* 查询所有记录，不指定 cond 和 sel 字段。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find()</pre>


* 查询匹配条件的记录，即设置 cond 参数的内容。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:25},name:"Tom"})</pre>

此操作返回集合 bar 中符合条件 age 字段值大于25且 name 字段值为“Tom”的记录。

* 指定返回的字段名，即设置 sel 参数的内容。如有记录

<pre class="prettyprint lang-diy">
{age:25,type:"system"}和{age:20,name:"Tom",type:"normal"}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.find(null,{age:"",name:""})</pre>

此操作返回记录的 age 字段和 name 字段，执行后返回：

<pre class="prettyprint lang-diy">
{age:25,name:""}，{age:20,name:"Tom"}</pre>

虽然第一条记录没有 name 字段，还是会返回 

<pre class="prettyprint lang-diy">
name:""</pre>

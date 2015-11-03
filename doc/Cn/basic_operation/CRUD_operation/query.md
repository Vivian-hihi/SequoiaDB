##find()##

我们使用 [find()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/find.html) 方法读取 SequoiaDB 中的记录。find方法是从集合中选择记录的主要方法，它返回一个包含很多记录的游标。它的语法结构如下：

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.find([cond],[sel])</pre>

在 SQL 中对应的操作：find() 的方法与 SELECT 语句相似：

-  [cond] 参数对应 WHERE 语句

-  [sel] 参数对应从结果集中选择的字段列表

现集合中有如下一条记录：

<pre class="prettyprint lang-diy">
{
  "_id":1,
  "name":
    {
      "first" : "Tom",
      "second":"David"
    },
  "age":23
  "birth":"1990-04-01",
  "phone":
    [
      10086,
      10010,
      10000
    ],
  "family":
    [
      {
        "Dad":"Kobe",
        "phone":139123456
      },
      {
        "Mom":"Julie",
        "phone":189123456
      }
    ]
 }</pre>

##返回集合所有记录##

如果没有 cond 参数，方法 db.collectionspace.colletion.find() 选择集合中所有的记录，如下返回集合空间 foo 中集合 bar 的所有记录：

<pre class="prettyprint lang-javascript">
> db.foo.bar.find()</pre>

##返回匹配条件的记录##

-   Equality 匹配

    下面的操作返回集合 bar 中 age 等于23的记录

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find({age:23})</pre>

-   使用匹配符

    下面操作返回集合 bar 中 age 字段值大于20的记录

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find({age:{$gt:20}})</pre>

-   嵌套数组匹配

    1.数组元素查询，下面的操作操作返回一个游标，指向集合 bar 中所有数组类型字段 phone 含有元素10086的记录：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find({"phone":10086})</pre>

    2.数组元素为 BSON 对象的查询，下面的操作返回一个游标指向集合 bar 中 family 字段包含的子元素 Dad 字段值为“Kobe”，phone字段值为139123456的记录：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find(
    {
      "family":{
        $elemMatch: {
          "Dad":"Kobe",
          "phone":139123456
        }
      }
    })</pre>

-   嵌套 BSON 对象匹配查询

    下面的操作返回一个游标指向集合 bar 中嵌套 BSON 对象的 name 字段匹配{"first":"Tom"}的记录

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find(
    {
      "name":{
        "first":"Tom"
      }
    }
    )</pre>

    上面还可以写成：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find(
    {
      "name.first":"Tom"
    }
    )</pre>

##指定返回记录字段##

如果指定 find 方法的 sel 参数，那么只返回指定的 sel 参数内的字段名。下面的操作返回记录的 name 字段：

<pre class="prettyprint lang-javascript">
> db.foo.bar.find(null,{name:""})</pre>

**Note:**

如果记录中不存在指定的字段名（如：people），SequoiaDB 默认也返回。如：

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{name:"",people:""})</pre>

返回：
<pre class="prettyprint lang-diy">
{
  "name":
  {
    "fist":"Tom",
    "second":"David"
  },
  "people":""
}</pre>

##更多信息##

执行 db.foo.bar.find().help() , 会看到 find() 的更多使用方法

-   cursor.sort(&lt;sort&gt;)

    sort()方法用来按指定的字段排序，语法格式为：sort({"字段名1"：1|-1,"字段名2"：1|-1,...})，1为升序，-1为降序。如果 find 方法的 sel 参数不设定内容，sort() 方法按指定 sort 参数设定的字段排序，如果 sel 参数设定了返回的字段名，那么 sort() 方法只能对 sel 参数中选定的字段进行排序。如：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find().sort({age:1})</pre>

    对返回的记录按 age 字段值的升序排序

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find(null,{name:""}).sort({age:1})</pre>

    此操作实际上对返回的记录达不到排序的效果。

-   cursor.hint(&lt;hint&gt;)

    添加索引加快查找速度，假设存在名为“testIndex”的索引：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find().hint({"":"testIndex"})</pre>

-   cursor.limit(&lt;num&gt;)

    在结果集中限制返回的记录条数：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find().limit(3)</pre>

    返回结果集里面的的前三条记录

-   cursor.skip(&lt;num&gt;)

    skip() 方法控制结果集的开始点，即跳过前面的 num 条记录，从num+1条记录开始返回：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find().skip(5)</pre>

    从查询的结果集的第6条记录开始返回

-   使用游标控制 find() 返回的记录

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find().current()   //返回当前游标指向的记录
    > db.foo.bar.find().next()      //返回当前游标指向的下一条记录
    > db.foo.bar.find().close()     //关闭当前游标，当前游标不再可用
    > db.foo.bar.find().count()     //返回当前游标的记录总数
    > db.foo.bar.find().size()      //返回当前游标到最终游标的距离
    > db.foo.bar.find().toArray()   //以数组形式返回结果集</pre>

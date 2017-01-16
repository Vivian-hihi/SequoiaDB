在 SequoiaDB 中，create 操作是向集合中添加新的文档记录。我们可以使用[insert()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/insert.html) 方法向 SequoiaDB 中的集合中添加记录。

  所有的插入操作在 SequoiaDB 中具有如下性质：

  -   如果插入的文档记录没有 \_id 字段，客户端将会为记录自动添加 \_id字段，并且填充一个唯一值。
  -   如果指定 \_id 字段，那个在集合中 \_id 的值必须唯一；否则出现操作异常。
  -   最大的 BSON 文档长度为16MB。
  -   文档结构的字段命名有如下限制：

字段名 \_id作为主键保存在集合中，它的值必须唯一且不可改变，它的值可以是除数组类型以外的其他任何类型。字段的命名不能是空串；不能以$开始；不能含有（.）。

**Note:** 本文档的所有例子都是使用 SequoiaDB 的 shell 接口。

##insert()##

insert() 是向SequoiaDB 集合中插入记录的主要方法，它有以下语法：

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.insert(&lt;doc|docs&gt;,[flag])</pre>

###插入第一个文档###

如果[集合空间](SdbDoc_Cn/data_model/collectionspace.html)和[集合](SdbDoc_Cn/data_model/collection.html)不存在，首先创建集合空间（如db.createCS("foo")：创建集合空间 foo）和集合（如db.foo.createCL("bar")：在集合空间下创建集合 bar），之后才能做插入操作。

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert(
  {
     _id:1,
     name:{fist:"Jhon",last:"Black"},
     phone:[1853742000,1802321000],
     remark:[
      {
        position:"manager",
        year:2000
      },
      {
        position:"CEO",
        year:2012
      }
    ]
  }
)</pre>

可以使用 find() 方法确认是否插入成功。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find()</pre>

此操作返回结果如下：

<pre class="prettyprint lang-diy">
{
  _id:1,
  name:{fist:"Jhon",last:"Black"},
  phone:[1853742000,1802321000],
  remark:[
    {
      position:"manager",
      year:2000
    },
    {
      position:"CEO",
      year:2012
     }
  ]
}</pre>

###不指定 \_id 字段###

如果新的文档记录不包含 \_id字段，[insert()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/insert.html)方法向文档添加 \_id 字段并生成一个唯一的 $oid 值

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({name:"Tom",age:20})</pre>

此操作是向集合 bar 中插入一条新的记录，记录 name 字段的值为“Tom”，age字段的值为20，_id 字段被唯一创建：

<pre class="prettyprint lang-diy">
{ "_id": { "$oid": "515152ba49af395200000000" }, "name": "Tom", "age": 20 }</pre>

###插入多条记录###

如果向 insert 方法中传一个数组类型的文档，insert()方法将会在集合中执行批量插入。

下面的操作是向集合 bar 中插入两条记录。此操作也说明了 SequoiaDB的动态模式的特点。尽管 _id:20 的记录含有字段名 phone 而在另一条记录中不存在，SequoiaDB 不要求其他记录必须含有此字段。

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert([{name:"Mike",age:15},{_id:20,name:"John",age:25,phone:123}])</pre>

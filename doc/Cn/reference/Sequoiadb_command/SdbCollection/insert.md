##语法##
***db.collectionspace.collection.insert(&lt;doc|docs&gt;,[flag])***

向指定集合中插入记录。如果集合空间或集合不存在，首先需要手动创建一个集合空间，如 db.createCS("foo")，再在该集合空间下手动创建集合，如 db.foo.createCL("bar")。然后在集合中插入记录。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| doc|docs | Json 对象 | 文档记录。doc 为一条记录，docs 为多条记录。 | 是 |
| flag | Int | 可取 SDB_INSERT_RETURN_ID 或者 SDB_INSERT_CONTONDUP。前者在插入单条记录时有效，表示插入记录后返回记录中“\_id”字段内容；后者在插入多条记录时有效，表示在插入的记录中，若存在“\_id”字段内容重复的记录时，将跳过这些存在重复“\_id”的记录继续插入后面记录。默认情况下，当存在重复“\_id”字段内容的记录时，将停止插入后面的记录。 | 否 |

## 格式##

insert() 方法的定义格式包含 doc|docs 和 flag 两个字段。

doc：
<pre class="prettyprint lang-diy">
{"&lt;字段名 1&gt;":"&lt;值&gt;","&lt;字段名 2>":"&lt;值&gt;",…}</pre>

docs：
<pre class="prettyprint lang-diy">
{ 
    [
        {"&lt;字段名 1&gt;":"&lt;值&gt;","&lt;字段名 2>":"&lt;值&gt;",…},
        {"&lt;字段名 1&gt;":"&lt;值&gt;","&lt;字段名 2>":"&lt;值&gt;",…},
        ...
    ] 
}</pre>

**Note:**

如果插入的记录不指定 \_id 字段时，SequoiaDB 会自动为记录添加一个 \_id 字段来标识记录的唯一性。

## 示例##

* 不指定 _id 字段，插入一条记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({name:"Tom",age:20})</pre>

此操作是向集合 bar 中插入一条新的记录，name 字段的值为“Tom”，age 字段的值为20，_id 字段被唯一创建：
<pre class="prettyprint lang-diy">
{ "_id": { "$oid": "515152ba49af395200000000" }, "name": "Tom", "age": 20 }</pre>

* 插入一条带有 _id 字段的记录。
<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({_id:10,age:20})</pre>

此操作是向集合 bar 中插入一条新的记录，_id 字段的值为10，age 字段的值为20：
<pre class="prettyprint lang-diy">
{ "_id": 10, "age": 20 }</pre>

* 插入多条记录。
<pre class="prettyprint lang-javascript">
db.foo.bar.insert([{_id:20,name:"Mike",age:15},{name:"John",age:25,phone:123}])</pre>

此操作将会在集合 bar 中插入两条记录：

1）其中一条记录 _id 字段的值为20，name 字段的值为“Mike”，age 字段的值为15。

2）一条记录系统自动为 _id 字段生成唯一值，name 字段的值为“John”，age 字段的值为25，phone 字段的值为123。
<pre class="prettyprint lang-diy">
{
    "_id": 20,
    "name": "Mike",
    "age": 15
}
{
    "_id": { "$oid": "5151557a49af395200000001" },
    "name": "John",
    "age": 25,
    "phone": 123
}</pre>


* 插入拥有重复“_id”键的多条记录。
<pre class="prettyprint lang-javascript">
> db.foo.bar.insert([{_id:1,a:1 },{_id:1,b:2 },{_id:3,c:3}], SDB_INSERT_CONTONDUP)</pre>

此操作将会在集合 bar 中插入两条记录：
<pre class="prettyprint lang-diy">
{
    "_id": 1,
    "a": 1,
}
{
    "_id": 3,
    "c": 3
}</pre>

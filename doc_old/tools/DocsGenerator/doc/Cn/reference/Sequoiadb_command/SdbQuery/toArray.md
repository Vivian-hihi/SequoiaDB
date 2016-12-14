##语法##
***query.toArray()***

以数组的形式返回结果集。


## 示例##

* 以数组的形式返回集合 bar 中 age 字段值大于5的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).toArray()</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "516a76a1c9565daf06030000"
  },
  "age": 10,
  "name": "Tom"
},{
  "_id": {
    "$oid": "516a76a1c9565daf06050000"
  },
  "age": 20,
  "a": 10
},{
  "_id": {
    "$oid": "516a76a1c9565daf06040000"
  },
  "age": 15
}</pre>

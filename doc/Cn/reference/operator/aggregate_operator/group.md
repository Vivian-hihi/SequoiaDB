##描述##

$group 实现对结果集的分组，类似 SQL 中的 group by 语句。首先指定分组键（\_id） ，通过“\_id”来标识分组字段，分组字段可以是单个，也可以是多个，格式如下：

单个分组键：

<pre class="prettyprint lang-diy">
{_id:"$field"}</pre>

多个分组键：

<pre class="prettyprint lang-diy">
{_id:{field1:"$field1",field2:"$field2",...}}</pre>

##示例##

* $group 使用如下

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$major",avg_score:{$avg:"$score"},Major:{$first:"$major"}}})</pre>

该操作表示从集合 collection 中读取记录，并按 major 字段进行分组。在返回的结果集中，取各分组的第一条记录的 major 字段，重命名为 Major；对各分组中的 score 字段值求平均值，重命名为 avg_score。返回如下所示：

<pre class="prettyprint lang-diy">
{
  "avg_score": 82,
  "major": "光学"
}
{
  "avg_score": 77.25,
  "major": "物理学"
}</pre>

##$group 支持的聚集函数：##

+-----------+--------------------------------------------------------------------+
| 函数名    | 描述                                                               |
+===========+====================================================================+
| $addtoset | 将字段添加到数组中，相同的字段值只会添加一次                       |
+-----------+--------------------------------------------------------------------+
| $first    | 取分组中第一条记录中的字段值                                       |
+-----------+--------------------------------------------------------------------+
| $last     | 取分组中最后一条记录中的字段值                                     |
+-----------+--------------------------------------------------------------------+
| $max      | 取分组中字段值最大的                                               |
+-----------+--------------------------------------------------------------------+
| $min      | 取分组中字段值最小的                                               |
+-----------+--------------------------------------------------------------------+
| $avg      | 取分组中字段值的平均值                                             |
+-----------+--------------------------------------------------------------------+
| $push     | 将所有字段添加到数组中，即使数组中已经存在相同的字段值，也继续添加 |
+-----------+--------------------------------------------------------------------+
| $sum      | 取分组中字段值的总和                                               |
+-----------+--------------------------------------------------------------------+
| $count    | 对记录分组后，返回表所有的记录条数                                 |
+-----------+--------------------------------------------------------------------+

##$addtoset##

记录分组后，使用 $addtoset 将指定字段值添加到数组中，相同的字段值只会添加一次。对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 如下操作对记录分组后将指定字段值添加到数组中输出

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",Dep:{$first:"$dep"},addtoset_major:{$addtoset:"$major"}}})</pre>

此操作对记录按 dep 字段值进行分组，并使用 \$first 输出每个组第一条记录的 dep 字段，输出字段名为 Dep；又将 major 字段的值使用 \$addtoset 放入数组中返回，输出字段名为 addtoset_major，如下：

<pre class="prettyprint lang-diy">
{
  "Dep": "物电学院",
  "addtoset_major": [
    "物理学",
    "光学",
    "电学"
  ]
}
{
  "Dep": "计算机学院",
  "addtoset_major": [
    "计算机科学与技术",
    "计算机软件与理论",
    "计算机工程"
  ]
}</pre>

##$count##

记录分组后，用 $count 取出分组所包含的总记录条数。

###示例###

* 对记录分组后，返回表所有的记录条数

<pre class="prettyprint lang-javascript">
> db.foo.bar.count({$group:{Total:{$count:"$dep"}}})
{
  "Total": 1001
}</pre>

##$first##

记录分组后，取分组中第一条记录指定的字段值，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，输出每个分组第一条记录的指定字段值

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",Dep:{$first:"$dep"},Name:{$first:"$info.name"}}})</pre>

此操作对记录按 dep 字段分组，取每个分组中第一条记录的 dep 字段值和嵌套对象 name 字段值，输出字段名分别为 Dep 和 Name，记录返回如下：

<pre class="prettyprint lang-diy">
{
  "Dep": "物电学院",
  "Name": "Lily"
}
{
  "Dep": "计算机学院",
  "Name": "Tom"
}</pre>

##$avg##

记录分组后，取分组中指定字段的平均值返回，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，返回分组中指定字段的平均值

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",avg_age:{$avg:"$info.age"},max_age:{$max:"$info.age"},min_age:{$min:"$info.age"}}})</pre>

此操作对记录按 dep 字段分组，使用 \$avg 返回每个分组中的嵌套对象 age 字段的平均值，输出字段名为 avg_age；又使用 \$min 返回每个分组中嵌套对象 age 字段的最小值，输出字段名为 min_age，使用 $max 返回每个分组中嵌套对象 age 字段的最大值，输出字段名为 max_age。记录返回如下：

<pre class="prettyprint lang-diy">
{
  "avg_age": 23.727273,
  "max_age": 36,
  "min_age": 15
}
{
  "avg_age": 24.5,
  "max_age": 30,
  "min_age": 20
}</pre>

##$max##

记录分组后，取分组中指定字段的最大值返回，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，返回分组中指定字段的最大值

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",max_score:{$max:"$score"},Name:{$last:"$info.name"}}})</pre>

此操作对记录按 dep 字段分组，使用 \$max 返回每个分组中 score 字段的最大值，输出字段名为 max_score，又使用 \$last 取每个分组中最后一条记录嵌套对象 name 字段值，输出字段名为 Name。记录返回如下：

<pre class="prettyprint lang-diy">
{
  "max_score": 93,
  "Name": "Kate"
}
{
  "max_score": 90,
  "Name": "Jim"
}</pre>

##$min##

记录分组后，取分组中指定字段的最小值返回，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，返回分组中指定字段的最小值

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",min_score:{$min:"$score"},Name:{$last:"$info.name"}}})</pre>

此操作对记录按 dep 字段分组，使用 \$min 返回每个分组中 score 字段的最小值，输出字段名为 min_score，又使用 \$last 取每个分组中最后一条记录嵌套对象 name 字段值，输出字段名为 Name。记录返回如下：

<pre class="prettyprint lang-diy">
{
  "min_score": 72,
  "Name": "Kate"
}
{
  "min_score": 69,
  "Name": "Jim"
}</pre>

##$last##

记录分组后，取分组中最后一条记录指定的字段值，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，输出每个分组最后一条记录的指定字段值

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",Major:{$addtoset:"$major"},Name:{$last:"$info.name"}}})</pre>

此操作对记录按 dep 字段分组，使用 \$last 取每个分组中最后一条记录嵌套对象 name 字段值，输出字段名为 Name，并且将每个分组中的 major 字段值使用 \$addtoset 填充到数组中返回，返回字段名为 Major；记录返回如下：

<pre class="prettyprint lang-diy">
{
  "Major": [
    "物理学",
    "光学",
    "电学"
  ],
  "Name": "Kate"
}
{
  "Major": [
    "计算机科学与技术",
    "计算机软件与理论",
    "计算机工程"
  ],
  "Name": "Jim"
}</pre>

##$push##

记录分组后，使用 $push 将指定字段值添加到数组中，即使数组中已经存在相同的值，也继续添加。对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 如下操作对记录分组后将指定字段值添加到数组中输出

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",Dep:{$first:"$dep"},push_age:{$push:"$info.age"}}})</pre>

此操作对记录按 dep 字段值进行分组，每个分组中嵌套对象 age 字段的值使用 $push 放入数组中返回，输出字段名为 push_age，如下：

<pre class="prettyprint lang-diy">
{
  "Dep": "物电学院",
  "push_age": [
    28,
    18,
    20,
    30,
    28,
    20
  ]
}
{
  "Dep": "计算机学院",
  "push_age": [
    25,
    20,
    22
  ]
}</pre>

##$sum##

记录分组后，返回每个分组中指定字段值的总和，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，返回分组中指定字段值的总和

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",sum_score:{$sum:"$score"},Dep:{$first:"$dep"}}})</pre>

此操作对记录按 dep 字段分组，使用 \$sum 返回每个分组中 score 字段值的总和，输出字段名为 sum_score；又使用 \$first 取每个分组中第一条记录的 dep 字段值，输出字段名为 Dep。记录返回如下：

<pre class="prettyprint lang-diy">
{
  "sum_score": 888,
  "Dep": "物电学院"
}
{
  "sum_score": 476,
  "Dep": "计算机学院"
}</pre>

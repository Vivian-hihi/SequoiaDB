##说明##

选择某个字段。当字段不存在时返回默认值。可简写为{&lt;fieldName&gt;:&lt;defaultValue&gt;}。

##示例##

为方便理解，给出原始数据样例：

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 }
  ]
}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:[], teacher:{$default:"Mr Liu"}})</pre>

返回

<pre class="prettyprint lang-diy">
{
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 },
  ],
  teacher:"Mr Liu"
}</pre>
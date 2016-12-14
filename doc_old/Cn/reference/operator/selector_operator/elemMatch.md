##说明##

返回数组内满足条件的元素的集合

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
> db.foo.bar.find({class:1}, {students:{$elemMatch:{age:18}}})</pre>

返回

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"WangErmazi", age:18 },
  ]
}</pre>
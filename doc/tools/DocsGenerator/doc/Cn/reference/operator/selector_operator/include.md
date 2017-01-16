##说明##

选择(!=0)或移除(=0)某个字段

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

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1},{students:{$include:1}})</pre>

返回 student 字段

<pre class="prettyprint lang-diy">
{
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 },
  ]
}</pre>

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1},{_id:{$include:0}})</pre>

返回移除 _id 字段

<pre class="prettyprint lang-diy">
{
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 },
  ]
}</pre>
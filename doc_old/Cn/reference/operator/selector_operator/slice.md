##说明##

返回数组的切片

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

-    <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$slice:1}})</pre>

返回下标为0，长度为1的切片

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 }
  ]
}</pre>

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$slice:2}})</pre>

返回下标为0，长度为2的切片

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 }，
    {name:"LiSi", age:19 }
  ]
}</pre>

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$slice:-2}})</pre>

返回倒数前2个元素的切片

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"LiSi", age:19 }，
    {name:"WangErmazi", age:18 }
  ]
}</pre>

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$slice:[1,2]}})</pre>

返回下标为1，长度为2的切片

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"LiSi", age:19 }，
    {name:"WangErmazi", age:18 }
  ]
}</pre>

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$slice:[-1,2]}})</pre>

返回倒数第1个开始，长度为2的切片（长度不足，只返回1个元素）

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"WangErmazi", age:18 }
  ]
}</pre>
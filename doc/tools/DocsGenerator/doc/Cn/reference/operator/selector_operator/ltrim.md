##说明##

去掉字符串左侧开头的空格(或制表符)。非字符串类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$ltrim:1}})</pre>

**Note:**

{a:{$ ltrim:1}}中的1作为占位符出现。
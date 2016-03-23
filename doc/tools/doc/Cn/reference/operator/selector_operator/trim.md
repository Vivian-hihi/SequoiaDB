##说明##

去掉字符串两侧开头的空格(及制表符)。非字符串类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$trim:1}})</pre>

**Note:**

{a:{$ trim:1}}中的1作为占位符出现。
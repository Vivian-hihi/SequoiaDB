##说明##

返回数字的绝对值，非数字类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$abs:1}})</pre>

**Note:**

{$abs:1}中1没有特殊含义，仅作为占位符出现。
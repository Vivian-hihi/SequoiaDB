##说明##

返回字符串长度，不包括终止符。非字符串返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$strlen:1})</pre>

**Note:**

{$strlen:1}中1没有特殊含义，仅作为占位符出现。
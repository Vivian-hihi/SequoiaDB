##说明##

返回大于目标字段值的最小整数值，非数字类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$ceiling:1}})</pre>

**Note:**

{$ceiling:1}中1没有特殊含义，仅作为占位符出现。
##说明##

返回字段值除以某个数值的结果。非数字类型返回 null

##示例##

返回a字段除以10的结果

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$divide:10}})</pre>

**Note:**

除数不能为0。
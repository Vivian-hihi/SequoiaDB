##说明##

返回字段值与某个数值相乘的结果。非数字类型返回 null

##示例##

-   返回a字段乘以10的结果

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$multiply:10}})</pre>
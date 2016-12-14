##说明##

返回字段值减去某个数值的结果。非数字类型返回 null

##示例##

-   返回a字段减去10的结果

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$subtract:10}})</pre>
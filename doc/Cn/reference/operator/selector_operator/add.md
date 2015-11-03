##说明##

返回字段值加上某个数值的结果。非数字类型返回 null

##示例##

-   返回a字段加上10的结果

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$add:10}})</pre>
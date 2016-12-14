##说明##

返回取模的结果，非数字类型返回 null。

##示例##

-   返回字段a对10取模的结果

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$mod:10}})</pre>

**Note:**

不能对零取模。
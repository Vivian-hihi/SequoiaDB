##说明##

去掉字符串右侧开头的空格(回车符 '\\r'、换行符 '\\n' 以及制表符'\\t')。非字符串类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$rtrim:1}})</pre>

**Note:**

 {a:{$ rtrim:1}}中的1作为占位符出现。
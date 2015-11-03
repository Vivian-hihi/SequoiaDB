##说明##

返回字符串的子串，非字符串类型返回 null

##示例##

记录；**{a:"abcdefg"}**

-   返回下标为0，长度为2的子串

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$substr:2}})</pre>

返回：**{a:" ab"}**

-   返回倒数第二个字符开始的子串

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$substr:-2}})</pre>

返回：**{a:"fg"}**

-   返回下标为0，长度为3的子串

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$substr:[2,3]}})</pre>

返回：**{a:"cde"}**

-   返回倒数第二个字符开始，长度为3的子串

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$substr:[-2, 3]}})</pre>

返回：**{a:"fg"}**(长度不足3)
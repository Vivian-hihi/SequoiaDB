##Json 格式##

***{"$minKey":1 }***

##函数格式##

+----------+-------------+
| 格式     | 描述        |
+==========+=============+
| MinKey() | MinKey 对象 |
+----------+-------------+

##示例##

-   插入一个 MinKey 对象

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({key:MinKey()})</pre>
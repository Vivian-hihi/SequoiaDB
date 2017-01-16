##语法##
***cursor.next()***

返回当前游标指向的下一条记录。更多查看 cursor.current() 方法。

## 示例##

* 选择集合 bar 下 age 大于10的记录，返回当前游标指向的下一条记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).next()</pre>

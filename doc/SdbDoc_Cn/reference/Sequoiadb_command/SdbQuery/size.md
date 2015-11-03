##语法##
***query.size()***

返回当前游标到最终游标的记录条数。

## 示例##

* 选择集合 bar 下 age 大于10的记录，返回当前游标到最终游标的记录条数。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).size()</pre>

**Note:**

query.size()返回的结果考虑query.skip()及query.limit()的影响。

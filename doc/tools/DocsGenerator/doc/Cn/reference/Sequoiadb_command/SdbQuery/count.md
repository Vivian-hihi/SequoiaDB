##语法##
***query.count()***

返回符合匹配条件的记录条数。

## 示例##

选择集合 bar 下 age 大于10的记录，返回符合匹配条件{age:{$gt:10}}的记录条数。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).count()</pre>

**Note:**

query.count()返回的结果忽略query.skip()及query.limit()的影响。

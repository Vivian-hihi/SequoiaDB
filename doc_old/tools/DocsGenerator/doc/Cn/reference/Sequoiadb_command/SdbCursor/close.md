##语法##
***cursor.close()***

关闭当前游标，当前游标不再可用。

## 示例##

* 插入10条记录

<pre class="prettyprint lang-javascript">
> for(i = 0; i < 10; i++) { db.foo.bar.insert({a:i}) }</pre>

查询集合 foo.bar 的所有记录

<pre class="prettyprint lang-javascript">
> var cur = db.foo.bar.find()</pre>

使用游标取出一条记录

<pre class="prettyprint lang-javascript">
> cur.next()

{
  "_id": {
  "$oid": "53b3c2d7bb65d2f74c000000"
  },
  "a": 0
}</pre>

关闭游标

<pre class="prettyprint lang-javascript">
> cur.close()</pre>

再次获取下一条记录

<pre class="prettyprint lang-javascript">
> cur.next()</pre>

无结果返回

## list collectionspaces 语句##

枚举数据库中的集合空间。

## 语法##

<pre class="prettyprint lang-javascript">
list collectionspaces</pre>

## 示例##

* 本例会返回数据库中的所有集合空间

<pre class="prettyprint lang-javascript">
> db.exec("list collectionspaces") </pre>

结果：

<pre class="prettyprint lang-javascript">
{
  "Name": "testfoo"
  "Name":"big"
  ...
}</pre>

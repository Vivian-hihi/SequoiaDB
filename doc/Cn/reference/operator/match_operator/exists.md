## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$exists:<0|1>}}</pre>

## 描述##

选择集合中是否存在指定“<字段名>”的记录。“0”表示选择不存在指定“<字段名>”的记录，“1”表示选择存在指定“<字段名>”的记录。

## 示例##

* 选择集合 bar 中存在字段 age 的记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$exists:1}})</pre>

* 选择集合 bar 中嵌套对象 content 不存在 name 字段的记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({"content.name":{$exists:0}})</pre>

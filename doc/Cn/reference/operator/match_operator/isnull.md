## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$isnull: &lt;0|1&gt;}}</pre>

## 描述##

选择集合中指定的“<字段名>”是否为空，或不存在。“0”代表期望该字段存在且不为 null；“1”代表期望该字段不存在或为 null。

## 示例##

* 选择集合 bar 中 age 字段不为空且存在的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$isnull:0}})</pre>

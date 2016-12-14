## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$elemMatch:{子字段名:<值>,...}}}</pre>

## 描述##

选择集合中“<字段名>”匹配指定“{子字段名:<值>,....}”的记录。

## 示例##

* 嵌套 JSON 对象匹配

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({content:{$elemMatch:{name:"Tom",phone:123}}})</pre>

字段 content 是一个 JSON 嵌套对象，此操作匹配 content 内字段 name 值为“Tom”，phone 值为123的记录。

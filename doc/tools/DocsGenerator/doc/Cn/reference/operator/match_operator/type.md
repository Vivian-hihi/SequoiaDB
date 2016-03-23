## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$type:&lt;BSON type&gt;}}</pre>

## 描述##

选择集合中的“<字段名>”值的类型等于指定“&lt;BSON type&gt;”的值。

## BSON Type##

| Type | 描述 | 值 |
| ------ | ------ | ------ |
| 32-bit integer | 整型，范围-2147483648至2147483647 | 16 |
| 64-bit integer | 长整型，范围-9223372036854775808至9223372036854775807。如果用户指定的数值无法适用于整数，则 SequoiaDB 自动将其转化为长整数。 | 18 |
| double | 浮点数，范围1.7E-308至1.7E+308 | 1 |
| string | 字符串 | 2 |
| ObjectID | 十二字节对象 ID | 7 |
| boolean | 布尔（true \| false） | 8 |
| date | 日期（YYYY-MM-DD） | 9 |
| timestamp | 时间戳（YYYY-MM-DD-HH.mm.ss.ffffff） | 17 |
| Binary data | Base64 形式的二进制数据 | 5 |
| Regular expression | 正则表达式 | 11 |
| Object | 嵌套 JSON 文档对象 | 3 |
| Array | 嵌套数组对象 | 4 |
| null | 空 | 10 |

## 示例##

* 选择集合 bar 下 age 字段是整型的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$type:16}})</pre>

* 选择集合 bar 下嵌套对象 content 中的 arr 字段是数组类型的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({"content.arr":{$type:4}})</pre>

## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$mod:[value1,value2]},...}</pre>

## 描述##

$mod 是取模匹配符，返回指定字段名的值对 value1 取模的值等于 value2 的记录。

## 示例##

* 返回集合 bar 中 age 字段值对5取模后的值等于3的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$mod:[5,3]}})</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "521d5446e2d3c4e31c000000"
  },
  "age": 3
}
...</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$mod:[2.3,1.5]}})</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "521d5446e2d3c4e31c000000"
  },
  "age": 3
}
{
  "_id": {
    "$oid": "521d544ee2d3c4e31c000002"
  },
  "age": 5
}</pre>

对数组[2.3,1.5]中的两个元素只截取了整数部分。

**Note:**

1.不能对零取模；

2.由于操作系统提供的浮点数（IEEE754浮点数标准）的特性，浮点数的模运算结果是不准确的，超出15位有效数字的浮点数的结果甚至会严重偏离准确值，所以不建议对浮点数进行模运算，尤其是对浮点数模运算的结果进行“==”和“!=”的判断。

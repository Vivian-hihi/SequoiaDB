## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$mod:[value1,value2]},...}</pre>

## 描述##

$mod 是取模匹配符，返回指定字段名的值对 value1 取模的值等于 value2 的记录。

**Note:**

* 参数 value1 是除0以外的整型数；如果是浮点型，那只会截取整数部分；不能为其他基础类型。
* 参数 value2 是整型数；如果是浮点型，也只截取整数部分；其他类型以0处理。

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

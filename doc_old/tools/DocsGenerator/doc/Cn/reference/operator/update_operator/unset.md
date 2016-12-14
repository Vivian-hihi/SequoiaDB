## 语法##

<pre class="prettyprint lang-diy">
{$unset:{<字段名1>:"",<字段名2>:"",...}}</pre>

## 描述##

$unset 操作是删除集合中指定的字段名。如果记录中没有指定的字段名，跳过。

## 示例##

* 删除集合 bar 下记录的 name 字段和 age 字段，如果记录中没有字段 name 或 age，跳过，不做任何处理

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{name:"",age:""}})</pre>

* $unset 删除数组对象中的元素。如有一条记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,3],name:"Tom"}</pre>

使用 $unset 删除第二个元素操作如下：

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{"arr.2":""}})</pre>

此操作后，记录更新为

<pre class="prettyprint lang-diy">
{arr:[1,null,3],name:"Tom"}</pre>

* $unset 删除嵌套对象中的字段。如有一条记录：

<pre class="prettyprint lang-diy">
{content:{ID:1,type:"system",position:"manager"},name:"Tom"}</pre>

content 是一个嵌套对象，它有 ID，type，position 三个字段。使用 $unset 删除 type 字段操作如下：

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{"content.type":""}})</pre>

此操作后，记录更新为

<pre class="prettyprint lang-diy">
{content:{ID:1,position:"manager"},name:"Tom"}</pre>

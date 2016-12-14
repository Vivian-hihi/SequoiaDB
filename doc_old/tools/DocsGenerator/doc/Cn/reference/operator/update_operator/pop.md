## 语法##

<pre class="prettyprint lang-diy">
{$pop:{<字段名1>:<N>,<字段名2>:<N>,...}}</pre>

## 描述##

$pop 操作是删除指定数组对象（<字段名1>,<字段名2>,...）最后 N 个元素，操作对象必须为数组类型的字段。如果记录中不存在指定的数组对象，跳过不做任何操作；如果指定的 N 值大于数组对象的长度，数组对象的长度更新为0，即它的元素全部被删除；如果指定的 N 值 < 0，意味着从数组起始删除第 -N 个元素。

## 示例##

* 删除集合 bar 下数组对象 arr 的最后两个元素。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,3,4],age:20,name:"Tom"}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pop:{arr:2}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,2],age:20,name:"Tom"}</pre>

* 删除集合 bar 下数组对象 arr 的最后10个元素。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,3,4],age:20,name:"Tom"}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pop:{arr:10}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[],age:20,name:"Tom"}</pre>

* 删除集合 bar 下数组对象 arr 的前两个元素，即设置N的值为-2。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,3,4],age:20,name:"Tom"}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pop:{arr:-2}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[3,4],age:20,name:"Tom"}</pre>

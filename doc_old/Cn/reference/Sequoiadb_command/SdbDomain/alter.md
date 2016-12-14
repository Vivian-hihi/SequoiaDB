##语法##
***domain.alter(&lt;options&gt;)***

修改域的属性。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 需要修改的属性列表。 | 是 |

## 格式##

目前通过 options 可设置域的属性有：

| 属性名 | 描述 | 格式 |
| ------ | ------ | ------ |
| Groups | 包含的复制组。 | Groups:['data1','data2'] |
| AutoSplit | 自动切分。 | AutoSplit:true|false |

**Note:**

* 删除复制组前必须保证其不包含任何数据。
* AutoSplit 的更改不对之前创建的集合和集合空间产生影响。

## 示例##

* 首先创建一个域，包含两个复制组，开启自动切分。
	<pre class="prettyprint lang-javascript">
	> var domain = db.createDomain('mydomain',['data1','data2'],{AutoSplit:true})</pre>

	从域中删除一个复制组 data2，添加另一个复制组 data3，最后域中包含 data1 和 data3 两个复制组。
	<pre class="prettyprint lang-javascript">
	> domain.alter({Groups:['data1','data3']})</pre>

* 首先创建一个域，包含一个复制组，复制组中包含表 foo.bar。
	<pre class="prettyprint lang-javascript">
	> var domain = db.createDomain('mydomain',['group1'])</pre>

	从域中删除原复制组，添加另一个复制组，将因把拥有数据的 group1 从域中删除而报错。
	<pre class="prettyprint lang-javascript">
	> domain.alter({Groups:['group2']})
	(nofile):0 uncaught exception: -256
	> getErr(-256)
	Domain is not empty</pre>

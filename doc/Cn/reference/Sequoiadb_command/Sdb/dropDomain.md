## 语法##
***db.dropDomain(&lt;name&gt;)***

删除指定域。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 域名。 | 是 |

## 格式##

dropDomain() 方法的定义格式必须指定 name 参数，并且 name 的值在系统中存在，否则操作异常。

<pre class="prettyprint lang-diy">
{"name":"&lt;域名&gt;"}</pre>

**Note:**

* 删除域前必须保证域中不存在任何数据。
* 不能删除系统域。

## 示例##

* 删除一个之前创建的域。

<pre class="prettyprint lang-javascript">
> db.dropDomain('mydomain')</pre>

* 删除一个包含集合空间的域，返回错误：

<pre class="prettyprint lang-javascript">
> db.dropDomain('hello')
(nofile):0 uncaught exception: -256
Takes 0.1865s.
> getErr(-256)
Domain is not empty</pre>

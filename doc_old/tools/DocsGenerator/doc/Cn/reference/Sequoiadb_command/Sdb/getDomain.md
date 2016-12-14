## 语法##
***db.getDomain(&lt;name&gt;)***

获取指定域。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 域名 | 是 |

## 格式

getDomain() 方法的定义格式必须指定 name 参数，并且 name 的值在系统中存在，否则操作异常。

<pre class="prettyprint diy">
{"name":"<域名>"}</pre>

**Note:**

不能获取系统域。

## 示例##

* 获取一个之前创建的域。

<pre class="prettyprint lang-javascript">
> var domain = db.getDomain('mydomain')</pre>

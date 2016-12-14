##语法##
***db.removeProcedure(&lt;function name&gt;)***

删除指定的函数名，函数名必须存在，否则出现异常信息。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| function name | 字符串 | 函数名 | 是 |

removeProcedure() 方法的定义，只有一个字符串类型的参数名 function name，它的值必须已存在，否则异常。

## 示例##

* 删除 sum 函数

<pre class="prettyprint lang-javascript">
> db.removeProcedure("sum")</pre>

必须保证待删除函数的函数名和定义时的一致。诸如 db.removeProcedure('sum') 的调用将返回失败。

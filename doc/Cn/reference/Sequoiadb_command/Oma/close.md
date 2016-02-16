##语法##
***oma.close()***

关闭 oma 连接对象

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| - | - | - | - |

**Note:**

* 关闭的 oma 连接对象必须存在，否则出现异常。

## 示例##

* 关闭 oma

<pre class="prettyprint lang-javascript">
var oma = new Oma("localhost", 11790);
oma.close()</pre>

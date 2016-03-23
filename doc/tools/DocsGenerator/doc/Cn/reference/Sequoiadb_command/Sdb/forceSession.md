## 语法##
***db.forceSession(&lt;sessionID&gt;)***

终止指定会话的当前操作。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| sessionID | int | 会话编号。 | 是 |

**Note:**

只有用户会话可以被终止。

## 示例##

* 终止编号为100的会话。

<pre class="prettyprint lang-javascript">
> db.forceSession(100)</pre>

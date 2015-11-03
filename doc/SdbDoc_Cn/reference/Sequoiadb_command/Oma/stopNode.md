##语法##
***oma.stopNode(< svcname >)***

停止一个节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |

**Note:**

* 指定停止的节点必须存在，否则出现异常。

## 示例##

* 在集群中停止一个端口号为11830的节点

<pre class="prettyprint lang-javascript">
oma.stopNode(11830)</pre>

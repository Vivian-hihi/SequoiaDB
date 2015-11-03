##语法##
***oma.startNode(< svcname >)***

启动一个节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |

**Note:**

* 指定启动的节点必须存在，否则出现异常。

## 示例##

* 在集群中启动一个端口号为11830的节点

<pre class="prettyprint lang-javascript">
oma.startNode(11830)</pre>

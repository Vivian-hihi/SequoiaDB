##语法##
***oma.startNode(< svcname >)***

在目标集群控制器（sdbcm）所在的机器中启动一个节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |

**Note:**

* oma对象为连接到目标（本地/远端机器）集群控制器（sdbcm）获得的连接对象。
* 指定启动的节点必须存在，否则出现异常。

## 示例##

* 在本地启动一个端口号为11830的节点

<pre class="prettyprint lang-javascript">
var oma = new Oma("localhost", 11790);
oma.startNode(11830);</pre>

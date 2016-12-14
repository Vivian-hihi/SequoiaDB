##语法##
***oma.removeData(< svcname >)***

在目标集群控制器（sdbcm）所在的机器中删除一个 standalone 节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |

**Note:**

* oma对象为连接到目标（本地/远端机器）集群控制器（sdbcm）获得的连接对象。
* 指定删除的节点必须存在，否则出现异常。

## 示例##

* 在本地删除一个端口号为11820的 standalone 节点

<pre class="prettyprint lang-javascript">
var oma = new Oma("localhost", 11790);
oma.removeData(11820);</pre>

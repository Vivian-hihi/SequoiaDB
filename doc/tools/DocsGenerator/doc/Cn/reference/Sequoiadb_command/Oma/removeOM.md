##语法##
***oma.removeOM(< svcname >)***

删除sdbom服务进程。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |

**Note:**

* oma对象为连接到目标（本地/远端机器）集群控制器（sdbcm）获得的连接对象。

## 示例##

* 删除安装在本地的sdbom服务进程

<pre class="prettyprint lang-javascript">
var oma = new Oma("localhost", 11790);
oma.removeOM(11830)</pre>

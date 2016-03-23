##语法##
***oma.createCoord(< svcname >,< dbpath >,[ config obj ])***

在目标集群控制器（sdbcm）所在的机器中创建一个 coord 节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |
| dbpath | string | 节点路径。 | 是 |
| config obj | Json 对象 | 节点配置信息，如配置日志大小，是否打开事务等，具体可参考数据库配置。 | 否 |

**Note:**

* oma对象为连接到目标（本地/远端机器）集群控制器（sdbcm）获得的连接对象。

## 示例##

* 在本地创建一个端口号为11810的 coord 节点，该节点将关联到指定的catalog 节点

<pre class="prettyprint lang-javascript">
var oma = new Oma("localhost", 11790);
oma.createCoord(11810,"/opt/sequoiadb/database/coord/11810",{catalogaddr:"ubuntu1:11823,ubuntu2:11823"});</pre>

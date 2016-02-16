##语法##
***oma.createData(< svcname >,< dbpath >,[ config obj ])***

在目标集群控制器（sdbcm）所在的机器中创建一个 standalone 节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |
| dbpath | string | 节点路径。 | 是 |
| config obj | Json 对象 | 节点配置信息，如配置日志大小，是否打开事务等，具体可参考数据库配置。 | 否 |

**Note:**

* oma对象为连接到目标（本地/远端机器）集群控制器（sdbcm）获得的连接对象。

## 示例##

* 在本地创建一个端口号为11820的 standalone 节点，指定日志文件大小为64MB

<pre class="prettyprint lang-javascript">
var oma = new Oma("localhost", 11790);
oma.createData(11820,"/opt/sequoiadb/standlone/11820",{logfilesz:64});</pre>

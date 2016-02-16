##语法##
***oma.createOM(< svcname >,< dbpath >,[ config obj ])***

在目标集群控制器（sdbcm）所在的机器中创建sdbom服务进程（SequoiaDB管理中心进程）。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |
| dbpath | string | 节点路径。 | 是 |
| config obj | Json 对象 | 节点配置信息，如配置日志大小，是否打开事务等，具体可参考数据库配置。 | 否 |

**Note:**

* oma对象为连接到目标（本地/远端机器）集群控制器（sdbcm）获得的连接对象。
* 一个集群只能归属于一个SequoiaDB管理中心管理，但一个SequoiaDB管理中心却可管理多个集群。一般只创建一个sdbom服务进程即可。

## 示例##

* 在本地中创建一个本地端口号为11780，http端口为8000的的sdbom进程。

<pre class="prettyprint lang-javascript">
var oma = new Oma("localhost", 11790);
oma.createOM(11780,"/opt/sequoiadb/database/sms/11780",{httpname:8000})</pre>

##语法##
***oma.createCoord(< svcname >,< dbpath >,[ config obj ])***

在集群中创建一个 coord 节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |
| dbpath | string | 节点路径。 | 是 |
| config obj | Json 对象 | 节点配置信息，如配置日志大小，是否打开事务等，具体可参考数据库配置。 | 否 |

**Note:**

* 在一个集群中可以创建多个节点，但是连个节点的端口号必须相差5以上，因为系统为每个节点后台控制了5个通信接口。

## 示例##

* 在集群中创建一个端口号为11810的 coord 节点，指定日志文件大小为64MB

<pre class="prettyprint lang-javascript">
oma.createCoord(11810,"/opt/sequoiadb/coord/11810",{logfilesz:64})</pre>

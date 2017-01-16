## 语法##
***rg.createNode(&lt;host&gt;,&lt;service&gt;,&lt;dbpath&gt;,[config])***

在分区组中创建节点。

**Note:**

只有在分区组启动之后，才能创建节点操作。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| host | string | 指定节点的主机名。 | 是 |
| service | int/string | 节点端口号。 | 是 |
| dbpath | string | 1.数据文件路径，用于存放节点数据文件，请确保数据管理员（安装时创建，默认为sdbadmin）用户有写权限； <br/> 2.如果配置路径不以“/”开头，数据文件存放路径将是数据库管理员用户(默认为sdbadmin)的主目录(默认为/home/sequoiadb) + 配置的路径。 | 是 |
| config | Json 对象 |  节点配置信息，如配置日志大小，是否打开事务等，具体可参考[数据库配置](SdbDoc_Cn/database_management/runtime_configuration.html)。 | 否 |

## 格式##

rg.createNode() 方法的定义格式有四个参数：host，service，dbpath，config，如上表所示，host，dbpath 为字符串类型，Service 类型支持 int 或 string ，必填；最后一个是 Json 对象，选填。

<pre class="prettyprint lang-diy">
("<主机名>","<端口号>","<节点路径>,"[{&lt;configParam&gt;:value,...}])</pre>

## 示例##

* 在分区组 group 中创建一个端口号为11800的节点，指定日志文件大小为64MB

<pre class="prettyprint lang-javascript">
> rg.createNode("vmsvr2-suse-x64",11800,"/opt/sequoiadb/data/11800",{logfilesz:64})</pre>

**Note:**

在一个分区组中能创建多个节点，但是连个节点的端口号必须相差5以上。因为系统为每个节点后台控制了5个通信接口。

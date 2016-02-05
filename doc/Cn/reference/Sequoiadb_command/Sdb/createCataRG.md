##语法##
***db.createCataRG(&lt;host&gt;,&lt;service&gt;,&lt;dbpath&gt;,[ config ])***

新建一个编目分区组，同时创建并启动一个编目节点。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| host | string | 指定编目节点的主机名。 | 是 |
| service | int/string | 指定编目节点的服务端口，请确保该端口号，以及往后延续的3个端口号未被占用；如设置为11800，请确保11800/11801/11802/11803端口都未被占用。 | 是 |
| dbpath | string | 数据文件路径，用于存放编目数据文件，需确保数据管理员（安装时创建，默认为 sdbadmin）用户有写权限。 | 是 |
| config | json | 参数为可选参数，用于配置更多细节参数，格式必须为 json 格式，参数参见数据库配置一节；如需要配置日志大小参数{logfilesz:64}。 | 否 |

##格式##

createCataRG() 方法的定义格式有 host，service，dbpath，config 四个参数，host，dbpath 为字符串类型，Service 类型支持 int 或 string ，config 为 json 对象，格式如下：

<pre class="lang-diy">
{"<主机名>",<端口号>,"<数据文件路径>",[数据库配置参数对象]}</pre>

**Note:**

-    如果配置路径不以“/”开头，数据文件存放路径将是数据库管理员用户(默认为sdbadmin)的主目录(默认为/home/sequoiadb) + 配置的路径。
-    请确保数据文件存放路径的权限，如果 SequoiaDB 采用的默认安装，那么给路径赋予 sdbadmin 权限。
-    Service 目前建议直接采用port。


##示例##

- 在名为：sdbserver1 的主机上创建一个编目节点组，服务端口为：11800，数据文件存放路径为：/opt/sequoiadb/database/cata/11800

<pre class="prettyprint lang-javascript">
> db.createCataRG("sdbserver1", 11800,"/opt/sequoiadb/database/cata/11800")</pre>

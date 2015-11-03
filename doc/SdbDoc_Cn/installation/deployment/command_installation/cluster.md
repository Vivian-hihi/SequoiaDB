##集群模式的配置与启动##

集群模式是启动 SequoiaDB 的标准模式，至少需要三个节点。

在集群环境下，SequoiaDB 数据库需要三种角色的节点，分别为：

-   [数据节点](SdbDoc_Cn/infrastructure/data_node.html)
-   [编目节点](SdbDoc_Cn/infrastructure/catalog_node.html)
-   [协调节点](SdbDoc_Cn/infrastructure/coord_node.html)

集群模式的最小配置中，每种角色的节点至少启动一个，才能构成完整的集群模式。

集群模式中客户端或应用程序直接连接到协调节点，其余数据节点与编目节点对应用程序完全透明。

应用程序本身不需关心数据存放在哪个数据节点，协调节点会对接收到的请求解析，自动将其发送到需要的数据节点上进行处理。

在集群模式下，复制组之间的数据无共享，复制组内的节点间进行异步数据复制，保证数据的最终一致性。

**Note:**

在配置集群模式时，请先确保服务器与主机名的映射关系正确，详细请参考[系统配置需求](SdbDoc_Cn/installation/system.html) ，确保各节点之间能相互通信，将节点的防火墙关闭。

**说明：**

（1）本节按照高可用部署为例，介绍配置和启动步骤；

（2）以下操作步骤假设 SequoiaDB 程序安装在 /opt/sequoiadb 目录下；

（3）sdb服务进程全部以 sdbadmin 用户运行，请确保所有数据库目录都赋予 sdbadmin 读写权限。

-   步骤一：检查 SequoiaDB 的配置服务状态在每台数据库服务器上检查 SequoiaDB 配置服务状态：

<pre class="prettyprint lang-javascript">
$ service sdbcm status</pre>

确认系统提示“sdbcm is running”表示服务正在运行，否则请执行如下命令重新配置服务程序：

<pre class="prettyprint lang-javascript">
$ service sdbcm start</pre>

-   步骤二：启动一个临时协调节点（该节点只是为了创建其它节点而临时使用，后面会删除）

1.切换到 sdbadmin 用户

<pre class="prettyprint lang-javascript">
$ su sdbadmin</pre>

2.在任意一台数据库服务器上（以下步骤都只需要在这台服务器上操作），启动 SequoiaDB Shell 控制台

<pre class="prettyprint lang-javascript">
$ sdb</pre>

3.连接到本地的集群管理服务进程 sdbcm

<pre class="prettyprint lang-javascript">
> var oma = new Oma("localhost", 11790)</pre>

4.创建临时协调节点

<pre class="prettyprint lang-javascript">
> oma.createCoord(18800, "/opt/sequoiadb/database/coord/18800")</pre>

5.启动临时协调节点

<pre class="prettyprint lang-javascript">
> oma.startNode(18800)</pre>

-   步骤三：通过命令配置和启动编目节点

1.连接到临时协调节点，在 shell 命令中输入：

<pre class="prettyprint lang-javascript">
> var db = new Sdb("localhost",18800)</pre>

其中18800为协调节点端口号

2.创建一个编目节点组

<pre class="prettyprint lang-javascript">
> db.createCataRG("sdbserver1", 11800, "/opt/sequoiadb/database/cata/11800")</pre>


**sdbserver1：**第一个服务器主机名；

**11800：**为编目节点服务端口（该端口配置不要与随机端口冲突，以下其它端口的配置也需要注意）；

**/opt/sequoiadb/database/cata/11800：**为编目节点的数据文件存放路径；

**Note: **

请确保存放路径的权限，如果 SequoiaDB 采用的默认安装，那么给路径赋予 sdbadmin 权限，下同。

3.添加另外两个编目节点

<pre class="prettyprint lang-javascript">
> var cataRG = db.getRG("SYSCatalogGroup");
> var node1 = cataRG.createNode("sdbserver2", 11800,"/opt/sequoiadb/database/cata/11800")
> var node2 = cataRG.createNode("sdbserver3", 11800,"/opt/sequoiadb/database/cata/11800")</pre>

4.启动编目节点组

<pre class="prettyprint lang-javascript">
> node1.start()
> node2.start()</pre>

**Note:**

创建节点的第一个参数必须为“主机名”，而不能使主机的 IP。

-   步骤四：通过命令配置和启动数据节点

1.创建数据节点组

<pre class="prettyprint lang-javascript">
> var dataRG = db.createRG("datagroup")</pre>

2.添加数据节点

<pre class="prettyprint lang-javascript">
> dataRG.createNode("sdbserver1", 11820, "/opt/sequoiadb/database/data/11820")
> dataRG.createNode("sdbserver2", 11820, "/opt/sequoiadb/database/data/11820")
> dataRG.createNode("sdbserver3", 11820, "/opt/sequoiadb/database/data/11820")</pre>

**Note:**

创建节点的第一个参数必须为“主机名”，而不能是主机的 IP。

3.启动数据节点组

<pre class="prettyprint lang-javascript">
> dataRG.start()</pre>

-   步骤五：部署启动协调节点

1.创建协调节点组

<pre class="prettyprint lang-javascript">
> var rg = db.createCoordRG()</pre>

2.创建协调节点

<pre class="prettyprint lang-javascript">
> rg.createNode("sdbserver1", 11810, "/opt/sequoiadb/database/coord/11810")
> rg.createNode("sdbserver2", 11810, "/opt/sequoiadb/database/coord/11810")
> rg.createNode("sdbserver3", 11810, "/opt/sequoiadb/database/coord/11810")</pre>

3.启动协调节点

<pre class="prettyprint lang-javascript">
> rg.start()</pre>

-   步骤六：删除临时协调节点

1.连接到本地的集群管理服务进程 sdbcm

<pre class="prettyprint lang-javascript">
> var oma = new Oma("localhost", 11790)</pre>

2.删除临时协调节点

<pre class="prettyprint lang-javascript">
> oma.removeCoord(18800)</pre>

-   数据库配置启动完成
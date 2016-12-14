##独立模式的配置与启动##

-   独立模式是启动 SequoiaDB的最精简模式，仅需要启动一个独立模式的[数据节点](SdbDoc_Cn/infrastructure/data_node.html)，即可进行数据服务。

-   在独立模式中，SequoiaDB数据库作为一个独立的进程不需要与其他除客户端以外的进程进行通讯。所有的数据均存放在数据节点内。以独立模式启动的数据库不可进行分区，也不可进行数据复制。因此，在对数据安全性要求较高的环境下不建议使用独立模式。  

-   独立模式的数据库中不存在编目信息。  

-   一般推荐在开发环境中使用独立模式，以减少对硬件资源的需求。  

**说明：**

（1）本节按照最简部署为例，介绍配置和启动步骤；

（2）以下操作步骤假设 SequoiaDB 程序安装在 /opt/sequoiadb 目录下；

（3）sdb 服务进程全部以 sdbadmin 用户运行，请确保所有数据库目录都赋予sdbadmin 读写权限。

-   切换到 sdbadmin 用户

	<pre class="prettyprint lang-javascript">
	$ su sdbadmin</pre>

-   启动 SequoiaDB Shell 控制台（下文以默认安装路径 /opt/sequoiadb 为例）

	<pre class="prettyprint lang-javascript">
	$ /opt/sequoiadb/bin/sdb</pre>

-   连接到本地的集群管理服务进程 sdbcm

	<pre class="prettyprint lang-javascript">
	> var oma = new Oma("localhost", 11790)</pre>

-   创建独立模式的数据节点

	<pre class="prettyprint lang-javascript">
	> oma.createData(11810, "/opt/sequoiadb/database/standalone/11810")</pre>

**Note:**

其中11810为数据库服务端口名，为避免出现端口冲突等问题，切勿将数据库端口配置在随机端口范围以内。如：多数Linux 默认随机端口范围为32768～61000，可将数据库端口配置在32767以下。

-   启动该节点

	<pre class="prettyprint lang-javascript">
	> oma.startNode(11810)</pre>

-   数据库配置启动完成
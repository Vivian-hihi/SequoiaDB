在安装 SequoiaDB 产品之前，请确保您选择的系统满足必须的操作系统，硬件，通信，磁盘和内存的要求。sdbpreck命令将检查系统是否满足安全先决条件。

##硬件要求##

+--------+----------------------------------------------------------------------+-----------------------------------------------------------------------+
| 需求项 | 要求                                                                 | 建议                                                                  |
+========+======================================================================+=======================================================================+
| CPU    |                                                                      | 建议采用 X64（64 位 AMD64 和 Intel EM64T 处理器 ）或者 PowerPC 处理器 |
|        |  -   x86（Intel Pentium、Intel Xeon 和 AMD）32位 Intel 和 AMD 处理器 |                                                                       |
|        |  -   x64（64位 AMD64 和 Intel EM64T 处理器）                         |                                                                       |
|        |  -   PowerPC 7 或者 PowerPC 7+ 处理器                                |                                                                       |
+--------+----------------------------------------------------------------------+-----------------------------------------------------------------------+
| 磁盘   | 至少 10GB 空间                                                       | 建议大于 100GB 磁盘空间                                               |
+--------+----------------------------------------------------------------------+-----------------------------------------------------------------------+
| 内存   | 至少 1GB                                                             | 大于 2GB 物理内存                                                     |
+--------+----------------------------------------------------------------------+-----------------------------------------------------------------------+
| 网卡   | 配备至少 1 张网卡                                                    | 建议至少配置 1GE 网卡                                                 |
+--------+----------------------------------------------------------------------+-----------------------------------------------------------------------+

##受支持的操作系统##

+----------------+------------------------------------------------------------+
| 系统类型       | 系统列表                                                   |
+================+============================================================+
| Linux          |                                                            |
|                | 	-   Red Hat Enterprise Linux (RHEL) 6                     |
|                | 	-   SUSE Linux Enterprise Server (SLES) 11 Service Pack 1 |
|                | 	-   SUSE Linux Enterprise Server (SLES) 11 Service Pack 2 |
|                | 	-   SUSE Linux Enterprise Server (SLES) 12 Service Pack 1 |
|                | 	-   Ubuntu 12                                             |
|                | 	-   CentOS 6                                              |
+----------------+------------------------------------------------------------+
| Power PC Linux |                                                            |
|                |  -   Red Hat Enterprise Linux (RHEL) 6                     |
|                |  -   SUSE Linux Enterprise Server (SLES)11 Service Pack 1  |
|                |  -   SUSE Linux Enterprise Server (SLES)11 Service Pack 2  |
+----------------+------------------------------------------------------------+

##软件要求##

###Linux 系统要求###

+--------------------------------+-------------------------------------------------------------+----------------------------------------------------+
| 配置项                         | 配置方法                                                    | 验证方法                                           |
+================================+=============================================================+====================================================+
| 配置主机名                     | 1.使用 root 权限登陆，执行 hostname sdbserver1              | 执行 hostname 命令，确认打印信息是否为“sdbserver1” |
|                                |（sdbserver1为主机名称，可根据需要修改。）；                 |                                                    |
+--------------------------------+-------------------------------------------------------------+----------------------------------------------------+
|                                | -   对于 SUSE：                                             |                                                    |
|                                |      2.  打开 /etc/HOSTNAME 文件；<br>                      |                                                    |
|                                |          vi /etc/HOSTNAME<br>                               |                                                    |
|                                |      3.  修改文件内容，配置为主机名称；<br>                 |                                                    |
|                                |          sdbserver1 （主机名称）<br>                        |                                                    |
|                                |      4.  按 : wq 保存退出；                                 |                                                    |
|                                | -   对于 RedHat：                                           |                                                    |
|                                |      2.  打开 /etc/sysconfig/network 文件；<br>             |                                                    |
|                                |          vi /etc/sysconfig/network <br>                     |                                                    |
|                                |      3.  将 HOSTNAME 一行修改为 HOSTNAME = sdbserver1，其中 |                                                    |
|                                |          sdbserver1 为新主机名；                            |                                                    |
|                                |      4.  按 : wq 保存退出；                                 |                                                    |
|                                | -   对于 Ubuntu：                                           |                                                    |
|                                |      2.  打开 /etc/hostname 文件；<br>                      |                                                    |
|                                |          vi /etc/hostname<br>                               |                                                    |
|                                |      3.  修改文件内容，配置为主机名称；<br>                 |                                                    |
|                                |          sdbserver1<br>                                     |                                                    |
|                                |      4.  按 : wq 保存退出；                                 |                                                    |
+--------------------------------+-------------------------------------------------------------+----------------------------------------------------+
| 配置物理机之间通过主机名可连接 |                                                             |                                                    |
|                                | -   使用 root 权限，打开 /etc/hosts 文件<br>                | 1.ping sdbserver1（本机主机名）可以 ping 通      |
|                                |     vi /etc/hosts<br>                                       | 2.ping sdbserver2（远端主机名）可以 ping 通      |
|                                | -   修改 /etc/hosts，将服务器节点的主机名与IP映射关系配置到 |                                                    |
|                                |     该文件中<br>                                            |                                                    |
|                                |     192.168.20.200 sdbserver1<br>                           |                                                    |
|                                |     192.168.20.201 sdbserver2<br>                           |                                                    |
|                                |     192.168.20.202 sdbserver3<br>                           |                                                    |
|                                | -   保存退出                                                |                                                    |
+--------------------------------+-------------------------------------------------------------+----------------------------------------------------+
| 关闭防火墙 (需要管理员权限)    |                                                             | -   对于 SUSE：                                    |
|                                | 	-   对于 SUSE：                                            | 		chkconfig -list &#124 grep fire；           |
|                                | 		1.  SuSEfirewall2 stop；                               | -   对于 RedHat：                                  |
|                                | 		2.  chkconfig SuSEfirewall2\_setup；                   | 		service iptables status；                   |
|                                | 	-   对于 RedHat：                                          | -   对于 Ubuntu：                                  |
|                                | 		1.  service iptables stop；                            | 		ufw status；                                |
|                                | 		2.  chkconfig iptables off；                           |                                                    |
|                                | 	-   对于 Ubuntu：                                          |                                                    |
|                                | 		1.  ufw disable；                                      |                                                    |
+--------------------------------+-------------------------------------------------------------+----------------------------------------------------+

**Note: **

1.每台作为数据库服务器的机器都需要配置;

2.社区版要求系统安装glibc 2.15以及libstdc++ 6.0.18以上版本。

###Linux 推荐配置###

-   调整 ulimit

    在配置文件 /etc/security/limits.conf 中设置：

<pre class="prettyprint lang-diy">
#&lt;domain&gt;      &lt;type&gt;    &lt;item&gt;     &lt;value&gt;
*                    soft        core             0
*                    soft        data             unlimited
*                    soft       fsize             unlimited
*                    soft         rss             unlimited
*                    soft          as             unlimited</pre>

**参数说明：**

**core**：数据库出现故障时产生 core 文件用于故障诊断，生产系统建议关闭；

**data**：数据库进程所允许分配的数据内存大小；

**fsize**：数据库进程所允许寻址的文件大小；

**rss**：数据库进程所允许的最大 resident set 大小；

**as**：数据库进程所允许最大虚拟内存寻址空间限制；

	在配置文件 /etc/security/limits.d/90-nproc.conf 中设置：

<pre class="prettyprint lang-diy">
#&lt;domain&gt;      &lt;type&gt;    &lt;item&gt;     &lt;value&gt;
*                   soft       nproc             unlimited</pre>

**参数说明：**

**nproc：**数据库所允许的最大线程数限制；

**Note:**

1.每台作为数据库服务器的机器都需要配置；

2.更改配置后需重新登录使得配置生效。

-   调整内核参数

	1.  使用下列命令输出当前 vm 配置，并将其归档保存：

	<pre class="prettyprint lang-javascript">
	> cat /proc/sys/vm/swappiness
	> cat /proc/sys/vm/dirty_ratio
	> cat /proc/sys/vm/dirty_background_ratio
	> cat /proc/sys/vm/dirty_expire_centisecs
	> cat /proc/sys/vm/vfs_cache_pressure
	> cat /proc/sys/vm/min_free_kbytes</pre>

	2.  添加下列参数至 /etc/sysctl.conf 文件调整内核参数：

	<pre class="prettyprint lang-diy">
	vm.swappiness = 0
	vm.dirty_ratio = 100
	vm.dirty_background_ratio = 40
	vm.dirty_expire_centisecs = 3000
	vm.vfs_cache_pressure = 200
	vm.min_free_kbytes = &lt;物理内存大小的8%，单位KB&gt;</pre>

	**Note: **
		
	当数据库可用物理内存不足 8GB 时不需使用 vm.swappiness = 0；上述 dirty 类参数只是建议值，具体系统设置时请按原则（控制系统的 flush 进程只采用脏页超时机制刷新脏页，而不采用脏页比例超支刷新脏页）进行设置。

	3.  执行如下命令，使配置生效：

	<pre class="prettyprint lang-javascript">
	/sbin/sysctl -p</pre>

	**Note:** 

	每台作为数据库服务器的机器都需要配置。

-   数据库目录结构

    用户应尽可能使数据目录，索引目录与日志目录存放在不同物理磁盘中，以减少顺序 I/O 与随机 I/O 之间的竞争。并且保证安装路径上的文件夹具有可读和可执行权限。
手工安装需要先分别在每一台主机上安装 SequoiaDB，然后再命令行安装部署集群。


##准备安装介质##

请到 SequoiaDB 官方网站下载相应的版本。

下载地址：[SequoiaDB](http://www.sequoiadb.com/cn/index.php?a=index&m=Download)


##安装 SequoiaDB##

###安装前准备###

-   确保系统满足硬件和软件要求
-   使用 root 用户权限来安装 SequoiaDB 数据库服务
-   检查 SequoiaDB 产品软件包与 OS 系统配套
-   如果需要图形界面模式安装，请确保 X Server 服务正在运行
-   服务器配置了主机名，且与其他服务器之间可通过主机名建立网络连接（如 ssh 主机名）

**Note:**

SequoiaDB 的安装向导需要的参数不接受非英文字符。

###安装步骤###

**说明：**

（1）产品包名字以 sequoiadb-2.0-linux-x86_64-installer.run 为例；

（2）步骤以命令行方式进行介绍，图形界面按照图像向导提示完成。

**Note:** 

（1）如果有多台服务器，每台机器都需要重复如下步骤安装服务器程序。

（2）需要确保安装路径（见下文介绍）的每一级文件夹都具有可读和可执行权限。

-   参照[系统配置需求](SdbDoc_Cn/installation/system.html)配置好主机名以及修改系统内核参数

-   运行安装程序

	<pre class="prettyprint lang-javascript">
	> ./sequoiadb-2.0-linux-x86_64-installer.run --SMS false</pre>

-   程序提示选择向导语言

	<pre class="prettyprint lang-diy">
	Language Selection
	Please select the installation language
	[1] English - English
	[2] Simplified Chinese - 简体中文
	Please choose an option [1] :2</pre>

-   输入2，选择中文，显示安装协议，默认忽略阅读，如果需要读取全部文件，输入2

	<pre class="prettyprint lang-diy">
	&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
	由 BitRockInstallBuilder 评估本所建立
	&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
	欢迎来到 SequoiaDB Server 安装程序


	重要信息：请仔细阅读

	下面提供了两个许可协议。

	1. SequoiaDB 评估程序的最终用户许可协议
	2. SequoiaDB 最终用户许可协议

	如果被许可方为了生产性使用目的（而不是为了评估、测试、试用“先试后买”或演示）获得本程序，单击下面的“接受”按钮即表示被许可方接受 SequoiaDB 最终用户许可协议，且不作任何修改。

	如果被许可方为了评估、测试、试用“先试后买”或演示（统称为“评估”）目的获得本程序：单击下面的“接受”按钮即表示被许可方同时接受（i）SequoiaDB 评估程序的最终用户许可协议（“评估许可”），且不作任何修改；和（ii）SequoiaDB 最终用户程序许可协议（SELA），且不作任何修改。

	在被许可方的评估期间将适用“评估许可”。

	如果被许可方通过签署采购协议在评估之后选择保留本程序（或者获得附加的本程序副本供评估之后使用），SequoiaDB 评估程序的最终用户许可协议将自动适用。

	“评估许可”和 SequoiaDB 最终用户许可协议不能同时有效；两者之间不能互相修改，并且彼此独立。

	这两个许可协议中每个协议的完整文本如下。

	评估程序的最终用户许可协议



	[1] 同意以上协议: 了解更多的协议内容，可以在安装后查看协议文件
	[2] 查看详细的协议内容
	请选择选项 [1] :</pre>

-   是否同意协议：

	<pre class="prettyprint lang-diy">
	&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
	同意以上协议

	按 [Enter] 继续：

	您是否接受此软件授权协议？ [y/n]:</pre>

-   按 y 表示同意：

	<pre class="prettyprint lang-diy">
	&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
	请指定 SequoiaDBServer 将会被安装到的目录
	安装目录 [/opt/sequoiadb]:</pre>

-   输入安装路径后按回车（默认安装在 /opt/sequoiadb ），此时系统提示输入用户名，该用户名用于运行 SequoiaDB 服务

	<pre class="prettyprint lang-diy">
	&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
	数据库管理用户配置
	配置用于启动 SequoiaDB 的用户名和密码
	用户名[sdbadmin]:</pre>

-   输入用户名后按回车（默认创建 sdbadmin
    用户），此时系统提示输入该用户的密码和确认密码

	<pre class="prettyprint lang-diy">
	密码 [********] :
	确认密码 [********] :</pre>

-   输入两次密码后（默认密码为 sdbadmin），此时系统提示输入配置服务端口

	<pre class="prettyprint lang-diy">
	&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
	集群管理服务端口配置
	配置SequoiaDB集群管理服务端口，集群管理用于远程启动添加和启停数据库节点
	端口 [11790]:</pre>

**Note: **

所有服务器的配置服务端口必须相同。

-   输入端口（默认为11790），系统提示开始安装，需要用户确认

-   询问是否允许 SequoiaDB 相关进程开机自启动

	<pre class="prettyprint lang-diy">
	&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
	是否允许 SequoiaDB 相关进程开机自启动</pre>

-   SequoiaDB 相关进程开机自启动 [Y/n]：Y，输入 Y，按回车，同意 SequoiaDB 相关进程开机自启动

	<pre class="prettyprint lang-diy">
	正在安装 SequoiaDB Server 于您的电脑中，请稍候。
	安装中
	0% ______________ 50% ______________ 100%
	#########################################
	&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
	安装程序已经完成安装 SequoiaDB Server 于你的电脑中.</pre>

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

-   数据库配置启动完成##集群模式的配置与启动##

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
$ /opt/sequoiadb/bin/sdb</pre>

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

如果配置路径不以“/”开头，数据文件存放路径将是数据库管理员用户(默认为sdbadmin)的主目录(默认为/home/sequoiadb) + 配置的路径。
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

-   数据库配置启动完成可视化安装只需要选择一台机器，并在该机器上安装 OM 服务，便可以通过网页连接 OM，进行可视化安装部署集群。


##准备安装介质##  

请到 [SequoiaDB](http://www.sequoiadb.com/cn/index.php?a=index&m=Download) 官方网站下载相应的版本。


##安装 OM 服务##

###安装前准备###

-   确保系统满足硬件和软件要求
-   使用 root 用户权限来安装 SequoiaDB 数据库服务
-   检查 SequoiaDB 产品软件包与 OS 系统配套
-   如果需要图形界面模式安装，请确保 X Server 服务正在运行
-   服务器配置了主机名，且与其他服务器之间可通过主机名建立网络连接（如 ssh 主机名）

**Note: **

SequoiaDB 的安装向导需要的参数不接受非英文字符。

###安装步骤###

**说明：**

（1）产品包名字以 sequoiadb-2.0-linux-x86_64-installer.run 为例；

（2）步骤以命令行方式进行介绍，图形界面按照图像向导提示完成。

**Note: **

（1）如果有多台服务器，只需要选择其中一台作为控制机，执行以下操作，安装 OM 服务。

（2）需要确保安装路径（见下文介绍）的每一级文件夹都具有可读和可执行权限。

-   参照[系统配置需求](SdbDoc_Cn/installation/system.html)配置好主机名以及修改系统内核参数

-   运行安装程序

    <pre class="prettyprint lang-javascript">
    ./sequoiadb-2.0-linux-x86_64-installer.run --SMS true</pre>

-   程序提示选择向导语言

    <pre class="prettyprint lang-diy">
    Language Selection
    Please select the installation language
    [1] English - English
    [2] Simplified Chinese - 简体中文
    Please choose an option [1] :2</pre>

-   输入2，选择中文，显示安装协议，默认忽略阅读，如果需要读取全部文件，输入2

    <pre class="prettyprint lang-diy">
    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
    由 BitRockInstallBuilder 评估本所建立
    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
    欢迎来到 SequoiaDB Server 安装程序


    重要信息：请仔细阅读

    下面提供了两个许可协议。

    1. SequoiaDB 评估程序的最终用户许可协议
    2. SequoiaDB 最终用户许可协议

    如果被许可方为了生产性使用目的（而不是为了评估、测试、试用“先试后买”或演示）获得本程序，单击下面的“接受”按钮即表示被许可方接受 SequoiaDB 最终用户许可协议，且不作任何修改。

    如果被许可方为了评估、测试、试用“先试后买”或演示（统称为“评估”）目的获得本程序：单击下面的“接受”按钮即表示被许可方同时接受（i）SequoiaDB 评估程序的最终用户许可协议（“评估许可”），且不作任何修改；和（ii）SequoiaDB 最终用户程序许可协议（SELA），且不作任何修改。

    在被许可方的评估期间将适用“评估许可”。

    如果被许可方通过签署采购协议在评估之后选择保留本程序（或者获得附加的本程序副本供评估之后使用），SequoiaDB 评估程序的最终用户许可协议将自动适用。

    “评估许可”和 SequoiaDB 最终用户许可协议不能同时有效；两者之间不能互相修改，并且彼此独立。

    这两个许可协议中每个协议的完整文本如下。

    评估程序的最终用户许可协议



    [1] 同意以上协议: 了解更多的协议内容，可以在安装后查看协议文件
    [2] 查看详细的协议内容
    请选择选项 [1] :</pre>

-   是否同意协议：

    <pre class="prettyprint lang-diy">
    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
    同意以上协议

    按 [Enter] 继续：

    您是否接受此软件授权协议？ [y/n]:</pre>

-   按 y 表示同意：

    <pre class="prettyprint lang-diy">
    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
    请指定 SequoiaDBServer 将会被安装到的目录
    安装目录 [/opt/sequoiadb]:</pre>

-   输入安装路径后按回车（默认安装在/opt/sequoiadb），此时系统提示输入用户名，该用户名用于运行 SequoiaDB 服务

    <pre class="prettyprint lang-diy">
    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
    数据库管理用户配置
    配置用于启动 SequoiaDB 的用户名和密码
    用户名[sdbadmin]:</pre>

-   输入用户名后按回车（默认创建 sdbadmin 用户），此时系统提示输入该用户的密码和确认密码

    <pre class="prettyprint lang-diy">
    密码 [********] :
    确认密码 [********] :</pre>

-   输入两次密码后（默认密码为 sdbadmin），此时系统提示输入配置服务端口

    <pre class="prettyprint lang-diy">
    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
    集群管理服务端口配置
    配置SequoiaDB集群管理服务端口，集群管理用于远程启动添加和启停数据库节点
    端口 [11790]:</pre>

**Note:** 所有服务器的配置服务端口必须相同。

-   输入端口（默认为11790），系统提示开始安装，需要用户确认

-   询问是否允许 SequoiaDB 相关进程开机自启动

    <pre class="prettyprint lang-diy">
    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
    是否允许 SequoiaDB 相关进程开机自启动</pre>

-   SequoiaDB 相关进程开机自启动 [Y/n]：Y，输入 Y，按回车，同意 SequoiaDB 相关进程开机自启动

    <pre class="prettyprint lang-diy">
    正在安装 SequoiaDB Server 于您的电脑中，请稍候。
    安装中
    0% ______________ 50% ______________ 100%
    #########################################
    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
    安装程序已经完成安装 SequoiaDB Server 于你的电脑中.</pre>

安装完成后，OM 会自动启动并开启8000端口的 web 服务，用户可以通过浏览器登陆 OM，并进行集群的部署。假设安装 OM 的机器 IP 为192.168.10.10，则在浏览器键入 http://192.168.1.100:8000，访问 OM 服务。


##自动化部署##

OM 支持 IE7/8/9+、chrome、firefox 等主流浏览器。

-   步骤一：登录 OM

    1.1 浏览器访问，访问地址为 OM 的地址，访问端口默认8000

    1.2 例如 http://192.168.1.100:8000

    1.3 登录用户名默认 admin，初始密码 admin

    1.4 输入默认密码 admin

    1.5 点击 <**登录**> 按钮

    ![](s1.jpg)

-   步骤二：开始部署

    2.1 点击导航栏的 <**部署引导**>

    ![](s21.jpg)

    2.2 集群的参数都有默认值，除了 <**集群名**> 和 <**业务名**>，基本上不需要修改其他参数，点击 <**高级选项**>，可以设置更多的参数。修改后点击 <**确定**>。

    ![](s22.jpg)

-   步骤三：添加主机

    3.1 在左边操作栏，输入<**地址**>、<**用户名**>、<**密码**>、<**SSH端口**>，可以扫描出添加的主机。<**地址**> 不仅支持 IP、HostName 扫描主机，还可以通过 IP 段和 HostName 段，<**地址**> 可以参考左边操作栏的提示，点击 <**帮助**>。

    ![](s31.jpg)

    3.2 在左边操作栏，输入
    <**地址**>、<**用户名**>、<**密码**>、<**SSH 端口**>，然后点击 <**扫描**>。

    ![](s32.jpg)

    3.3 扫描完成后，扫描结果会在右边显示出来，默认正常连接的主机都会选上，有问题的主机不会选择，点击有问题的主机，可以重新输入参数，点击输入框以外的地方，自动重新扫描主机。

    ![](s331.jpg)

    ![](s332.jpg)

    3.4 在主机列表选择要添加的主机，图中3台主机都添加，因此全选。点击 <**下一步**>。

    ![](s34.jpg)

    3.5 等待 OM 获取主机信息。

    ![](s35.jpg)

    3.6 左边是主机列表栏，右边是主机的详细信息，点击左边列表的主机，可以显示该主机信息。

    ![](s36.jpg)

    3.7 主机列表，蓝色提示已选择的磁盘数，黄色提示该主机有4个警告，当鼠标移到提示中，会自动显示内容。（注意：图中3台主机都有1个磁盘是满足容量的）

    ![](s37.jpg)

    3.8 检查主机没有问题，点击 <**下一步**>，等待完成。完成后会自动进入下一步操作。

    ![](s381.jpg)

    ![](s382.jpg)

-   步骤四：创建业务

    4.1 配置业务信息，可以通过下拉菜单选择 <**集群模式**> 和 <**独立模式**>。

    ![](s41.jpg)

    4.2 <**高级选项**>，可以选择指定的1台或多台主机安装业务，默认由系统自动分配。

    ![](s42.jpg)

    4.3 设置好参数后，点击 <**下一步**>。

    ![](s43.jpg)

    4.4 左边是业务和每一个分区组信息，可以通过业务添加分区组，通过分区组添加删除节点、删除分区组。（注意：协调组和编目组是不能删除的）

    ![](s441.jpg)

    ![](s442.jpg)

    ![](s443.jpg)

    ![](s444.jpg)

    4.5 右边是节点列表，点击列表每一行，都可以修改该行节点的配置。

    ![](s45.jpg)

    4.6
    批量修改节点配置，可以在节点列表每一行的方框打勾来指定要修改的节点，也可以点击 <**选择操作**> - <**全选**> - <**反选**> 来选择，然后点击 <**已选定操作**> - <**修改节点配置**>进行修改。

    ![](s46.jpg)

    4.7
    节点列表第二行是查找条件，通过输入参数，可以快速找到指定条件的节点。

    ![](s471.jpg)

    ![](s472.jpg)

    ![](s473.jpg)

    4.8 批量修改节点配置，<**数据路径**> 和 <**服务名**> 可以通过规则来设置。点击左边 <**提示**> - <**帮助**>，有详细说明。

    ![](s481.jpg)

    ![](s482.jpg)

    4.9 业务配置修改完成后，点击 <**下一步**>，开始安装业务。

    ![](s49.jpg)

-   步骤五：安装业务

    5.1 开始安装业务

    ![](s511.jpg)

    ![](s512.jpg)

    ![](s513.jpg)

    ![](s514.jpg)

    5.2 安装完成，点击 <**返回**>。

    ![](s52.jpg)

    5.3 业务安装完成。

    ![](s53.jpg)

-   步骤六：主页左边是集群列表，右边是集群实时监控信息

    ![](m1.jpg)

-   步骤七：创建集群

    7.1 点击 <**状态**> 旁边的下拉菜单；

    ![](m21.jpg)

    7.2 点击 <**创建集群**>；

    ![](m22.jpg)

    7.3 输入集群参数，点击 <**高级选项**> 可以设置更多参数；

    ![](m231.jpg)

    ![](m232.jpg)

    7.4 点击 <**确定**>，完成创建。

    ![](m24.jpg)

-   步骤八：查看业务信息和删除业务

    8.1 查看当前已安装的业务（图中 myBusiness 是已经安装的一个业务，myBusiness 是业务名）；

    ![](m31.jpg)

    8.2 点击表格 <**业务**> 查看业务信息（一）；

    ![](m321.jpg)

    ![](m322.jpg)

    8.3 点击业务名旁边的下拉菜单，点击 <**业务列表**> 看业务信息（二）；

    ![](m331.jpg)

    ![](m332.jpg)

    ![](m333.jpg)

    8.4 删除业务，在业务列表，点击业务对应的 <**删除业务**> 按钮，弹出警告窗口，点击 <**确定**> 开始删除业务，等待卸载完成，完成后弹出提示，点击 <**返回**>。

    ![](m341.jpg)

    ![](m342.jpg)

    ![](m343.jpg)

    ![](m344.jpg)

    ![](m345.jpg)

    ![](m346.jpg)

-   步骤九：查看集群主机数量和主机详细信息，删除主机

    9.1 点击要查看的集群名；

    ![](m41.jpg)

    9.2 鼠标移动到主机旁边蓝色的徽章，徽章上面的数字就是当前主机数量；

    ![](m42.jpg)

    9.3 表格第二列是显示当前状态信息，黄色表示 warning，红色表示 danger；

    ![](m431.jpg)

    ![](m432.jpg)

    9.4 主页右边的 <**实时监控**>，监控集群的平均 CPU 使用率，平均内存使用率，平均磁盘使用率，网络入口流量，网络出口流量，网络每秒接收数据包个数，网络每秒发送数据包个数；

    ![](m44.jpg)

    9.5 点击表格 <**主机**>，查看主机列表；

    ![](m451.jpg)

    ![](m452.jpg)

    9.6 点击业务名旁边的下拉菜单，点击 <**主机列表**>查看主机列表；

    ![](m461.jpg)

    ![](m462.jpg)

    ![](m463.jpg)

    9.7 删除主机，选择要删除的主机，点击 <**已选定操作**>，点击 <**删除主机**>，弹出警告窗口，点击 <**确定**> 开始删除主机，等待完成，完成后弹出删除结果，点击 <**关闭**>。

    ![](m471.jpg)

    ![](m472.jpg)

    ![](m473.jpg)

    ![](m474.jpg)

    ![](m475.jpg)

    ![](m476.jpg)

-   步骤十：删除集群

    10.1 回到主页，点击要删除的集群旁边下拉菜单；

    ![](m51.jpg)

    10.2 点击菜单的 <**删除集群**>；

    ![](m52.jpg)

    10.3 删除完成。

    ![](m53.jpg)
通过OM部署好数据库之后，就可以进行数据库操作了。

##推荐浏览器##

支持IE7/8/9/10/11, Microsoft Edge, Chrome, Firefox等。

推荐使用Chrome, Microsoft Edge, Firefox。

##安装 OM 服务##

安装OM服务、部署SequoiaDB数据库，请参考 [可视化安装](SdbDoc_Cn/installation/deployment/visualization_installation.html) 。

###进入数据库操作###

-   在首页点击集群对应的&lt;**业务**&gt;，进入业务列表页面。

    点击myCluster集群的&lt;**业务**&gt;。

    ![](visualization_data_operation_cn_1.jpg)

-   点击要进行操作的&lt;**业务名**&gt;，进入数据操作页面。

    点击myModule业务。

    ![](visualization_data_operation_cn_2.jpg)
    
-   点击&lt;**数据库操作**&gt;
    
    ![](visualization_data_operation_cn_11.jpg)

###进入数据操作###

-   在首页点击集群对应的&lt;**业务**&gt;，进入业务列表页面。

    如图，点击myCluster集群的&lt;**业务**&gt;。

    ![](visualization_data_operation_cn_1.jpg)

-   点击要进行操作的&lt;**业务名**&gt;，进入数据操作页面。

    如图，点击myModule业务。

    ![](visualization_data_operation_cn_2.jpg)
    
-   如图，点击**集合名**、**记录数**都可以进入记录操作页面，点击Lob数则进入Lob操作页面。
    
    ![](visualization_data_operation_cn_3.jpg)

**Note: **

-   新界面的设计采用更合理的方式展示信息。

    1. 所有**<span style="color:#00BFDD">天蓝色</span>**的字体、按钮都是可点击的。

    ![](visualization_data_operation_cn_4.jpg)

    ![](visualization_data_operation_cn_5.jpg)

    2. 所有出现 ![](visualization_data_operation_cn_7.jpg) 图标的，当鼠标停在图标上，都会出现相关提示。

    ![](visualization_data_operation_cn_6.jpg)

    3. 当出现 **...** 省略符时，把鼠标停留在上面，会出现完整的信息。

    ![](visualization_data_operation_cn_8.jpg)

    ![](visualization_data_operation_cn_9.jpg)###页面简介###

![](visualization_data_operation_cn_10.jpg)

###创建集合空间###

-   

    **准备工作： 创建SequoiaDB业务。**

    1. 点击&lt;**数据库操作**&gt;。

    ![](visualization_data_operation_cs_cn_1.jpg)

    2. 点击&lt;**创建集合空间**&gt;。

    ![](visualization_data_operation_cs_cn_2.jpg)

    3. 填写新建集合空间的名字，其他参数不是必填项，可以根据实际情况设置。

    ![](visualization_data_operation_cs_cn_3.jpg)

    4. 其他参数说明，请参考 [数据模型 - 集合空间](SdbDoc_Cn/data_model/collectionspace.html) 。

    5. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_cs_cn_4.jpg)

    6. 创建成功;。

    ![](visualization_data_operation_cs_cn_5.jpg)

###创建集合（普通类型）###

-   

    **准备工作： 创建SequoiaDB业务、创建集合空间。**

    1. 点击&lt;**创建集合**&gt;。

    ![](visualization_data_operation_cl_cn_1.jpg)
 
    2. 选择创建集合所属的集合空间。因为只有刚刚创建的**集合空间foo**，所以自动选择了**foo**。

    ![](visualization_data_operation_cl_cn_2.jpg)

    3. 现在要创建一个 **普通类型** 的集合，因此&lt;**集合类型**&gt;不需要修改，填写新建集合的名字。

    **Note: ** 集合名可以自由取。

    ![](visualization_data_operation_cl_cn_3.jpg)

    4. 其他参数说明，请参考 [数据模型 - 集合](SdbDoc_Cn/data_model/collection.html) 。

    5. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_cl_cn_4.jpg)

    6. 等待创建结束。

    ![](visualization_data_operation_cl_cn_5.jpg)

    7. 创建成功。

    ![](visualization_data_operation_cl_cn_6.jpg)

###创建集合（水平范围分区）###

-   

    **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间。**

    1. 点击&lt;**创建集合**&gt;。

    ![](visualization_data_operation_cl_cn_1.jpg)

    2. 选择创建集合所属的集合空间。这里仍然默认选**foo**。

    ![](visualization_data_operation_cl_cn_2.jpg)

    3. **集合类型** 选 **水平范围分区**。

    ![](visualization_data_operation_cl_cn_7.jpg)

    4. 填写新建集合的名字。

    **Note: ** 集合名可以自由取。

    ![](visualization_data_operation_cl_cn_8.jpg)

    5. 填写分区键，这里演示取 **id** 和 **time**，详细说明参考 [基本操作 - 数据分区 - 分区键](SdbDoc_Cn/basic_operation/sharding/shardingkey.html) 。

    **Note: ** 分区键可以有多个，通过后面的 **“+”** 和 **“-”** 可以添加或删除分区键。

    ![](visualization_data_operation_cl_cn_9.jpg)

    ![](visualization_data_operation_cl_cn_10.jpg)

    ![](visualization_data_operation_cl_cn_11.jpg)
    
    6. 其他参数说明，请参考 [数据模型 - 集合](SdbDoc_Cn/data_model/collection.html) 。

    7. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_rangecl_cn_1.jpg)

    8. 等待创建结束。

    ![](visualization_data_operation_cl_cn_5.jpg)

    9. 创建成功。

    ![](visualization_data_operation_cl_cn_12.jpg)

###创建集合（水平散列分区）###

-   

    **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间。**

    1. 点击&lt;**创建集合**&gt;。

    ![](visualization_data_operation_cl_cn_1.jpg)

    2. 选择创建集合所属的集合空间。这里仍然默认选**foo**。

    ![](visualization_data_operation_cl_cn_2.jpg)

    3. **集合类型** 选 **水平散列分区**。

    ![](visualization_data_operation_cl_cn_18.jpg)

    4. 填写新建集合的名字。

    **Note: ** 集合名可以自由取。

    ![](visualization_data_operation_cl_cn_13.jpg)

    5. 填写分区键，这里演示取 **id** 和 **time**，详细说明参考 [基本操作 - 数据分区 - 分区键](SdbDoc_Cn/basic_operation/sharding/shardingkey.html) 。

    **Note: ** 分区键可以有多个，通过后面的 **“+”** 和 **“-”** 可以添加或删除分区键。

    ![](visualization_data_operation_cl_cn_14.jpg)

    ![](visualization_data_operation_cl_cn_15.jpg)

    ![](visualization_data_operation_cl_cn_16.jpg)

    6. 其他参数说明，请参考 [数据模型 - 集合](SdbDoc_Cn/data_model/collection.html) 。

    7. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_hashcl_cn_1.jpg)

    8. 等待创建结束。

    ![](visualization_data_operation_cl_cn_5.jpg)

    9. 创建成功。

    ![](visualization_data_operation_cl_cn_17.jpg)

###创建集合（垂直分区）###

-   

    **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间。**

    1. 点击&lt;**创建集合**&gt;。

    ![](visualization_data_operation_cl_cn_1.jpg)

    2. 选择创建集合所属的集合空间。这里仍然默认选**foo**。

    ![](visualization_data_operation_cl_cn_2.jpg)

    3. **集合类型** 选 **垂直分区**。

    ![](visualization_data_operation_maincl_cn_1.jpg)

    4. 填写新建集合的名字。

    **Note: ** 集合名可以自由取。

    ![](visualization_data_operation_maincl_cn_2.jpg)

    5. 填写分区键，这里演示取 **id** 和 **time**，详细说明参考 [基本操作 - 数据分区 - 分区键](SdbDoc_Cn/basic_operation/sharding/shardingkey.html) 。

    **Note: ** 分区键可以有多个，通过后面的 **“+”** 和 **“-”** 可以添加或删除分区键。

    ![](visualization_data_operation_maincl_cn_3.jpg)

    ![](visualization_data_operation_maincl_cn_4.jpg)

    ![](visualization_data_operation_maincl_cn_5.jpg)

    6. 其他参数说明，请参考 [数据模型 - 集合](SdbDoc_Cn/data_model/collection.html) 。

    7. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_maincl_cn_6.jpg)

    8. 等待创建结束。

    ![](visualization_data_operation_cl_cn_5.jpg)

    9. 创建成功。

    ![](visualization_data_operation_maincl_cn_7.jpg)

###切分数据###

-   

    **准备工作： 创建集群模式的SequoiaDB业务(必须有2个或以上的分区组)、创建集合空间、创建水平分区的集合并且插入一定数量的记录。**

    **Note:** 列表中必须至少有1个水平分区集合，才可以做**切分数据**操作。

    ![](visualization_data_operation_split_cn_1.jpg)

    1. 点击&lt;**切分数据**&gt;。

    ![](visualization_data_operation_split_cn_2.jpg)


    2. 选择 **切分方式**，实际根据需求选择，这里使用默认**百分比切分**。

    ![](visualization_data_operation_split_cn_3.jpg)

    3. 选择要**切分数据**的集合。

    ![](visualization_data_operation_split_cn_4.jpg)

    4. 设置**源分区组**和**目标分区组**。

    ![](visualization_data_operation_split_cn_5.jpg)

    ![](visualization_data_operation_split_cn_6.jpg)

    5. 设置**百分比切分**。

    ![](visualization_data_operation_split_cn_7.jpg)

    6. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_split_cn_8.jpg)

    7. 等待切分操作结束。

    ![](visualization_data_operation_split_cn_9.jpg)

    8. 切分数据完成。

    ![](visualization_data_operation_split_cn_10.jpg)

    ![](visualization_data_operation_split_cn_11.jpg)


###挂载###

-   

    **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间、创建1个垂直分区的集合、创建1个或更多普通集合或水平分区集合。**

    **Note:** 列表中必须至少有1个垂直分区集合，1个普通集合或水平分区集合，才可以做**挂载**操作。

    ![](visualization_data_operation_attach_cn_1.jpg)

    1. 点击&lt;**挂载集合**&gt;。

    ![](visualization_data_operation_attach_cn_2.jpg)

    2. 选择<垂直分区>的**集合**。

    ![](visualization_data_operation_attach_cn_3.jpg)

    3. 选择要挂载的**集合**。

    ![](visualization_data_operation_attach_cn_4.jpg)

    4. 填写分区范围，这里演示取 **time**。

    **Note: ** 分区范围可以有多个字段，通过后面的 **“+”** 和 **“-”** 可以添加或删除。

    把 **foo.y2014** 挂载到 **foo.main**。

    ![](visualization_data_operation_attach_cn_5.jpg)

    把 **foo.y2015** 挂载到 **foo.main**。

    ![](visualization_data_operation_attach_cn_6.jpg)

    5. 点击&lt;**确定**&gt;。

    6. 挂载完成。

    ![](visualization_data_operation_attach_cn_7.jpg)

    7. 如果不希望在列表中呈现子集合，可以设置**屏蔽子集合**。

    ![](visualization_data_operation_attach_cn_8.jpg)###页面简介###

![](visualization_data_operation_record_cn_1.jpg)


###插入(Insert)###

-   

    1. 点击 &lt;**插入**&gt; 按钮。

    ![](visualization_data_operation_insert_cn_1.jpg)

    2. 界面。

    ![](visualization_data_operation_insert_cn_2.jpg)

    3. 通过图形化构建一条记录。 { "name": "Jack", "age": 22, "phone": [ "123", "456" ], "time": { "$date": "2015-01-01" } }

    4. 添加一个新字段，修改字段名为 name，修改值为 Jack。

    ![](visualization_data_operation_insert_cn_3.jpg)

    ![](visualization_data_operation_insert_cn_4.jpg)

    5. 添加一个新字段，修改字段名为 age，修改类型为 **Auto** ，修改值为 22。

    ![](visualization_data_operation_insert_cn_5.jpg)

    6. 添加一个新字段，修改字段名为 phone，修改类型为 **Array** 。

    ![](visualization_data_operation_insert_cn_6.jpg)

    7. 在 phone 添加2个新数组元素，修改值为 123 和 456。

    ![](visualization_data_operation_insert_cn_7.jpg)

    8. 添加一个新字段，修改字段名为 time，修改类型为 **Date** ，修改值为 2015-01-01。

    ![](visualization_data_operation_insert_cn_8.jpg)

    ![](visualization_data_operation_insert_cn_9.jpg)

    9. 点击 &lt;**确定**&gt;。

    ![](visualization_data_operation_insert_cn_10.jpg)

    10. 除了使用图形化构建记录，也可以切换成字符串模式，输入一条Json也是可以的。

    ![](visualization_data_operation_insert_cn_11.jpg)

    11. 插入成功。

    ![](visualization_data_operation_insert_cn_12.jpg)

###复制(Copy)###

-   

    为了方便构建相似或重复的记录，提供 **复制** 功能来实现。

    1. 点击记录的 &lt;**复制**&gt; 按钮。

    ![](visualization_data_operation_copy_cn_1.jpg)

    2. 按需求修改记录，点击 &lt;**确定**&gt;。

    ![](visualization_data_operation_copy_cn_2.jpg)

    3. 复制成功。

    ![](visualization_data_operation_copy_cn_3.jpg)

###删除(Delete)###

-   

    1. 点击 &lt;**删除**&gt; 按钮。

    ![](visualization_data_operation_delete_cn_1.jpg)

    2. 根据实际需求设置 **删除条件**。

    3. 点击 &lt;**确定**&gt;。

    ![](visualization_data_operation_delete_cn_2.jpg)

    4. 删除成功。

    ![](visualization_data_operation_delete_cn_3.jpg)

###删除单条记录###

-   

    快速删除一条指定记录。

    1. 点击记录的 &lt;**删除**&gt; 按钮。

    ![](visualization_data_operation_delete_cn_4.jpg)

    2. 点击 **是的，删除**。

    ![](visualization_data_operation_delete_cn_5.jpg)

    3. 删除成功。

    ![](visualization_data_operation_delete_cn_6.jpg)

###更新(Update)###

-   

    1. 点击 &lt;**更新**&gt; 按钮。

    ![](visualization_data_operation_update_cn_1.jpg)

    2. 根据实际需求设置 **更新条件**、**更新操作**。

    ![](visualization_data_operation_update_cn_2.jpg)

    3. 点击 &lt;**确定**&gt;。

    ![](visualization_data_operation_update_cn_3.jpg)

    4. 更新成功。

    ![](visualization_data_operation_update_cn_4.jpg)

###更新单条记录###

-   

    快速修改一条指定记录。

    1. 点击记录的 &lt;**编辑**&gt; 按钮。

    ![](visualization_data_operation_update_cn_5.jpg)

    2. 根据实际需求修改记录。

    ![](visualization_data_operation_update_cn_6.jpg)

    3. 点击 &lt;**确定**&gt;。

    ![](visualization_data_operation_update_cn_7.jpg)

    4. 修改成功。

    ![](visualization_data_operation_update_cn_8.jpg)

###查询(Query)###

-   

    1. 点击 &lt;**查询**&gt; 按钮。

    ![](visualization_data_operation_query_cn_1.jpg)

    2. 根据实际需求设置 **查询条件**、**选择字段**、**排序字段**、**扫描方式**、**返回记录数**、**跳过记录数**。

    ![](visualization_data_operation_query_cn_2.jpg)

    3. 点击 &lt;**确定**&gt;。

    ![](visualization_data_operation_query_cn_3.jpg)

    4. 查询成功。

    ![](visualization_data_operation_query_cn_4.jpg)

    5. 如果需要构建复杂查询，可以切换**高级查询**。

    ![](visualization_data_operation_query_cn_5.jpg)

    ![](visualization_data_operation_query_cn_6.jpg)SequoiaDB 自带一个 JavaScript shell，可以从命令行与 SequoiaDB 实例交互。这个 shell 非常有用，通过它可以执行管理操作、检查运行实例，亦或做其他尝试。这个 shell 对于使用 SequoiaDB 来说是至关重要的工具。

##运行 SequoiaDB shell##

1.启动 shell：

<pre class="prettyprint lang-javascript">
$ su - sdbadmin
$ /opt/sequoiadb/bin/sdb
Welcome to SequoiaDB shell!
help() for help, Ctrl+c or quit to exit</pre>

2.创建一个新的 sdb 连接

<pre class="prettyprint lang-javascript">
> db = new Sdb("localhost",11810);</pre>

3.创建集合空间

<pre class="prettyprint lang-javascript">
> db.createCS("foo");</pre>

4.创建集合

<pre class="prettyprint lang-javascript">
> db.foo.createCL("bar");</pre>

5.写入记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({"name":"sequoiadb"});</pre>

6.查询结果

<pre class="prettyprint lang-javascript">
> db.foo.bar.find();
{
  "_id": {
    "$oid": "53a82aa2c4b970091e000000"
  },
  "name": "sequoiadb"
}
Return 1 row(s).</pre>
查询结果正确

shell 是一个功能完备的 JavaScript 解析器，可以运行任何 JavaScript 程序。如：

<pre class="prettyprint lang-javascript">
> y=200
200
> y/20
10</pre>

还可以充分利用 JavaScript 的标准库。

<pre class="prettyprint lang-javascript">
> new Date("2013/04/17");
Wed Apr 17 2013 00:00:00 GMT+0800 (CST)
> "hello,world".replace("world","SequoiaDB")
hello,SequoiaDB</pre>

也可以定义和调用 JavaScript 函数：

<pre class="prettyprint lang-javascript">
> function sdb(n){
> ... if(n<=1)return 1;
> ... else return n*sdb(n-1);
> ... }
> sdb(4);
24</pre>

**Note:** 

可以使用多行命令。shell 会检测输入的 JavaScript 语句是否完整，如没有写完还可以接着写下一行。

###SequoiaDB 客户端###

启动 shell 可以运行任意 JavaScript 程序，但是 shell 的真正威力在于它是一个独立的 SequoiaDB 客户端。在使用 SequoiaDB shell 之前，确保 SequoiaDB 服务已启动。

假设 SequoiaDB 服务端口地址是 localhost:11810，下面介绍使用 shell 连接数据库。

<pre class="prettyprint lang-javascript">
> db = new Sdb()           //连接到数据库
localhost:11810

> db.help()         //查看db对象的方法
...

> db.createCS("foo")   //创建集合空间
localhost:11810.foo

> db.foo.createCL("bar")   //创建集合
localhost:11810.foo.bar

> db.foo.bar.insert({a:1,b:2,c:3})   //插入记录
Takes 0.17162s.

> db.foo.bar.find()  // 查询记录
{
  "_id": {
    "$oid": "559e21b3d057b0f226000000"
  },
  "a": 1,
  "b": 2,
  "c": 3
}
Return 1 row(s).</pre>

##使用 SequoiaDB shell 的窍门##

SequoiaDB shell 本身内置了帮助文档，通过 help() 命令可以查看使用介绍。另外，参考手册 SequoiaDB JavaScript 方法一节中，有各方法的详细使用介绍。

-   Help

	查看使用介绍：

	<pre class="prettyprint lang-javascript">
	> help()
	var db = new Sdb()                     connect to database use default host 'localhost' and default port 11810
	var db = new Sdb('localhost',11810)    connect to database use specified host and port
	var db = new Sdb('ubuntu',11810,'','') connect to database with username and password
	help(&lt;method&gt;)                       help on specified method, e.g. help('createCS')
	db.help()                              help on db methods
	db.cs.help()                           help on collection space cs
	db.cs.cl                               access collection cl on collection space cs
	db.cs.cl.help()                        help on collection cl
	db.cs.cl.find()                        list all records
	db.cs.cl.find({a:1})                   list records where a=1
	db.cs.cl.find().help()                 help on find methods
	db.cs.cl.count().help()                help on count methods
	print(x), println(x)                   print out x
	traceFmt(&lt;type&gt;, &lt;in&gt;,&lt;out&gt;)     format trace input(in) to output(out) by type
	getErr(ret)                            print error description for return code
	clear                                  clear the terminal screen
	history -c                             clear the history
	quit                                   exit
	Takes 0.2993s.</pre>

	**Note:**

	SequoiaDB shell 主要包括 database(db)，collectionspace(cs)，collection(cl)，cursor(cur)，replicagroup(rg)，node(nd)，domain(dm) 这7大级别的操作。用户需要理解各级别之间的关系。各级别都有使用帮助命令如下所示：

-   Database Help

	database 级别主要包括用管理户组，集合空间，副本组，域，快照，存储过程，备份，事务，sql，及错误跟踪等操作。

	假设已经连接上数据库，并取得 database 的 javascript 对象 db。

	查看 database 所有方法：

	<pre class="prettyprint lang-javascript">
	> db.help()</pre>

	查看 database 具体方法：

	<pre class="prettyprint lang-javascript">
	> db.help("method")</pre>

-   CollectionSpace Help

	collection space 级别主要包括对集合管理的操作。

	假设存在名字为“foo”的集合空间。

	查看 collection space 所有方法：

	<pre class="prettyprint lang-javascript">
	> db.foo.help()</pre>

	查看 collection space 具体方法：

	<pre class="prettyprint lang-javascript">
	> db.foo.help("method")</pre>

-   Collection Help

	collection 级别主要包括CRUD，索引管理，数据切分，垂直分区表管理等操作。

	假设在集合空间“foo”中存在名字为“bar”的集合。

	查看 collection 所有方法：

	<pre class="prettyprint lang-javascript">
	> db.foo.bar.help()</pre>

	查看 collection 具体方法：

	<pre class="prettyprint lang-javascript">
	> db.foo.bar.help("method")</pre>

-   Cursor Help

	cursor 级别主要包括对返回记录（数据）的操作。

	在 shell 命令中，与 sequoiadb 引擎交互时，若有记录（数据）返回，都是以游标（cursor）的方式呈现。例如，当使用 db.foo.bar.find() 方法执行数据库查询操作，将返回一个游标对象，所有查询结果将放在这个游标中。通常的使用方法如下：

	<pre class="prettyprint lang-javascript">
	> db.foo.bar.find()</pre>

	或者

	<pre class="prettyprint lang-javascript">
	> var cur = db.foo.bar.find()</pre>

	前者直接将所有结果显示在屏幕上，后者将结果放到游标中。

	查看 cursor 所有方法：

	<pre class="prettyprint lang-javascript">
	> db.foo.bar.find().help()</pre>

	或者

	<pre class="prettyprint lang-javascript">
	> cur.help()</pre>

	查看 cursor 具体方法：

	<pre class="prettyprint lang-javascript">
	> db.foo.bar.find().help("method")</pre>

	或者

	<pre class="prettyprint lang-javascript">
	> cur.help("method")</pre>

	类似于 find() 返回游标的方法，还有 list，snapshot 等等。

-   Replica Group Help

	replica group 级别主要包括对数据节点的管理的操作。

	假设数据库中存在名字为“group1”的副本组，通过 var rg = db.getRG("group1") 获取一个关于副本组的 javascript 对象 rg。

	查看 replica group 所有方法：

	<pre class="prettyprint lang-javascript">
	> rg.help()</pre>

	查看 replica group 具体方法：

	<pre class="prettyprint lang-javascript">
	> rg.help("method")</pre>

-   Node Help

	node 级别主要包括对数据节点状态信息获取的操作。

	假设在副本组“group1”中创建一个数据节点，var rn = rg.createNode("ubuntu-dev1", 51000,"/opt/sequoiadb/database/data/51000")，获取一个关于数据节点的 javascript 对象 rn。

	查看 node 所有方法：

	<pre class="prettyprint lang-javascript">
	> rn.help()</pre>

	查看 node 具体方法：

	<pre class="prettyprint lang-javascript">
	> rn.help("method")</pre>

-   Domain Help

	domain 级别主要包括对域更改及获取域信息的操作。

	假设在数据库中创建一个名字为“domain1”的域，var dm = db.createDomain("domain1",["group1","group2"],{AutoSplit:true})，获取一个关于域的 javascript 对象 dm。

	查看 domain 所有方法：

	<pre class="prettyprint lang-javascript">
	> dm.help()</pre>

	查看 domain 具体方法：

	<pre class="prettyprint lang-javascript">
	> dm.help("method")</pre>

	**Note: **
	
	以 man page 方式显示帮助文档功能是随 SequoiaDB 1.8版本发布的，若使用1.8版本以下的 sdb shell 客户端，将不具备上述的 help("method") 功能。另外，应该确保 /opt/sequoiadb/doc/manual 目录下有相关方法的 troff 文件，否则，无法显示相应的 man page 介绍。
如果需要在数据库集群中新增一台主机（物理机或者虚拟机），用于部署编目节点或者数据节点，则必须需要按照如下步骤配置好主机系统：

1.安装好与其他主机相同的操作系统，并配置好IP地址；

2.按照[系统要求](SdbDoc_Cn/installation/system.html)一节配置好主机名/内核参数，并将其他的主机名和 IP 对应关系加入到 /etc/hosts 中；

3.修改每台集群主机上 /etc/hosts 文件，将新增的主机IP地址与主机对应关系加入到 /etc/hosts 文件中；

4.按照[系统要求](SdbDoc_Cn/installation/system.html)一节验证配置的正确性；

5.按照 [数据库安装](SdbDoc_Cn/installation/deployment/command_installation/installation.html)一节，安装 SequoiaDB 软件。安装时，注意配置管理服务端口与现有系统的端口保持一致。
##卸载旧版本##

-   以 root 身份登陆数据库服务器

-   执行如下命令停止 SequoiaDB 配置服务程序

	<pre class="prettyprint lang-javascript">
	$ service sdbcm stop</pre>

-   执行如下命令检测本机与 SequoiaDB 数据库相关的服务进程是否都已经停止

	<pre class="prettyprint lang-javascript">
	$ su - sdbadmin
	$ /opt/sequoiadb/bin/sdblist -t all</pre>
	
-   若仍存在 SequoiaDB 数据库进程，可使用以下命令停止这些进程

	<pre class="prettyprint lang-javascript">
	$ /opt/sequoiadb/bin/sdbstop -t all</pre>

-   若仍存在 SequoiaDB 数据库管理进程，可使用以下命令停止这些进程

	<pre class="prettyprint lang-javascript">
	$ /opt/sequoiadb/bin/sdbcmtop</pre>

-   执行如下命令备份 SequoiaDB 执行程序以及配置文件

	<pre class="prettyprint lang-javascript">
	$ scp /opt/sequoiadb/bin/sequoiadb /home/sequoiadb_bak
	$ scp -r /opt/sequoiadb/conf /home/sequoiadb_conf_bak</pre>

-   以 root 身份执行如下命令卸载 SequoiaDB 软件

	<pre class="prettyprint lang-javascript">
	$ /opt/sequoiadb/uninstall</pre>

分别在所有数据库服务器上执行完上述操作后，再执行以下操作：

##安装新版本##

-   具体安装步骤参见[SequoiaDB安装](SdbDoc_Cn/installation/deployment/command_installation/installation.html)章节

-   待新版本安装成功后，执行以下命令删除备份文件：

	<pre class="prettyprint lang-javascript">
	$ rm -rf /home/sequoiadb_bak
	$ rm -rf /home/sequoiadb_conf_bak</pre>
**Note: **

集群环境需要在每台数据执行如下操作：

-   以 root 身份登陆数据库服务器

-   执行如下命令卸载 SequoiaDB 软件

	<pre class="prettyprint lang-javascript">
	$ /opt/sequoiadb/uninstall</pre>

-   回退系统配置参数

    1.删除配置文件 /etc/security/limits.conf 中的如下配置参数：

    <pre class="prettyprint lang-diy">
	 •  &lt;#domain&gt;     &lt;type&gt;    &lt;item&gt;     &lt;value&gt;
	 •  *               soft        core         0
	 •  *               soft        data         unlimited
	 •  *               soft        fsize        unlimited
	 •  *               soft        rss          unlimited
	 •  *               soft        as           unlimited</pre>

    2.删除配置文件 /etc/sysctl.conf 中的如下配置参数：

	<pre class="prettyprint lang-javascript">
	vm.swappiness = 0
	vm.dirty_ratio = 100
	vm.dirty_background_ratio = 10
	vm.dirty_expire_centisecs = 50000
	vm.vfs_cache_pressure = 200
	vm.min_free_kbytes = &lt;物理内存大小的8%，单位KB&gt;</pre>
在 SequoiaDB 中，create 操作是向集合中添加新的文档记录。我们可以使用[insert()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/insert.html) 方法向 SequoiaDB 中的集合中添加记录。

  所有的插入操作在 SequoiaDB 中具有如下性质：

  -   如果插入的文档记录没有 \_id 字段，客户端将会为记录自动添加 \_id字段，并且填充一个唯一值。
  -   如果指定 \_id 字段，那个在集合中 \_id 的值必须唯一；否则出现操作异常。
  -   最大的 BSON 文档长度为16MB。
  -   文档结构的字段命名有如下限制：

字段名 \_id作为主键保存在集合中，它的值必须唯一且不可改变，它的值可以是除数组类型以外的其他任何类型。字段的命名不能是空串；不能以$开始；不能含有（.）。

**Note:** 本文档的所有例子都是使用 SequoiaDB 的 shell 接口。

##insert()##

insert() 是向SequoiaDB 集合中插入记录的主要方法，它有以下语法：

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.insert(&lt;doc|docs&gt;,[flag])</pre>

###插入第一个文档###

如果[集合空间](SdbDoc_Cn/data_model/collectionspace.html)和[集合](SdbDoc_Cn/data_model/collection.html)不存在，首先创建集合空间（如db.createCS("foo")：创建集合空间 foo）和集合（如db.foo.createCL("bar")：在集合空间下创建集合 bar），之后才能做插入操作。

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert(
  {
     _id:1,
     name:{fist:"Jhon",last:"Black"},
     phone:[1853742000,1802321000],
     remark:[
      {
        position:"manager",
        year:2000
      },
      {
        position:"CEO",
        year:2012
      }
    ]
  }
)</pre>

可以使用 find() 方法确认是否插入成功。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find()</pre>

此操作返回结果如下：

<pre class="prettyprint lang-diy">
{
  _id:1,
  name:{fist:"Jhon",last:"Black"},
  phone:[1853742000,1802321000],
  remark:[
    {
      position:"manager",
      year:2000
    },
    {
      position:"CEO",
      year:2012
     }
  ]
}</pre>

###不指定 \_id 字段###

如果新的文档记录不包含 \_id字段，[insert()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/insert.html)方法向文档添加 \_id 字段并生成一个唯一的 $oid 值

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({name:"Tom",age:20})</pre>

此操作是向集合 bar 中插入一条新的记录，记录 name 字段的值为“Tom”，age字段的值为20，_id 字段被唯一创建：

<pre class="prettyprint lang-diy">
{ "_id": { "$oid": "515152ba49af395200000000" }, "name": "Tom", "age": 20 }</pre>

###插入多条记录###

如果向 insert 方法中传一个数组类型的文档，insert()方法将会在集合中执行批量插入。

下面的操作是向集合 bar 中插入两条记录。此操作也说明了 SequoiaDB的动态模式的特点。尽管 _id:20 的记录含有字段名 phone 而在另一条记录中不存在，SequoiaDB 不要求其他记录必须含有此字段。

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert([{name:"Mike",age:15},{_id:20,name:"John",age:25,phone:123}])</pre>
##find()##

我们使用 [find()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/find.html) 方法读取 SequoiaDB 中的记录。find方法是从集合中选择记录的主要方法，它返回一个包含很多记录的游标。它的语法结构如下：

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.find([cond],[sel])</pre>

在 SQL 中对应的操作：find() 的方法与 SELECT 语句相似：

-  [cond] 参数对应 WHERE 语句

-  [sel] 参数对应从结果集中选择的字段列表

现集合中有如下一条记录：

<pre class="prettyprint lang-diy">
{
  "_id":1,
  "name":
    {
      "first" : "Tom",
      "second":"David"
    },
  "age":23
  "birth":"1990-04-01",
  "phone":
    [
      10086,
      10010,
      10000
    ],
  "family":
    [
      {
        "Dad":"Kobe",
        "phone":139123456
      },
      {
        "Mom":"Julie",
        "phone":189123456
      }
    ]
 }</pre>

##返回集合所有记录##

如果没有 cond 参数，方法 db.collectionspace.colletion.find() 选择集合中所有的记录，如下返回集合空间 foo 中集合 bar 的所有记录：

<pre class="prettyprint lang-javascript">
> db.foo.bar.find()</pre>

##返回匹配条件的记录##

-   Equality 匹配

    下面的操作返回集合 bar 中 age 等于23的记录

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find({age:23})</pre>

-   使用匹配符

    下面操作返回集合 bar 中 age 字段值大于20的记录

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find({age:{$gt:20}})</pre>

-   嵌套数组匹配

    1.数组元素查询，下面的操作操作返回一个游标，指向集合 bar 中所有数组类型字段 phone 含有元素10086的记录：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find({"phone":10086})</pre>

    2.数组元素为 BSON 对象的查询，下面的操作返回一个游标指向集合 bar 中 family 字段包含的子元素 Dad 字段值为“Kobe”，phone字段值为139123456的记录：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find(
    {
      "family":{
        $elemMatch: {
          "Dad":"Kobe",
          "phone":139123456
        }
      }
    })</pre>

-   嵌套 BSON 对象匹配查询

    下面的操作返回一个游标指向集合 bar 中嵌套 BSON 对象的 name 字段匹配{"first":"Tom"}的记录

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find(
    {
      "name":{
        "$elemMatch":{ "first":"Tom" }
      }
    }
    )</pre>

    上面还可以写成：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find(
    {
      "name.first":"Tom"
    }
    )</pre>

##指定返回记录字段##

如果指定 find 方法的 sel 参数，那么只返回指定的 sel 参数内的字段名。下面的操作返回记录的 name 字段：

<pre class="prettyprint lang-javascript">
> db.foo.bar.find(null,{name:""})</pre>

**Note:**

如果记录中不存在指定的字段名（如：people），SequoiaDB 默认也返回。如：

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{name:"",people:""})</pre>

返回：
<pre class="prettyprint lang-diy">
{
  "name":
  {
    "fist":"Tom",
    "second":"David"
  },
  "people":""
}</pre>

##更多信息##

执行 db.foo.bar.find().help() , 会看到 find() 的更多使用方法

-   cursor.sort(&lt;sort&gt;)

    sort()方法用来按指定的字段排序，语法格式为：sort({"字段名1"：1|-1,"字段名2"：1|-1,...})，1为升序，-1为降序。如果 find 方法的 sel 参数不设定内容，sort() 方法按指定 sort 参数设定的字段排序，如果 sel 参数设定了返回的字段名，那么 sort() 方法只能对 sel 参数中选定的字段进行排序。如：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find().sort({age:1})</pre>

    对返回的记录按 age 字段值的升序排序

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find(null,{name:""}).sort({age:1})</pre>

    此操作实际上对返回的记录达不到排序的效果。

-   cursor.hint(&lt;hint&gt;)

    添加索引加快查找速度，假设存在名为“testIndex”的索引：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find().hint({"":"testIndex"})</pre>

-   cursor.limit(&lt;num&gt;)

    在结果集中限制返回的记录条数：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find().limit(3)</pre>

    返回结果集里面的的前三条记录

-   cursor.skip(&lt;num&gt;)

    skip() 方法控制结果集的开始点，即跳过前面的 num 条记录，从num+1条记录开始返回：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find().skip(5)</pre>

    从查询的结果集的第6条记录开始返回

-   使用游标控制 find() 返回的记录

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.find().current()   //返回当前游标指向的记录
    > db.foo.bar.find().next()      //返回当前游标指向的下一条记录
    > db.foo.bar.find().close()     //关闭当前游标，当前游标不再可用
    > db.foo.bar.find().count()     //返回当前游标的记录总数
    > db.foo.bar.find().size()      //返回当前游标到最终游标的距离
    > db.foo.bar.find().toArray()   //以数组形式返回结果集</pre>
更新操作即修改集合中已存在的记录。SequoiaDB中使用 [update()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/update.html)方法做更新操作。

**Note:**
本文档的所有例子都是使用 SequoiaDB 的 shell 接口。

##update()##

update() 方法是修改集合中记录的主要方法，它的语法结构为：

<pre class="prettyprint lang-javascript">
db.collectionspace.collection.update(&lt;rule&gt;,[cond],[hint])</pre>

在 SQL中 对应的操作：update() 的方法与 update...set 语句相似：

-  &lt;rule&gt;参数对应 set 语句

-  [cond] 参数对应 where 语句

-  [hint] 参数是对应索引表里的名称

##使用 update 操作修改记录##

如果 update() 方法只有 rule 参数的表达式（例如使用 $set 更新表达式），那么 update 方法会修改集合记录中所有指定的字段；更新嵌套对象 SequoiaDB 使用点（.）操作符。

-   更新记录字段

    使用 $set 更新记录字段的值。下面的操作修改集合 bar 中符合条件 \_id 字段值等于1的记录，使用 $set 修改 name 字段的嵌套元素 first字段的值，将它的值修改为“Mike”：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.update({$set:{"name.first":"Mike"}},{_id:1})</pre>

    **Note: **

    如果 rule 参数包含的字段名没有在当前的记录中，update()方法会添加 rule 参数包含的字段到记录中。

-   删除记录字段

    使用 $unset 删除记录的字段名。下面的操作是删除集合 bar 中所有含有 age 字段的记录，如果记录中没有 age 字段，跳过。

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.update({$unset:{age:""}})</pre>

-   数组元素更新

    如果需要更新数组中的元素，SequoiaDB使用点操作符（.），数组下标从0开始。下面的操作是修改数组字段 arr 的第二个元素的值，将它的值添加为5：

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.update({$inc:{"arr.1":5}})</pre>

-   hint 参数

    下面操作会通过索引遍历对所有记录的 name 字段内容修改为Tom，“textIndex”为索引名称。

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.update({$set:{name:"Tom"}},null,{"":"textIndex"})</pre>
  删除操作即移除集合中的记录。SequoiaDB中使用[remove()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/remove.html)方法做删除操作。

  **Note:** 本文档的所有例子都是使用 SequoiaDB 的 shell 接口。

##remove()##

remove() 方法是删除集合中记录主要方法，它的语法结构为：

<pre class="prettyprint lang-javascript">
db.collectionspace.collection.remove([cond],[hint])</pre>

在 SQL 中对应的操作：remove() 的方法与 DELETE 语句相似：

-  [cond] 参数对应 where 语句

-  [hint] 参数是对应索引表里的名称

##删除集合记录##

-   删除集合中的所有记录

    以下操作会删除集合 bar 中所有的记录。

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.remove()</pre>

-   删除集合中匹配条件的记录

    以下操作会删除集合 bar 中所有匹配 name 字段值为“Tom”的记录。

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.remove({name:"Tom"})</pre>

-   hint 参数

    以下操作会通过索引遍历快速删除匹配条件的记录，“textIndex”为索引名称。

    <pre class="prettyprint lang-javascript">
    > db.foo.bar.remove({name:"Tom"},{"":"testIndex"})</pre>
##概念##

在 SequoiaDB 数据库中，索引是一种特殊的数据对象。索引本身不做为保存用户数据的容器，而是作为一种特殊的元数据，提高数据访问的效率。

每一个索引必须建立在一个集合之中，一个集合最多可以拥有64个索引。

索引可以被认为是将数据按照某一个或多个给定的字段进行排序，从而在其中快速搜索到用户指定查询条件的方式。

在SequoiaDB 中，索引使用 B 树或LSM树（社区版）结构。

一个典型的索引结构如图1所示：

![图1](index_concept.jpg)

*图中左边的方形为数据，右边的三角形为索引。索引按照从小到大的树形结构排列，每条索引记录分别指向一条记录文档。*

通过进行树形遍历，对于查找某个特定数值的操作，可以使用树遍历在索引中快速定位其所需的数据。

一个典型的btree索引结构如图2所示：

![图2](index_btree.jpg)

*图中方形为btree的结点。索引纪录从小到大排列在结点中，每个结点的纪录值范围由其父结点决定。*

一个典型的LSM树索引结构如图3所示：

![图3](index_lsm.jpg)

*图中三角形为索引。C0树置于内存，往后的LSM树置于硬盘。索引纪录会先进入C0树，通过Merge过程，纪录被逐渐合并到靠后的LSM树中。*

基于LSM树索引的结构,其适用于插入频率远大于查询频率的应用场景。

SequoiaDB 可以对任意数据类型进行索引，每一个索引包含几个属性：

  属性       描述
  ---------- --------------------------------------------------------------------------------------------------------------------------------------------------
  name       索引名，同一个集合中的索引名必须唯一。
  key        索引键，为一个 JSON 结构，包含一个或多个指定索引字段与方向的字段。其中方向为1代表从小到大排序，-1则为从大到小排序。
  unique     索引是否唯一，可选参数，默认 false。设置为 true 时代表该索引为唯一索引。在唯一索引所指定的索引键字段上，集合中不可存在一条以上的记录完全重复。
  enforced   索引是否强制唯一，可选参数，在 unique 为 true 时生效，默认 false。设置为 true 时代表该索引在 unique 为 true 的前提下，不可存在一个以上全空的索引键。

在 SequoiaDB 中，所有集合均包含一个名为“$id”的强制唯一索引。该索引包含一个“\_id”字段的索引键。

所有的分区集合在创建时均会自动生成一个额外的“$shard”索引，索引键为用户指定的分区键字段。

**Note:**

在分区集合中，所有的唯一索引必须包含集合分区键中所指定的全部字段。

分区集合中，“$id”索引仅保证单节点内记录的唯一性。如果用户希望指定全局唯一的字段，需要额外创建唯一索引，且该索引必须包含集合分区键中所指定的全部字段。

当使用LSM树索引，遇到磁盘满的错误时，需清除出足够空间后重启数据库，数据库方可恢复正常使用。

##格式##

索引的定义格式必须包含 name 与 key 两个字段。其中 name 的值必须为字符串，key 则为一个 JSON 对象。

<pre class="prettyprint lang-diy">
{ "name" : "&lt;索引名&gt;", "key" : "{ "&lt;索引字段1&gt;" : &lt;1|-1&gt;, [ "&lt;索引字段2&gt;" : &lt;1|-1&gt; ...] }, [ "unique" : &lt;true|false&gt; ], [ "enforced" : &lt;true|false&gt; ]}</pre>

key 的对象必须包含至少一个字段，其中字段名为用户需要索引的字段名，数值为1或者-1。其中1代表数据在索引中的排列顺序由小至大，-1则代表由大至小。

##示例##

-   非唯一索引，索引名“employee_id_key”，索引字段为正向“employee_id”

	<pre class="prettyprint lang-diy">
	{ "name" : "employee_id_key", "key" : {"employee_id" : 1 } }</pre>

-   唯一索引，索引名为“record_id_index”，索引字段为正向“product_key”与逆向“record_key”

	<pre class="prettyprint lang-diy">
	{ "name" : "record_id_index", "key" : { "product_key" : 1, "record_key" : -1 }, "unique" : true }</pre>

	在该索引中，不可存在两条记录拥有同样的 product_key 与 record_key（如果仅 product_key 相同，或者仅 record_key 相同则可以通过唯一判定）

-   强制唯一索引，索引名为“测试索引”，索引字段为正向“测试用例名称”

	<pre class="prettyprint lang-diy">
	{ "name" : "测试索引", "key" : { "测试用例名称" : 1 }, "unique" : true, "enforced" : true }</pre>

	在强制唯一索引中，所有记录必须遵循唯一索引规则，且不可存在一条以上的数据在“测试用例名称”字段为空。
聚集框架提供了对集合中的原始数据记录进行统计计算的能力。通过使用聚集框架，用户能够直接从集合中提取数据记录并获取所需的统计结果。聚集框架提供的操作接口类似于集合中的查询操作，不同的是聚集框架还提供了一系列函数及操作对查询结果进行处理。

##aggregate()##

以下是聚集操作举例：

![](aggregate_1.jpg)

上例聚集操作包含了两个子操作：

  -   “$match”子操作将集合中年龄大于30的数据记录筛选出来；
  -   “$group”操作从筛选出的数据记录按照城市进行分组，计算出每个城市的人均收入。
  
通过上例聚集操作将得到各城市30岁以上的人均收入。

![](aggregate.jpg)
事务是由一系列操作组成的逻辑工作单元。在同一个会话（或连接）中，同一时刻只允许存在一个事务，也就是说当用户在一次会话中创建了一个事务，在这个事务结束前用户不能再创建新的事务。

事务作为一个完整的工作单元执行，事务中的操作要么全部执行成功要么全部执行失败。SequoiaDB事务中的操作只能是插入数据、修改数据以及删除数据，在事务过程中执行的其它操作不会纳入事务范畴，也就是说事务回滚时非事务操作不会被执行回滚。如果一个表或表空间中有数据涉及事务操作，则该表或表空间不允许被删除。

默认情况下，事务功能是关闭的。

如要打开事务功能需要在节点的配置文件中配置参数：transactionon =TRUE；在创建数据节点时，增加 JSON 类型的参数：{ "transactionon" : "YES" } 或 { "transctionon" : true }。

注意：要打开事务功能，必须将[logfilenum](SdbDoc_Cn/database_management/runtime_configuration.html)设置为大于等于5的值（如果未单独配置，其默认为20，则不需要修改）

## 示例##

* 开启事务：

<pre class="prettyprint lang-javascript">
> db.transBegin()
Takes 0.4125s. </pre>

* 插入记录:

<pre class="prettyprint lang-javascript">
> cl.insert({date:99,id:8,a:0})
Takes 0.3434s. </pre>

* 回滚事务，插入的记录将被回滚，集合中无记录：

<pre class="prettyprint lang-javascript">
> db.transRollback()
Takes 0.6474s.
> cl.count()
Return 0 row(s). </pre>

* 开启事务：

<pre class="prettyprint lang-javascript">
> db.transBegin()
Takes 0.4084s. </pre>

* 插入记录:

<pre class="prettyprint lang-javascript">
> cl.insert({date:99,id:8,a:0})
Takes 0.2644s. </pre>

* 提交事务，插入的记录将被写入数据库：

<pre class="prettyprint lang-javascript">
> db.transCommit()
Takes 0.780s.
> cl.count()
Return 1 row(s). </pre>**水平分区** [水平分区](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/split.html)又称为数据库分区或横向分区。

|	在 SequoiaDB 集群环境中，用户可以通过将一个集合中的数据切分到多个复制组中，以达到并行计算的目的，此数据切分称为水平分区。水平分区是按一定的条件把全局关系的所有元组划分成若干不相交的子集，每个子集为关系的一个片段，称为分区；一个分区只能存在于一个复制组中，但一个复制组可以承载多个分区；分区在复制组之间可以通过水平切分操作进行移动。

![](sh3.jpg)

**垂直分区** 垂直分区又称为集合分区或纵向分区。

|	在 SequoiaDB集群环境中，用户也可以将一个集合全局关系的属性分成若干子集，并在这些子集上作投影运算，将这些子集映射到另外的集合上，从而实现集合关系的垂直切分；该集合称之为主集合，每个切分的子集称为分区，分区映射的集合称为子集合；一个分区只能映射到一个子集合中，但一个子集合可以承载多个分区；分区在子集合之间可以通过垂直切分操作进行重映射。

![](sh4.jpg)

**混合分区** 在 SequoiaDB 集群环境中，可以将集合先通过垂直分区映射到多个子集合中，再通过水平分区将子集合切分到多个复制组中，从而实现混合分区。

**分区方式**

数据分区有两种方式：范围分区（Range）和散列分区（Hash）。水平分区即可使用Hash 方式也可使用Range 方式进行数据分区；垂直分区只能使用Range 方式进行数据分区。Hash 及 Range这两种分区方式判定分区划分所依据的字段称为“分区键”。分区键基于集合定义，每个分区键可以包含一个或多个字段。

Range 方式下依据记录中分区键的范围选择所要插入的分区。Hash方式下根据记录中分区键生成的 hash 值选择所要插入的分区。

![](sharding_concept.jpg)

在所示图中，为一个 Range 方式分区，方形区域为三个分别位于不同数据组的数据节点，椭圆形为协调节点。每个数据节点各自定义了所包含数据的范围。例如对于节点1包含了大于等于0切小于10的数据。

当用户插入一条数据时，协调节点首先判定该数据的分区键应当坐落于哪个分区。如果分区键不存在则定义为 Undefined 类型（Undefined类型也可以与普通数据类型进行对比）。

当查询到该数据所在的分区后，协调节点会将请求直接下发给指定的分区。

##垂直分区示例##

1.创建主表（主表必须用range切分）
<pre class="prettyprint lang-javascript">
&gt; db.createCS("maincs").createCL("maincl",{IsMainCL:true,ShardingKey:{a:1},ShardingType:"range"})
localhost:50000.maincs.maincl
Takes 0.8144s.
</pre>

2.创建子表1（子表既可用range，也可用hash，ShardingKey也不必一定要和主表的一致）
<pre class="prettyprint lang-javascript">
&gt; db.createCS("year2015").createCL("month01",{ShardingKey:{a:1},ShardingType:"hash",Partition:1024})
localhost:50000.year2015.month01
Takes 0.899728s.
</pre>

3.创建子表2
<pre class="prettyprint lang-javascript">
&gt; db.year2015.createCL("month02",{ShardingKey:{a:1},ShardingType:"hash",Partition:1024})
localhost:50000.year2015.month02
Takes 0.9760s.
</pre>

4.将子表1、子表2关联到主表中（将子表附到主表中去，每个子表都有一个范围）
<pre class="prettyprint lang-javascript">
&gt; db.maincs.maincl.attachCL("year2015.month01",{LowBound:{a:0},UpBound:{a:100}})
Takes 0.8920s.
&gt; db.maincs.maincl.attachCL("year2015.month02",{LowBound:{a:100},UpBound:{a:200}})
Takes 0.9837s.
</pre>

5.查看主子表情况
<pre class="prettyprint lang-javascript">
&gt; db.snapshot(8)
{
  "CataInfo": [
    {
      "ID": 1,
      "SubCLName": "year2015.month01",
      "LowBound": {
        "a": 0
      },
      "UpBound": {
        "a": 100
      }
    },
    {
      "ID": 2,
      "SubCLName": "year2015.month02",
      "LowBound": {
        "a": 100
      },
      "UpBound": {
        "a": 200
      }
    }
  ],
  "EnsureShardingIndex": true,
  "IsMainCL": true,
  "Name": "maincs.maincl",
  "ShardingKey": {
    "a": 1
  },
  "ShardingType": "range",
  "Version": 3,
  "_id": {
    "$oid": "56c180b419ca59d5c29afb20"
  }
}
{
  "CataInfo": [
    {
      "GroupID": 1003,
      "GroupName": "datagroup",
      "LowBound": {
        "": 0
      },
      "UpBound": {
        "": 1024
      }
    }
  ],
  "EnsureShardingIndex": true,
  "InternalV": 3,
  "MainCLName": "maincs.maincl",
  "Name": "year2015.month01",
  "Partition": 1024,
  "ShardingKey": {
    "a": 1
  },
  "ShardingType": "hash",
  "Version": 2,
  "_id": {
    "$oid": "56c180c219ca59d5c29afb25"
  }
}
{
  "CataInfo": [
    {
      "GroupID": 1003,
      "GroupName": "datagroup",
      "LowBound": {
        "": 0
      },
      "UpBound": {
        "": 1024
      }
    }
  ],
  "EnsureShardingIndex": true,
  "InternalV": 3,
  "MainCLName": "maincs.maincl",
  "Name": "year2015.month02",
  "Partition": 1024,
  "ShardingKey": {
    "a": 1
  },
  "ShardingType": "hash",
  "Version": 2,
  "_id": {
    "$oid": "56c180ce19ca59d5c29afb28"
  }
}
Return 3 row(s).
Takes 0.10594s.
</pre>

6.插数据
<pre class="prettyprint lang-javascript">
&gt; db.maincs.maincl.insert({a:1})  // 数据落到子表1中
Takes 0.2195s.
&gt; db.maincs.maincl.insert({a:101}) // 数据落到子表2中
Takes 0.1872s.
</pre>

7.查看数据落表的情况
<pre class="prettyprint lang-javascript">
> db.year2015.month01.find()
{
  "_id": {
    "$oid": "564968bb5fc84bb828000000"
  },
  "a": 1
}
Return 1 row(s).
Takes 0.3209s.
&gt; db.year2015.month02.find()
{
  "_id": {
    "$oid": "564968be5fc84bb828000001"
  },
  "a": 101
}
Return 1 row(s).
Takes 0.3407s.
</pre>


8.解除主子表之间的关联
<pre class="prettyprint lang-javascript">
&gt; db.maincs.maincl.detachCL("year2015.month01")
Takes 0.10324s.
&gt; db.maincs.maincl.detachCL("year2015.month02")
Takes 0.7857s.
</pre>##概念##

分区键定义了每个集合中所包含数据的分区规则。每一个集合对应一个分区键，分区键中可以包含一个或多个字段。

在编目节点中，每个集合都拥有自己的分区范围，分区范围中每个范围段对应一个分区组，标示该集合的某一数据段坐落于该分区组。

**Note:**

集合的索引键在创建集合时指定，集合创建成功后索引键无法修改。

在[分区集合](SdbDoc_Cn/basic_operation/sharding/sharding_collection.html)中，记录插入数据库后无法对分区键值进行更新。

##格式##

-   Range 分区键

    Range 分区键的格式类似于索引键，为一个 JSON 对象。JSON 对象中每一个字段对应分区键的字段，数值为1或者-1，代表正向或逆向排序。

	<pre class="prettyprint lang-diy">
	{ <字段1> : <1|-1>, [ <字段2> : <1|-1> ...] }</pre>
	
-   Hash 分区键

    Hash 分区的 ShardingKey 组成方式与 Range 分区方式相同（但字段的正向/逆向不起作用）。Partition 代表了 hash 分区的个数。其值必须是2的幂。范围在[2^3 , 2^20]。此字段为可选字段。默认为2^12，代表我们将整个范围平均划分为4096个分区。设计hash分区的目的是让数据分布更灵活，可以根据需要自由设置每个数据分区承担 hash 分区的范围。ShardingType 如果不填则默认为 Range 分区。

	<pre class="prettyprint lang-diy">
	{ ShardingKey : { <字段1> : <1|-1>, [<字段2> : <1|-1>, ...] }, 
	{ ShardingType : "hash" }, 
	[ { Partition : <分区数> } ] }</pre>

##示例##

-   一个包含两个字段，分别为正向和逆向排序的 Range 分区键如下：

	<pre class="prettyprint lang-diy">
	{ Field1 : 1, Field2 : -1 }</pre>

-   Hash 分区键

	<pre class="prettyprint lang-diy">
	{ { Field1 : 1, Field2 : -1 }, { ShardingType : "hash" }, { Partition : 2^12 } }</pre>
##概念##

一个定义了分区键的集合为分区集合。分区集合可以按照分区键所指定的字段，将集合中的数据切分到超过一个数据分区组中。

当集合创建时，用户可以指定分区键。分区集合会在一个随机的数据分区组中创建。用户可以使用手工切分的方式对集合按照某一规则切分至多个数据分区组中。

##分区区间##

分区集合中每一个区间叫做一个分区区间。

分区集合创建时，其所在的分区组包含全部区间，为所有字段的 MinKey 至 MaxKey。

每一个分区区间为左闭右开规则，也就是包含大于等于低边界，且小于高边界的区域。例如：

<pre class="prettyprint lang-diy">
{ LowBound: { "" : 10 }, UpBound: { "" : 20 } }</pre>

在该例中，低边界为10，高边界为20，因此本区间包含所有分区字段大于等于10，且小于20的数据。

**Note:**

一个集合中所有边界的定义不包含字段名，其字段应当与分区键所定义的字段，与字段数量保持一致。

当分区键包含多个字段时，其匹配规则为第一字段首先匹配，如果坐落于边界值则匹配下一字段。例如：

<pre class="prettyprint lang-diy">
{ LowBound: { "" : 10, "" : 5 }, UpBound: { "" : 20, "" : 1 } }</pre>

在该分区区间中，如果用户输入的分区键的第一个字段坐落于10与20之间，则立刻判定为该区间内；如果存在于小于10或大于20，则不在该区间内；而如果为10或者20，则需要进行第二个字段的匹配，匹配规则仍为左闭右开。

##规则##

分区集合定义的规则参见 [SYSCOLLECTIONS](SdbDoc_Cn/infrastructure/catalog_node.html) 集合定义。

##示例##

一个存在于两个分区组的典型分区区间如下：

<pre class="prettyprint lang-diy">
[
      { "GroupID" : 1000,
        "LowBound" : { "" : MinKey, "" : MaxKey },
        "UpBound" : { "" : 10, "" : 5 }
      },
      { "GroupID" : 1001,
        "LowBound" : { "" : 10, "" : 5 },
        "UpBound" : { "" : MaxKey, "" : MinKey }
      }
]</pre>

其中第一个区间所在分区组 ID 为1000，包含的分区键存在两个字段，分别为：

-   低边界：{ "" : MinKey, "" : MaxKey }
-   高边界：{ "" : 10, "" : 5 }

而第二个区间所在分区组为1001，包含分区区间为：

-   低边界：{ "" : 10, "" : 5 }
-   高边界：{ "" : MaxKey, "" : MinKey }
##概念##

每一个分区集合都会默认创建一个名叫“$shard”的索引，该索引叫做分区索引。

非分区集合不存在分区索引。

分区索引存在于分区集合所在的每一个分区组中，其字段定义顺序和排列与分区键相同。

**Note:**

任何用户定义的唯一索引必须包含分区索引中所有的字段，其字段顺序无关。

在分区集合中，_id 字段仅保证分区内该字段唯一，无法保证全局唯一。

##示例##

一个典型的分区索引如下：

<pre class="prettyprint lang-diy">
{
  "IndexDef" : 
  {
    "name" : "$shard",
    "_id" : { "$oid" : "515954bfa88873112fa6bd3a" },
    "key" : { "Field1" : 1, "Field2" : -1 },
    "v" : 0,
    "unique" : false,
    "dropDups" : false,
    "enforced" : false
  },
  "IndexFlag" : "Normal"
}</pre>
##概念##

后台任务是一种不阻塞前端会话的任务，并不会随着前端会话的中断而停止。

所有的后台任务都会在编目节点的SYSCAT.SYSTASKS集合中跟踪，不同类型的后台任务可能包含不同的字段。

以下字段存在于所有的后台任务中：

+-----------------+--------+---------------------+
| 字段名          | 类型   | 描述                |
+=================+==============================+  
| JobType         | 整数   |任务类型，分别代表： |
|                 |        |                     |
|                 |        |  -   0:数据切分     |
+-----------------+--------+---------------------+
| Status          | 整数   | 任务状态，分别代表：|
|                 |        |                     |
|                 |        |  -   0：准备        |
|                 |        |  -   1：运行        |
|                 |        |  -   2：暂停        |
|                 |        |  -   3：取消        |
|                 |        |  -   4：变更元数据  |
|                 |        |  -   9：完成        |
+-----------------+--------+---------------------+
| CollectionSpace | 字符串 | 集合空间名          |
+-----------------+--------+---------------------+
| Collection      | 字符串 | 集合名              |
+-----------------+--------+---------------------+








	
	
	
	
	
	
	
	

##概念##

一个分区集合首先会被创建在一个随机的分区组中。如果用户希望对该集合水平切分，将其划分到超过一个分区组中，就需要数据切分功能。

数据切分是一种将数据在线从一个分区组转移到另一个分区组的方式。在数据转移的过程中，查询所得的结果集数据会存在暂时的不一致，但是 SequoiaDB 可以保证磁盘中数据的最终一致性。

Range 分区和 Hash 分区都包含两种切分方式：范围切分和百分比切分。在范围切分时，Range 分区使用精确条件，而 Hash 分区使用Partition（分区数）条件。切分时起始条件为必填字段，而结束条件为选填条件，结束条件默认为切分源当前包含的最大数据范围。


**Hash:**

<pre class="prettyprint lang-javascript">
> db.foo.bar.split('src', 'dst', {Partition:10}, {Partition:20})</pre>

**Range:**

<pre class="prettyprint lang-javascript">
> db.foo.bar.split('src', 'dst', {a:10}, {a:20})</pre>

数据切分及分区上的数据范围皆遵循左闭右开原则。即：{Partition:10},{Partition:20} 代表迁移数据范围为[10, 20)。

**百分比切分：**

<pre class="prettyprint lang-javascript">db.foo.bar.split('src', 'dst', 50)</pre>

**Note:**

-   当切分范围不冲突时，可以做并发切分
-   “src”、“dst”分别表示“源分区所在复制组”、“目标分区所在复制组”


![](split.jpg)

*图中左上角为系统的起始状态，4条记录均存放在左侧的节点中。切分时定义由3起始，因此数据3与4会被切分至右侧节点。（左下图）*

*右上图为第三状态，数据在两个分区组中同时存在。此刻数据会有暂时的不一致。最终状态切换到右下图，已经迁移成功的数据从原始节点删除，数据最终恢复一致。*

在数据切分过程在两个数据分区组之间进行交互：

-   源分区所在复制组：代表数据原本所存在的分区
-   目标分区所在复制组：代表切分后，所有需要迁移的数据的目标组

##后台任务##

数据切分属于一个[后台任务](SdbDoc_Cn/basic_operation/sharding/background_task.html)。

对于数据切分的后台任务拥有几个特有的字段：

  字段名       类型     描述
  ------------ -------- -----------------------
  SourceName   字符串   源分区所在复制组名
  TargetName   字符串   目标分区所在复制组名
  SourceID     整数     源分区所在复制组 ID
  TargetID     整数     目标分区所在复制组 ID
  SplitValue   对象     数据切分键

数据切分的后台操作分为几个阶段：

**准备阶段:**
  ~ 在准备阶段中，并不会向编目节点的 SYSCAT.SYSTASKS
    插入任务记录。该阶段首先向编目节点查询，确保该请求合法，并且向源数据节点组请求得到一条包含分区条件的记录或根据规则生成一条包含分区条件的记录。

**预备阶段:**
  ~ 在预备阶段中，协调节点将分区条件发送至编目节点。编目节点在
    SYSCAT.SYSTASKS 集合中插入后台操作记录。

**运行阶段:**
  ~ 在运行阶段中，协调节点向目标节点发送切分请求，目标节点创建后台任务，从源节点请求数据，并向编目节点上报自身状态。目标节点会在后台任务创建后直接返回给协调节点，并不会长时间阻塞用户会话。

**清除阶段:**
  ~ 在清除阶段中，目标节点已经从源节点得到所有的数据，因此向编目节点发送清除请求，并在源数据节点进行数据清除操作。

**完成阶段:**
  ~ 在源节点清除了所有已经迁移的数据后，会向编目节点发送完成消息。编目节点从
    SYSCAT.SYSTASKS 集合中删除该任务。
SequoiaDB 数据库使用 JSON 数据模型，而非传统的关系型数据模型。

JSON 数据结构的全称为 JavaScript Object Notation，是一种轻量级的数据交换格式，非常易于人阅读和编写，同时也易于机器生成和解析。

它基于 JavaScript Programming Language, Standard ECMA-262 3rd Edition – December 1999 的一个子集，为纯文本格式，支持嵌套结构与数组。

**JSON 建构基于两种结构：**

-   键值对集合 —— 在键值对集合结构中，每一个数据元素拥有一个名称与一个数值。数值可以包含数字，字符串等常用结构，或嵌套 JSON 对象和数组。
-   数组 —— 在数组中的每一个元素不包含元素名，其值可以为数字，字符串等常用结构，或者嵌套 JSON 对象和数组。

**JSON 具有如下形式：**

-   对象是一个无序的“键值对”集合，以“{”（左大括号）开始，“}”（右大括号）结束。每一个元素名后跟一个“:”（冒号）；而元素之间使用“,”（逗号）分隔；

    ![](sequoiadb_datamodel_jsonstruct_img1.jpg)

-   数组是值的有序集合，以“[”（左中括号）开始，“]”（右中括号）结束。值之间使用“,”（逗号）分隔；

    ![](sequoiadb_datamodel_jsonstruct_img2.jpg)

-   值可以为由双引号包裹的字符串，数值，对象，数组，true，false，null，以及
    SequoiaDB 数据库特有的数据结构（例如日期，时间等）组成。

    ![](sequoiadb_datamodel_jsonstruct_img3.jpg)

一个典型的嵌套式数据结构如下：

![](sequoiadb_datamodel_jsonstruct_img4.jpg)
##概念##

SequoiaDB 中的文档为 JSON 格式，一般又被称为记录。在数据库内部使用BSON，即二进制的方式存放 JSON 数据。

一般来说，一条文档由一个或多个字段构成，每个字段分为键值与数值两个部分，如下为包含两个字段的文档：
<pre class="prettyprint lang-diy">
{ "姓名" : "张三", "性别" : "男" }</pre>

**Note:**

BSON 文档可能有多个同名的字段，但是，大多数 SequoiaDB接口不支持重复的字段名，如果需要操作的文档有多个同名的字段，请参阅驱动程序了解更多信息。

SequoiaDB 内部程序创建的一些文档可能含有重名的字段，但是不会向现有的用户文档添加重名的键。

##字段类型##

每个字段的键值为字符串，而数值则可以为数字，字符串，嵌套JSON，嵌套数组等对象。SequoiaDB 所支持的数值类型见下表：

+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 数值类型       | 定义                                                   | 比较优先级权值 | 用例                                                          |
+================+========================================================+================+===============================================================+
| 整数           | 整数，范围 -2147483648 至 2147483647                  | 10             | { "key" : 123 }                                               |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 长整数         | 整数，范围 -9223372036854775808 至 9223372036854775807 | 10             | { "key" : 3000000000 }                                        |
|                | 如果用户指定的数值无法适用于整数，则 SequoiaDB 自动将  |                |                                                               |
|                | 其转化为浮点型                                         |                |                                                               |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 浮点数         | 浮点数，范围 -1.7E+308 至 1.7E+308                     | 10             | { "key" : 123.456 } 或 { "key" : 123e+50 }                    |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 字符串         | 双引号包含的字符串                                     | 15             | { "key" : "value" }                                           |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 对象 ID（OID） | 十二字节对象 ID                                        | 35             | { "key" : { "$oid" : "123abcd00ef12358902300ef" } }           |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 布尔           | true 或者 false                                        | 40             | { "key" : true } 或 { "key" : false }                         |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 日期           | YYYY-MM-DD 的日期形式                                  | 45             | { "key" : { "$date" : "2012-01-01" } }                        |
|                | 范围 1900-01-01 至 9999-12-31                          |                |                                                               |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 时间戳         | YYYY-MM-DD-HH.mm.ss.ffffff 的形式存取                  | 45             | { "key" : { "$timestamp" : "2012-01-01-13.14.26.124233" } }   |
|                | 范围 1902-01-01 00:00:00.000000 至                     |                |                                                               |
|                | 2037-12-31 23:59:59.999999                             |                |                                                               |
+---------------+---------------------------------------------------------+----------------+---------------------------------------------------------------+
| 二进制数据     | Base64 形式的二进制数据                                | 30             | { "key" : { "$binary" : "aGVsbG8gd29ybGQ=", "$type" : "1" } } |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 正则表达式     | 正则表达式                                             | 50             | { "key" : { "$regex" : "^张", "$options" : "i" } }            |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 对象           | 嵌套 JSON 文档对象                                     | 20             | { "key" : { "subobj" : "value" } }                            |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 数组           | 嵌套数组对象                                           | 25             | { "key" : [ "abc", 0, "def" ] }                               |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+
| 空             | null                                                   | 5              | { "key" : null }                                              |
+----------------+--------------------------------------------------------+----------------+---------------------------------------------------------------+


##字段顺序##

文档中的各字段无排列顺序，在进行数据操作时字段之间的顺序可能会被调换。

当表示嵌套对象中的某一个字段时，可以使用“.”（句号）在字段名之间进行分割。例如给定数据：

<pre class="prettyprint lang-diy">
{ "姓名" : "张三", "地址" : { "街道" : "水蓝街", "城市" : "xx", "省份" : "yy" } }</pre>

用户可以使用“地址.城市”字段名表示地址子对象中的城市字段。

**Note:**

-   每个文档的最大尺寸为16MB
-   文档中必须包括“_id”字段，如果用户没有提供该字段，系统会自动生成一个对象 ID 类型的字段
-   “_id”字段在集合内唯一
-   文档的字段名不可以“$”字符起始
-   文档的字段名不可以包含“.”字符
-   不同类型字段的值进行比较时，比较优先级权值越大，该类型的值就越大。

##数组##

###概念###

SequoiaDB 中的文档为 JSON 格式，一般又被称为记录。

###格式###

当记录中的某一字段对应多个数值是，用户可以使用数组结构存放数据。数组由“[”（左中括号）起始，至“]”（右中括号）结束，其中包含零个或多个数值。

<pre class="prettyprint lang-diy">
{ "字段名" : [ "<数值1>", "<数值2>", "<数值n>" ] }</pre>

###示例###

数组可以存放完全不相同的数据类型，其中每个记录以从0起始的下标表示。例如：

<pre class="prettyprint lang-diy">
{ "key" : [ "hello", "world" ] }</pre>

其中“hello”在数组中的下标为0，而“world”在数组中的下标为1。数组之中的数值有序，在进行数据操作时数组中的数值顺序不会改变。表示数组中某个元素时，可以使用“字段名.下标”的方式。

例如：如果希望表示 key 中“world”所在的数值，可以使用“key.1”作为字段名。


##对象 ID##

###概念###

对象 ID 为一个12字节的 BSON 数据类型，包括如下内容：

-   4字节精确到秒的时间戳
-   3字节系统（物理机）标示
-   2字节进程 ID
-   3字节由随机数起始的序列号  

<br>
<table>
    <tr>
        <th>4字节时间戳</th>
        <th>3字节系统标示</th>
        <th>2字节进程ID</th>
        <th>3字节序列号</th>
    </tr>
</table>

该对象 ID 可以在集群环境中，对每台系统中的每个进程，每秒钟标示16777216个不同的数值，因此基本可以认为在集群环境中全局唯一。

在 SequoiaDB 中，每个集合中存放的文档必须拥有一个 _id字段，并且该字段在集合中唯一。

###格式###

对象 ID 的表达形式如下：

<pre class="prettyprint lang-diy">
{ "$oid" : "<24字节16进制字符串>" }</pre>

###示例###

对象 ID 的显示结果如下：

<pre class="prettyprint lang-diy">
{ "key" : { "$oid" : "5156c192f970aed30c020000" } }</pre>

##日期##

###概念###

SequoiaDB 中的日期使用 YYYY-MM-DD的形式存取，在存储时将其转换为4字节的整数。

###格式###

日期的表达形式如下：

<pre class="prettyprint lang-diy">
{ "$date" : "&lt;YYYY-MM-DD&gt;" }</pre>

###示例###

<pre class="prettyprint lang-diy">
{ "createTime" : { "$date" : "2012-05-12" } }</pre>


##时间戳##

###概念###

SequoiaDB 中的时间戳使用 YYYY-MM-DD-HH.mm.ss.ffffff的形式存取，在存储时将其转换为8字节的整数。

###格式###

时间戳的表达形式如下：

<pre class="prettyprint lang-diy">
{ "$timestamp" : "&lt;YYYY-MM-DD-HH.mm.ss.ffffff&gt;" }</pre>

###示例###

<pre class="prettyprint lang-diy">
{ "createTime" : { "$timestamp" : "2012-05-12-13.15.21.241523" } }</pre>

##二进制数据##

###概念###

在 SequoiaDB 中的数据使用 JSON 形式访问，因此对于二进制的数据需要用户使用 Base64 方式进行编码，之后以字符串的形式发送至数据库。

###格式###

二进制数据的表达形式如下：

<pre class="prettyprint lang-diy">
{ "$binary" : "<数据>", "$type" : <类型> }</pre>

其中“数据”必须为 Base64编码的数据，“类型”为0-255之间的十进制数值，用户可以任意指定该范围之间的类型作为应用程序中的类型标示。

Base64 为一种通用的数据转换形式，主要将二进制数据转化为以纯 ASCII 字符串表示的字节流。一般来说转换之后的数据长度会大于原本的数据长度。

为了节省空间，在 SequoiaDB 的内部存放数据时，会将 Base64编码后的数据解码为原始数据进行存放。当用户读取数据时会再次将其转化为Base64 形式发送。

###示例###

字符串“hello world”被 Base64编码后的数据为“aGVsbG8gd29ybGQ=”。包含“helloworld”二进制数据，且类型为1的 JSON 数据为：

<pre class="prettyprint lang-diy">
{ "key" : { "$binary" : "aGVsbG8gd29ybGQ=", "$type" : "1" } }</pre>


##正则表达式##

###概念###

SequoiaDB 可以使用正则表达式检索用户数据。

###格式###

正则表达式输入的格式如下：

<pre class="prettyprint lang-diy">
{ "$regex" : "正则表达式", "$options" : "选项" }</pre>

其中“正则表达式”为一个正则表达式字符串，“选项”则参见下表：

  选项   描述
  ------ --------------------------------------------------------------------------------------------
  i      匹配时不区分大小写。
  m      允许进行多行匹配；当该参数打开时，字符“^”与“&”匹配换行符的之后与之前的字符。
  x      忽略正则表达式匹配中的空白字符；如果需要使用空白字符，在空白字符之前使用反斜线“\”进行转意。
  s      允许“.”字符匹配换行符。

当使用选项时，用户可以使用任意组合指定其中的选项。

###示例###

使用正则表达式进行大小写忽略，匹配以字符“W”起始的字符串，可以使用：

<pre class="prettyprint lang-diy">
{ "key" : { "$regex" : "^W", "$options" : "i" } }</pre>

关于正则表达式规则，请参阅 [Perl正则表达式手册](http://perldoc.perl.org/perlre.html)。
##概念##

集合（Collection）是数据库中存放文档的逻辑对象。任何一条文档必须属于一个且仅一个集合。

集合由“<集合空间名>.<集合名>”构成。集合名最大长度127字节，为 UTF-8编码。一个集合中可以包含零个至任意多个文档（上限为集合空间大小上限）。

在集群环境下，每个集合还可以拥有除名称外的以下属性：

  属性名                   描述
  ------------------------ -------------------------------------------------------------------------------------------------------------------------------------------------------
  分区键（ShardingKey）      指定集合的分区键，集合中所有的文档将分区键中指定的字段作为分区信息，文档分别存放在所对应的分区中。
  分区类型（ShardingType）   指定集合的分区类型：范围分区（Range）或散列分区（Hash）。
  写副本数（ReplSize）       指定该集合默认的写副本数。如果该值 ≤ 1，数据的写请求在一个副本写入成功后就会返回。如果该值 > 1，则需要等到至少指定数量的副本被成功写入数据后才会返回。
  数据压缩（Compressed）     创建集合时，指定 Compressed 属性的值代表着在做插入操作时，是否以压缩的形式存储数据，它的值有 true 和 false，默认为 false。
##概念##

集合空间（CollectionSpace）是数据库中存放集合的物理对象。任何一个集合必须属于一个且仅一个集合空间。

集合空间名最大长度127字节，为 UTF-8编码。一个集合空间中可以包含不超过4096个集合；每个数据节点可以包含不超过4096个集合空间。

每一个集合空间在数据节点均对应一个文件，文件名格式为“<集合空间名>.1”。

##数据页##

集合空间将文件划分为若干个固定大小的数据页（Page），在创建集合空间时用户可以指定数据页大小，且创建后不可更改。

每个数据节点中，单个集合空间可以访问16777216个数据页。因此对应不同数据页大小，单分区中集合空间容量上限为：

  数据页大小（字节）   集合空间最大容量（GB）
  ------------------ ----------------------
  4096               512
  8192               1024
  16384              2048
  32768              4096
  65536              8192

##数据块##

一个或多个数据页组成数据块。集合空间中的每个集合由零个或以上的数据块构成，每个数据块的大小由用户数据长度自动调整。集合中的文档不可跨多个数据块存放。

集合空间中的数据块存放方式如下图所示：

![](collectionspace_extent.jpg)

图中显示了一个集合空间中的三个集合，分别用不同的颜色代表。每个集合所对应的数据存放在各自的数据页中。一个或多个数据页可以组成一个数据块，每个数据块中的数据连续，且文档不能跨越多个数据块。
##概念##

大对象（LOB）功能旨在突破 SequoiaDB 的单条最大记录长度为 16MB
的限制，为用户写入和读取更大型记录提供便利。LOB 记录的大小目前不受限制。

每一个 LOB 记录拥有一个 OID，通过指定集合及 OID 可以访问一条 LOB
记录。在非分区集合及哈希分区集合中均可使用 LOB 功能。集合间不共享 LOB
记录。当一个集合被删除时，其拥有的 LOB 记录自动删除。

LOB 记录的存储格式：

![](lob.jpg)

每个 LOB 记录包含若干个分片。分片所占空间大小均为 LobPageSize（创建集合空间时指定）。在哈希分区中，LOB 记录的每一个分片会被按照 OID 加分片序号分散存储在相应的分区组中。其哈希空间与所属集合的哈希空间相同。

目前 LOB 的存储格式为二进制类型。

##支持的操作##

  操作   备注
  ------ -------------------------------------------
  创建   LOB 记录一旦创建完毕，其内容无法再做更改。
  读取   支持 seek 操作。
  删除   无

##示例##

在 Sdb Shell 中将本地文件 mylob 上传至集合 foo.bar 中：

<pre class="prettyprint lang-javascript">
> db.foo.bar.putLob('/opt/mylob');</pre>

在 Sdb Shell 中将集合 foo.bar 中的 OID 为 5435e7b69487faa663000897 的LOB 记录下载到本地文件 mylob 中：

<pre class="prettyprint lang-javascript">
> db.foo.bar.getLob('5435e7b69487faa663000897','/opt/newlob')</pre>
##sdbcm 概述##

数据库集群控制器（SequoiaDB Cluster Manager）是一个守护进程，在 Windows 中它是以服务的方式常驻系统后台。SequoiaDB 的所有集群管理操作都必须有 sdbcm 的参与，目前每一台物理机器上只能启动一个 sdbcm 进程，负责执行远程的集群管理命令和监控本地的 SequoiaDB 数据库。sdbcm 主要有两大功能：

* 远程启动，关闭，创建和修改节点：通过 SequoiaDB 客户端或者驱动连接数据库时，可以执行启动，关闭，创建和修改节点的操作，该操作向指定节点物理机器上的 sdbcm 发送远程命令，并得到 sdbcm 的执行结果。

* 本地监控：对于通过 sdbcm 启动的节点，都会维护一张节点列表，其中保存了所有本地节点的服务名和启动信息，如启动时间、运行状态等。如果某个节点是非正常终止的，如进程被强制终止，引擎异常退出等，sdbcm 会尝试重启该节点。

##sdbcm 操作##

* 配置文件

	在数据库安装目录的 conf 子目录下，有一个 sdbcm.conf 的配置文件，该文件给出了启动 sdbcm 时的配置信息，如下所示：

+-----------------------+-----------------------------------------------------------------------------------------+-----------------------------+
| 参数                  | 描述                                                                                    | 示例                        |
+=======================+=========================================================================================+=============================+
| defaultPort           | sdbcm 的默认监听端口                                                                    | defaultPort=11790           |
+-----------------------+-----------------------------------------------------------------------------------------+-----------------------------+
| &lt;hostname&gt;_Port | 物理主机 hostname 上 sdbcm 的监听端口                                                   | &lt;hostname&gt;_Port=11790 |
|                       | 若在该配置文件中找不到对应主机的参数，sdbcm 会以 defaultPort 启动                       |                             |
|                       | 若 defaultPort 不存在，则 sdbcm 以默认端口11790启动                                     |                             |
+-----------------------+-----------------------------------------------------------------------------------------+-----------------------------+
| RestartCount          | 重启次数，即定义 sdbcm 对节点的最大重启次数                                             | RestartCount=5              |
|                       | 该参数不存在时默认置为-1，即不断重启                                                    |                             |
+-----------------------+-----------------------------------------------------------------------------------------+-----------------------------+    
| RestartInterval       | 重启间隔，即定义 sdbcm 的最大重启间隔，单位是分钟                                       | RestartInterval=0           |
|                       | 该参数与 RestartCount 结合定义了重启间隔内 sdbcm 对节点的最大重启次数，超出时则不再重启 |                             |
|                       | 该参数不存在时默认置为0，即不考虑重启间隔                                               |                             |
+-----------------------+-----------------------------------------------------------------------------------------+-----------------------------+

* 启动 sdbcm

	运行 sdbcmart 命令可以启动 sdbcm。


* 关闭 sdbcm

	运行 sdbcmtop 命令可以关闭 sdbcm。
##概念##

协调节点为一种逻辑节点，其中并不保存任何用户数据信息。

协调节点作为数据请求部分的协调者，本身并不参与数据的匹配与读写操作，而仅仅是将请求分发到所需要处理的数据节点。

一般来说，协调节点的处理流程如下：

-   得到请求
-   解析请求
-   本地缓存查询该请求对应集合的信息
-   如果信息不存在则从编目节点获取
-   将请求转发至相应的数据节点
-   从数据节点得到结果
-   把结果汇总或直接传递给客户端

协调节点与其它节点之间主要使用分区服务端口（shardname 参数）进行通讯。

##新增协调节点##

当集群规模扩大时，协调节点也需要随着规模的增加而进行增加。建议匹配时，一台物理节点，配置一个协调节点。

1.创建协调节点配置目录；

<pre class="prettyprint lang-javascript">
$ mkdir -p /opt/sequoiadb/conf/local/11810</pre>

其中11810为协调节点的服务端口，可根据需要配置

2.拷贝协调节点样例配置文件；

<pre class="prettyprint lang-javascript">
$ cp /opt/sequoiadb/conf/samples/sdb.conf.coord /opt/sequoiadb/conf/local/11810/sdb.conf</pre>

3.修改配置文件；

<pre class="prettyprint lang-javascript">
$ vi /opt/sequoiadb/conf/local/11810/sdb.conf</pre>

修改内容

<pre class="prettyprint lang-diy">
# database path dbpath=/opt/sequoiadb/database/coord</pre>

该参数为数据库放置路径，可根据需要修改，请确保路径已经存在（不存在请手工创建）

将如下行：

<pre class="prettyprint lang-diy">
# catalog addr(hostname1:servicename1,hostname2:servicename2,...)
# catalogaddr=</pre>

**修改**

<pre class="prettyprint lang-diy">
# catalog addr(hostname1:servicename1,hostname2:servicename2,...)
catalogaddr=sdbserver1:11803,sdbserver2:11803,sdbserver3:11803</pre>

该参数为Catalog服务地址和端口

4.按 :wq，保存退出 vi；

5.创建数据文件存放路径；

<pre class="prettyprint lang-javascript">
$ mkdir -p /opt/sequoiadb/database/coord</pre>

路径为上一步骤配置的路径

6.启动协调节点进程。

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdbstart -c /opt/sequoiadb/conf/local/11810/</pre>

##故障恢复##

由于协调节点不存在用户数据，因此发生故障后可以直接重新启动，不参与任何额外的故障恢复步骤。
##概念##

编目节点为一种逻辑节点，其中保存了数据库的元数据信息，而不保存其他用户数据。

编目节点中包含4个集合空间：

-   SYSCAT 系统编目集合空间，包含4个系统集合：

      集合名                         描述
      ----------------------         ------------------------------------------
      SYSCOLLECTIONS                 保存了该集群中所有的用户集合信息
      SYSCOLLECTIONSPACES            保存了该集群中所有的用户集合空间信息
      SYSNODES                       保存了该集群中所有的逻辑节点与复制组信息
      SYSTASKS                       保存了该集群中所有正在运行的后台任务信息

-   SYSTEMP 系统临时集合空间，可以创建最多4096个临时集合

-   SYSAUTH
    系统认证集合空间，包含一个用户集合，保存当前系统中所有的用户信息

      集合名              描述
      --------------      ------------------------------
      SYSUSRS             保存了该集群中所有的用户信息

-   SYSPROCEDURES
    系统存储过程集合空间，包含一个集合，用于存储所有的存储过程函数信息

      集合名                     描述
      ------------------         --------------------------
      STOREPROCEDURES            保存所有存储过程函数信息

除了编目节点外，集群中所有其他的节点不在磁盘中保存任何全局元数据信息。当需要访问其他节点上的数据时，除编目节点外的其他节点需要从本地缓存中寻找集合信息，如果不存在则需要从编目节点获取。

编目节点与其它节点之间主要使用编目服务端口（catalogname参数）进行通讯。

##SYSCOLLECTIONS 集合##

###所属集合空间###

SYSCAT

###概念###

SYSCOLLECTIONS集合中包含了该集群中所有的用户集合信息。每个用户集合保存为一个文档。

每个文档包含以下字段：

|字段名|类型|描述|
|:----|:----|:----|
|Name|字符串|集合的完整名，为&lt;集合空间&gt;.&lt;集合名&gt;形式。|
|Version|整数|集合的版本号，由1起始，每次对该集合的元数据变更会造成版本号+1。|
|ReplSize|整数|最小复制组，确保任何写操作必须被复制到至少指定数量的节点后返回成功。|
|ShardingKey|对象|分区键，在分区集合中存在。对象包含一个或多个字段，字段名为分区字段名，数值为1或者-1，代表对该列正向或逆向排序。|
|ShardingType|字符串|分区类型，在分区集合中存在。分区类型有：范围分区（Range）和散列分区（Hash）两种。|
|Partition|整数|散列分区的分区大小值，必须为2的幂。|
|CataInfo|数组|集合所在的逻辑节点信息。在单分区集合中，该数组仅包含一个元素，代表该集合所在的分区组。在多分区集合中，该数组中包含一个或多个元素，代表该集合中的每一个取值范围所在的分区组。每个取值范围包括 LowBound 与UpBound，代表其下限与上限，闭合关系为左闭右开。|

###示例###

一个典型的单分区集合信息如下：

<pre class="prettyprint lang-diy">
{ "Name" : "test.foo", "Version" : 1, "CataInfo" : [ { "GroupID" : 1000 } ] }</pre>

一个典型的多分区集合信息如下：

<pre class="prettyprint lang-diy">
{ 
  "Name" : "foo.test",
  "Version" : 1,
  "ShardingKey" : { "Field1" : 1, "Field2" : -1 },
  "ShardingType" : "range" ,
  "ReplSize": 3,
  "CataInfo" : 
  [
    { 
      "GroupID" : 1000,
      "LowBound" : { "" : MinKey, "" : MaxKey },
      "UpBound" : { "" : MaxKey, "" : MinKey } 
    } 
  ]
}</pre>

##SYSCOLLECTIONSPACES 集合##

###所属集合空间###

SYSCAT

###概念###

SYSCOLLECTIONSPACES集合中包含了该集群中所有的用户集合空间信息。每个用户集合空间保存为一个文档。

每个文档包含以下字段：

  字段名       类型     描述
  ------------ -------- ----------------------------------------------------------------------------------------
  Name         字符串   集合空间名。
  Collection   数组     该集合空间中包含的所有的集合名，每个集合为一个 JSON 对象，包含“Name”字段与相应的集合名。
  Group        数组     该集合空间所在的复制组 ID。
  PageSize     整数     该集合空间的数据页大小。

###示例###

一个典型的包含一个集合，存放在一个复制组中的集合空间如下：

<pre class="prettyprint lang-diy">
{ 
  "Collection" : [ { "Name" : "foo" } ],
  "Group" : [ { "GroupID" : 1000 } ],
  "Name" : "test",
  "PageSize" : 4096
}</pre>

##SYSNODES 集合##

###所属集合空间###

SYSCAT

###概念###

SYSNODES集合中包含了该集群中所有的节点与复制组信息。每个复制组保存为一个文档。

每个文档包含以下字段：

+-------------+--------+---------------------------------------------------+
| 字段名      | 类型   | 描述                                              |
+=============+========+===================================================+
| GroupName   | 字符串 | 复制组名。                                        |
+-------------+--------+---------------------------------------------------+
| GroupID     | 整数   | 复制组 ID，该 ID 在集群中唯一。                   |
+-------------+--------+---------------------------------------------------+
| PrimaryNode | 整数   | 该复制组内主节点 ID。                             |
+-------------+--------+---------------------------------------------------+
| Role        | 整数   | 复制组角色，可以为：                              |
|             |        |                                                   |
|             |        |  -   0：数据节点                                  |
|             |        |  -   2：编目节点                                  |
+-------------+--------+---------------------------------------------------+
| Status      | 整数   |  -   1：已激活复制组                              |
|             |        |  -   0：未激活复制组                              |
|             |        |  -   不存在：未激活复制组                         |
+-------------+--------+---------------------------------------------------+
| Version     | 整数   |  版本号，由1起始，任何对该复制组的操作均会对其+1。|
+-------------+--------+---------------------------------------------------+
| Group       | 数组   |  复制组中节点信息，见下表：                       |
+-------------+--------+---------------------------------------------------+

复制组中如果存在一个以上节点，则每个节点作为一个对象存放在 Group字段数组中，每个对象的信息如下：

+----------+--------+----------------------------------------------------------------------------------------------------------------------+
| 字段名   | 类型   | 描述                                                                                                                 |
+==========+========+======================================================================================================================+
| HostName | 字符串 | 节点所在的系统名，需要完全匹配该节点所在操作系统中“hostname”命令的输出。                                             |
+----------+--------+----------------------------------------------------------------------------------------------------------------------+
| dbpath   | 字符串 | 数据库路径，为节点所在的物理节点中对应的绝对路径。                                                                   |
+----------+--------+----------------------------------------------------------------------------------------------------------------------+
| NodeID   | 整数   | 节点 ID，该 ID 在集群中唯一。                                                                                        |
+----------+--------+----------------------------------------------------------------------------------------------------------------------+
| Service  | 数组   | 服务名，每个逻辑节点对应4个服务名，每个服务名包括其类型与服务名（可以为端口号或services 文件中的服务名）。类型如下： |
|          |        |                                                                                                                      |
|          |        |  -   0：直连服务，对应数据库参数 svcname                                                                             |
|          |        |  -   1：复制服务，对应数据库参数 replname                                                                            |
|          |        |  -   2：复制服务，对应数据库参数 shardname                                                                           |
|          |        |  -   3：编目服务，对应数据库参数 catalogname                                                                         |
+----------+---------------------------------------------+---------------------------------------------------------------------------------+

**Note:**

-   编目复制组名固定为“SYSCatalogGroup”，复制组ID固定为1。

-   数据复制组 ID 由1000起始。

-   数据节点 ID 由1000起始。

###示例###

一个典型的包含单节点的编目复制组为：

<pre class="prettyprint lang-diy">
{ 
  "Group" : 
  [
    { 
      "NodeID" : 2,
      "HostName" : "vmsvr1-rhel-x64",
      "Service" : 
      [
        { "Type" : 3, "Name" : "11803" },
        { "Type" : 1, "Name" : "11801" },
        { "Type" : 2, "Name" : "11802" },
        { "Type" : 0, "Name" : "11800" } 
      ],
      "dbpath" : "/home/sequoiadb/sequoiadb/catalog"
    } 
  ],
  "GroupID" : 1,
  "GroupName" : "SYSCatalogGroup",
  "PrimaryNode" : 2,
  "Role" : 2,
  "Version" : 1 
}</pre>

一个典型的包含单节点的数据复制组为：

<pre class="prettyprint lang-diy">
{ 
  "Group" : 
    [
      { 
      "dbpath" : "/home/sequoiadb/sequoiadb/data3",
      "HostName" : "vmsvr1-rhel-x64",
      "Service" : 
        [
          { "Type" : 0, "Name" : "11820" },
          { "Type" : 1, "Name" : "11821" },
          { "Type" : 2, "Name" : "11822" },
          { "Type" : 3, "Name" : "11823" } 
        ],
      "NodeID" : 1001 
      } 
    ],
  "GroupID" : 1001,
  "GroupName" : "foo1",
  "PrimaryNode" : 1001,
  "Role" : 0,
  "Status" : 1,
  "Version" : 1 
}</pre>

##SYSTASKS 集合##

###所属集合空间###

SYSCAT

###概念###

SYSTASKS集合中包含了该集群中所有正在运行的后台任务信息。每个任务保存为一个文档。

每个文档包含以下字段：

+-----------------+--------+---------------------------------------------------+
| 字段名          | 类型   | 描述                                              |
+=================+========+===================================================+
| JobType         | 整数   | 任务类型，分别代表：                              |
|                 |        |                                                   |
|                 |        |  -   0：数据切分                                  |
+-----------------+--------+---------------------------------------------------+
| Status          | 整数   | 任务状态，分别代表：                              |
|                 |        |                                                   |
|                 |        |  -   0：准备                                      |
|                 |        |  -   1：运行                                      |
|                 |        |  -   2：暂停                                      |
|                 |        |  -   3：取消                                      |
|                 |        |  -   4：变更元数据                                |
|                 |        |  -   9：完成                                      |
|                 |        |  -   不存在：未激活复制组                         |
+-----------------+--------+---------------------------------------------------+
| CollectionSpace | 字符串 |  集合空间名                                       |
+-----------------+--------+---------------------------------------------------+
| Collection      | 字符串 |  集合名                                           |
+-----------------+--------+---------------------------------------------------+

###数据切分###

对于数据切分操作，每个文档还存在以下字段：

  字段名       类型     描述
  ------------ -------- ----------------------
  SourceName   字符串   源分区所在复制组名
  TargetName   字符串   目标分区所在复制组名
  SourceID     整数     源分区所在复制组ID
  TargetID     整数     目标分区所在复制组ID
  SplitValue   对象     数据分区键


##SYSUSRS 集合##

###所属集合空间###

SYSAUTH

###概念###

SYSUSRS 集合中包含了该集群中所有注册用户的信息。每个用户保存为一个文档。

每个文档包含以下字段：

  字段名     类型     描述
  ---------- -------- ---------------------------------
  User       字符串   用户名。
  Password   字符串   对用户密码进行 MD5 散列的结果。

**Note:**

如果该集合为空，则对任何连接不进行身份认证。

##STOREPROCEDURES 集合##

###所属集合空间###

SYSPROCEDURES

###概念###

STOREPROCEDURES集合中包含了所有的存储过程函数，每一个函数保存为一个文档，每个文档包含以下字段：

+-----------------+--------+---------------------------------------------------+
| 字段名          | 类型   | 描述                                              |
+=================+========+===================================================+
| name            | 字符串 | 函数名                                            |
+-----------------+--------+---------------------------------------------------+
| func            | 字符串 | 函数体                                            |
+-----------------+--------+---------------------------------------------------+
| funcType        |  整数  | 函数类型                                          |
|                 |        |                                                   |
|                 |        |  -   0：代表 JavaScript 函数                      |
|                 |        | 其他类型暂无                                      |
+-----------------+--------+---------------------------------------------------+

###示例###

一个简单的存储过程函数如下：

<pre class="prettyprint lang-diy">
{
  "_id" : { "$oid" : "5257b115925c31dd16ec4e4a" },
  "name" : "fun",
  "func" : "function fun(num) {
      if (num == 1) {
          return 1;
      } else {
          return fun(num - 1) * num;
      }
  }",
  "funcType" : 0 
}</pre>

#编目分区组中新增节点#

**Note:**

如果新增节点涉及到新增主机，则请首先按照在[集群中新增主机](SdbDoc_Cn/installation/create_host.html)一节完成主机的主机名和参数配置。

随着整个集群中的物理设备的扩展，可以通过增加更多的编目节点来提高编目服务的可靠性。

操作方法：

<pre class="prettyprint lang-javascript">
> var cataRG = db.getRG("SYSCatalog");
> var node1 = cataRG.createNode(&lt;host&gt;,&lt;service&gt;,&lt;dbpath&gt;,[config]);
> node1.start()</pre>

-   第一条命令用于获取编目分区组，“SYSCatalogGroup”为编目分区组组名；

-   第二条命令用于创建一个新的编目节点，其中：

    **参数：**

    **host**：指定编目节点的主机名；

    **service**：指定编目节点的服务端口，请确保该端口号，以及往后延续的3个端口号未被占用；如设置为11800，请确保11800/11801/11802/11803端口都未被占用；

    **dbpath**：数据文件路径，用于存放编目数据文件，请确保数据管理员（安装时创建，默认为sdbadmin）用户有写权限；

    **config**：该参数为可选参数，用于配置更多细节参数，格式必须为 json格式，参数参见[数据库配置](SdbDoc_Cn/database_management/runtime_configuration.html)一节；如需要配置日志大小参数｛logfilesz:64｝。

-   第三条命令用于启动新增的编目节点。


#新建编目分区组#

**Note:**

如果新增节点涉及到新增主机，则请首先按照在[集群中新增主机](SdbDoc_Cn/installation/create_host.html)一节完成主机的主机名和参数配置。

一个数据库集群必须有且仅有一个编目分区组，所以新建分区组往往在安装时就已经完成，不需要在安装后执行新建分区组操作。实例见安装指南[集群模式的配置与启动](SdbDoc_Cn/installation/deployment/command_installation/cluster.html)一节。

操作方法：

<pre class="prettyprint lang-javascript">
> db.createReplicaCataGroup(&lt;host&gt;,&lt;service&gt;,&lt;dbpath&gt;,[config])</pre>

该命令用于创建编目分区组，同时创建并启动一个编目节点，其中：

-   **host**：指定编目节点的主机名；

-   **service**：指定编目节点的服务端口，请确保该端口号，以及往后延续的3个端口号未被占用；如设置为11800，请确保11800/11801/11802/11803端口都未被占用；

-   **dbpath**：数据文件路径，用于存放编目数据文件，请确保数据管理员（安装时创建，默认为sdbadmin）用户有写权限。如果配置路径不以“/”开头，数据文件存放路径将是数据库管理员用户(默认为sdbadmin)的主目录(默认为/home/sequoiadb) + 配置的路径；

-   **config**：该参数为可选参数，用于配置更多细节参数，格式必须为 json格式，参数参见[数据库配置](SdbDoc_Cn/database_management/runtime_configuration.html)一节；如需要配置日志大小参数｛logfilesz:64｝。

#故障恢复#

编目节点故障恢复策略与[数据节点](SdbDoc_Cn/infrastructure/data_node.html)相同。
##概念##

数据节点为一种逻辑节点，其中保存用户数据信息。

数据节点中没有专门的编目信息集合，因此第一次访问集合前需要向编目节点请求该集合的元数据信息。

在独立模式中，数据节点为单独的服务提供者，直接与应用程序或客户端进行通讯，并且不需要访问任何编目信息。

##新增数据分区组##


如果新增节点涉及到新增主机，则请首先按照在[集群中新增主机](SdbDoc_Cn/installation/create_host.html)一节完成主机的主机名和参数配置。

一个集群中可以配置多个分区组，最大可配置60,000个分区组。通过增加分区组，可以充分利用物理设备进行水平扩展，理论上SequoiaDB 可以做到线性的水平扩展能力。

操作方法：

<pre class="prettyprint lang-javascript">
> var dataRG = db.createRG("datagroup1")
> dataRG.createNode("sdbserver1",11820,"/opt/sequoiadb/database/data/11820")
> dataRG.start()</pre>

-   第一条命令用于创建数据分区组，与编目分区组不同的是，该操作不会创建任何数据节点，其中参数为数据组名；

-   第二条命令用户在数据组中新增一个数据节点，可以根据需要多次执行该命令来创建多个数据节点。

	其中：

	**host**：指定数据节点的主机名；

	**service**：指定数据节点的服务端口，请确保该端口号，以及往后延续的3个端口号未被占用；如设置为11820，请确保11820/11821/11822/11823端口都未被占用；

	**dbpath**：数据文件路径，用于存放数据节点的数据文件，请确保数据管理员（安装时创建，默认为sdbadmin）用户有写权限；

	**config**：该参数为可选参数，用于配置更多细节参数，格式必须为 json 格式，参数参见[数据库配置](SdbDoc_Cn/database_management/runtime_configuration.html)一节；如需要配置日志大小参数｛logfilesz:64｝。

-   第三条命令用于启动数据分区组，该命令将该组的所有节点启动，并提供服务。

##分区组中新增节点##


如果新增节点涉及到新增主机，则请首先按照在[集群中新增主机](SdbDoc_Cn/installation/create_host.html)一节完成主机的主机名和参数配置。

某些分区组可能在创建时设定的副本数较少，随着物理设备的增加，可能需要增加副本数以提高分区组数据可靠性。

操作方法：

<pre class="prettyprint lang-javascript">
> var dataRG = db.getRG(<groupname>);
> var node1 = dataRG.createNode(< host >,< service >,< dbpath >,[config]);
> node1.start();</pre>

-   第一条命令用于获取数据分区组，参数 groupname 为数据分区组组名；

-   第二条命令用于创建一个新的数据节点，参数与[新增编目分区组中的创建节点](SdbDoc_Cn/infrastructure/catalog_node.html)参数相同；

-   第三条命令用于启动新增的数据节点。

##故障恢复##

数据节点发生故障后，重新启动会自动检测数据库目录下 .SEQUOIADB_STARTUP隐藏文件。

如果该文件存在则说明上次的执行意外终止（例如 kill -9）。对于意外终止的节点，系统会将该数据节点置入崩溃恢复状态。

在崩溃恢复的过程中，数据节点会与该组中的一个正常节点进行全量同步。在这种情况下，被恢复的节点中所有数据作废，同步到的新数据作为基准。假设该节点没有被意外终止（例如kill -15），则进入增量同步状态。在这种情况下，如果当前其它数据节点中包含的最老日志已经比被恢复节点新，则进入全量同步状态，否则只同步增量日志。

如果该数据组中所有节点都被意外终止，则需要以独立模式启动一个节点进行本地恢复。在该模式中，数据会被导出并再次导入，以过滤掉所有可能出现的数据损坏。当其中一个节点被本地恢复后，需要将其数据目录拷贝入其它所有数据节点。
##概念##

分区组又被称为复制组，一个复制组内可以包含一个或多个数据节点（或编目节点），节点之间的数据使用异步日志复制机制，保持最终一致。

分区组中所有的节点之间使用复制服务端口（replname参数）进行通讯，定期相互发送心跳信息以相互验证状态。

分区组结构如下图：

![](sequoiadb_infrastructure_shard1.jpg)

每个分区组内的节点有两种状态：

**主节点:**主节点可作读写操作。所有写入的数据会同步写入日志文件，日志文件中的日志信息会异步写入从节点。
**从节点:**从节点可作只读操作。所有主节点写入的数据会异步写入从节点。因此从节点与主节点之间可能存在暂时的数据不一致，但是复制机制可以保证数据的最终一致性。

分区组通过**数据复制**，**读写分离**与**选举机制**实现高可用。
##概念##

选举机制保障分区组中随时存在一个主节点。当该主节点宕机后会在其余从节点之间自动选举出主节点，进行读写操作。

选举机制的核心为节点状态监测。分区组内所有的节点定期向组内其他成员发送自身状态，因此当主节点宕机后，所有的从节点间会进行投票，当时最匹配原主节点的节点即当选新的主节点。

![](sequoiadb_infrastructure_shard2.jpg)

选举成功的前提条件为组内必须拥有超过半数以上的节点参与投票，否则为了避免“双活”问题（同时存在两个主节点）将无法进行选举。

任何时刻如果组内成员不足半数，则当前的主节点会自动降级为从节点，同时断开当前节点的所有用户连接。

当一个新的节点加入现存的分区组，或者某个故障节点重新加入分区组后，会进行[数据同步](SdbDoc_Cn/infrastructure/replication/replicate.html)。
##概念##

数据复制为分区组中节点之间的相互同步的机制。

在数据节点和编目节点中，任何数据增删改操作均会写入日志。SequoiaDB会首先将日志写入日志缓冲区，然后将其异步写入本地磁盘。

每个数据复制会在两个节点间进行：

**源节点:**为包含新数据的节点。主节点并不一定永远是复制的源节点。
**目标节点:**为请求进行数据复制的节点。

复制过程中，目标节点选择一个与其最接近的节点，然后向其发送一个复制请求。源节点接到复制请求后，会将目标节点请求的同步点之后的日志记录打包并发送给目标节点，目标节点接收到数据包后会重新处理日志中的所有操作。

节点之间的复制有两个状态：

-   对等状态（PEER）：当目标节点请求的日志依然存在于源节点的日志缓冲区中，两节点之间为对等状态
-   远程追赶状态（RemoteCatchup）：当目标节点请求的日志不存在于源节点的日志缓冲区中，但依然存在于源节点的日志文件中，两节点之间为远程追赶状态

如果目标节点请求的日志已经不再存在于源节点的日志文件中，目标节点则进入全量同步状态。

当两节点处于对等状态时，同步请求在源节点可以直接从内存中获取数据，因此目标节点选择复制源节点时，总会尝试选择距离自己当前日志点最近的数据节点，使其所包含的日志尽量坐落在内存中。


##全量同步##

###概念###

在分区组内，当一个新的节点加入分区组，或者故障节点重新加入分区组，需要进行数据全量同步，以保障新的节点与现有节点之间数据的一致性。

在进行数据全量同步时有两个节点参与：

**源节点:**
  ~ 为包含有效数据的节点。主节点并不一定永远是同步的源节点。任何与主节点处于同步状态的从节点均可作为源节点进行数据同步。（目前只能主节点作为同步源节点）目标节点
  ~ 为新加入组，或重新入组的故障节点。同步时该节点下原有的数据会被废弃。

![](sequoiadb_infrastructure_shard3.jpg)

全量同步发生时，目标节点会定期向源节点请求数据。源节点将数据打包后作为大数据块发送给目标节点。当目标节点重做该数据块内所有数据后，向源节点请求新的数据块。

为保障源节点在同步时可进行写操作，所有已经被发送给目标节点的数据页如果被更改，其更新会被同步到目标节点，以保障全量同步过程中更新的数据不会损失。


##实例##

**复制组实例**

复制组中的每个数据节点都存储该复制组的一份完整数据，因此也称复制组中的每个节点为复制组实例。复制组实例可根据节点在复制组中的位置分为“主”，“备”或“0~7”标识。

**数据库实例**

所有复制组中相同位置的复制组实例共同构成数据库实例，因此数据库实例也可以分为“主”，“备”或“0~7”标识。
##写请求处理##

所有写请求都只会发往主节点，如果没有主节点则当前复制组不可处理写请求。

##读请求处理##

读请求会按照会话（连接）随机选择组内任意一个节点（对外透明），或按照当前会话（连接）配置的优先实例策略选取相应复制组的数据节点。在一次会话中如果上一次查询（包括 query 和 fetch）返回成功，则下一次查询不会重选节点；如果上一次查询发生失败，则下一次查询将重选节点。如果没有可用节点则返回失败。一次查询中不会重选节点。

##最终一致性策略##

为了提升数据的可靠性和实现数据的读写分离，SequoiaDB中，对于复制组间的数据采用“最终一致性”策略，在读写分离时读取的数据某一个时期内可能不是最新的，但最终是一致的。

**名词解释**

**W**：副本写入个数

**R**：副本读取个数

**N**：副本个数

在 SequoiaDB 中，设置 R 的值为1，且不可配置。

默认情况下，复制组中的主节点在处理完一个写请求后会立即返回，即 W =1。数据同步会在后台异步完成（[同步日志](SdbDoc_Cn/database_management/log_synchronization.html)并达到最终一致。此时外部的读请求获得的数据可能不是最新的。在对数据一致性要求不高的场景中，这种方式可以提供最优的写入性能。

当我们[创建集合](SdbDoc_Cn/reference/Sequoiadb_command/SdbCS/createCL.html)时，可以通过ReplSize 属性指定集合的 W 值。

-   默认情况下 W = 1。
-   当 ReplSize 等于0时，W 的个数会根据当前复制组的 N变化而变化。即，如果开始组内有三个节点，则 W 等于3。当新增加一个入节点时，W 会自动变为4。
-   当手动指定 W 的个数时，不能超出当前复制组内节点个数。

增大 W 可以有效提高数据的一致性和可靠性。当 W = N并且写请求处理成功后，后续读到的数据一定是当前组内最新的。但是这样会降低复制组的写入性能。值得注意的是，虽然我们可以将W 设为 N，但这并不代表 SequoiaDB中的数据拥有强一致性。当某个副本写入失败（如磁盘满）时，复制组内可能存在多个版本的数据。此时既可能读到新的数据，也可能读到旧的数据。当失败副本恢复正常后，会继续从主节点上同步最新的数据并达到最终一致。
后台任务是 SequoiaDB中的一种特殊任务类型，一般用于将特定用户操作置于后台异步执行。在快照中，后台任务的类型（Type）为“Task”。

后台任务类型列表：

  任务名               描述
  -------------------- ----------------------------------------------------------------------------------------------------------------------------------
  Restore              数据库恢复任务，用于根据日志文件回滚恢复数据库。
  Job[PageCleaner]     脏页清除任务，用于异步将未写入磁盘的脏页刷入磁盘。可以使用 -numpagecleaners 控制脏页清除任务数量，默认为1。
  Job[Prefetch]        预取任务，用于在等待客户端接收下一个操作请求时，在后台执行用户接下来可能发生的操作。可以使用 -maxprefpool 控制最大预取任务的数量。
  CreateIndex          建立索引任务，用于后台建立索引，多用于备节点重做主节点的建立索引操作日志。
  DropIndex            删除索引任务，用于后台删除索引，多用于备节点重做主节点的删除索引操作日志。
  CleanUp              数据清理任务，多用于数据切分后，在源数据节点删除被切分数据。
  Job[ExtendSegment]   扩展集合空间文件任务，用于当集合空间空闲数据页小于特定阀值后，由后台启动异步扩充集合空间。
##概念##

域（Domain）是由若干个复制组（ReplicaGroup）组成的逻辑单元。每个域都可以根据定义好的策略自动管理所属数据，如数据切片和数据隔离等。

当域中的复制组为0个时，称作空域。空域中不能创建集合空间。

一个复制组可以属于多个域。

在逻辑上存在一个系统域称作“SYSDOMAIN”。当前系统所有复制组都属于系统域。用户创建域时不能使用“SYSDOMAIN”作为域名，也不能直接操作系统域。

域拥有除名称外的以下属性：

  属性名      描述
  ----------- ---------------------------------------------------------------------------------
  AutoSplit   当此属性为 True 时，在该域上创建的散列分区集合会被自动切分至包含的所有复制组上。
##参数说明##

  参数名                缩写   类型      说明
  --------------------- ------ --------- ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  --help                -h     --        打印帮助
  --dbpath              -d     str       1.指定数据文件存放路径。2.如果不指定，则默认为当前路径。
  --indexpath           -i     str       1.指定索引文件存放路径。2.如果不指定，则默认与'dbpath'相同。
  --confpath            -c     str       1.指定配置文件路径（不包含文件名），系统会在confpath下寻找sdb.conf。 2.sdb.conf中填入需要的配置项，配制方法为：参数名 = 参数值。如 svcname=11810；diaglevel=3 3.如果不指定此参数，系统默认在当前路径寻找sdb.conf。 4.sdb.conf可以不存在。
  --logpath             -l     str       1.副本节点在进行数据同步时会生成同步日志。此参数用来指定同步日志的路径。 2.如果不指定，则默认路径为：数据文件路径/replicalog
  --diagpath            --     str       1.指定诊断日志存放目录。 2.如果不指定，则默认为：数据文件路径/diaglog
  --diagnum             --     num       1.指定诊断日志文件最大数量。 2.如果不指定，则默认为：20，-1表示不限制。
  --bkuppath            --     str       1.指定备份文件生成目录。 2.如果不指定，则默认为：数据文件路径/bakfile
  --maxpool             --     str       1.指定线程池内线程数量。 2.如果不指定，则默认为0。
  --svcname             -p     str       1.指定本地服务端口。 2.如果不指定则默认为11810端口用于编目节点，11800用于协调节点，11820用于数据节点。
  --replname            -r     str       1.指定数据同步平面端口。 2.如果不指定则默认为svcname+1。
  --shardname           -a     str       1.指定shard平面端口。 2.如果不指定则默认为svcname+2。
  --catalogname         -x     str       1.指定catalog平面端口。 2.如果不指定则默认为svcname+3。
  --httpname            -s     str       1.指定http端口。 2.如果不指定则默认为svcname+4。
  --diaglevel           -v     num       1.指定诊断日志打印级别。SequoiaDB中诊断日志从0-5分别代表：SEVERE, ERROR, EVENT, WARNING, INFO, DEBUG。 2.如果不指定，则默认为WARNING。
  --role                -o     str       1.指定服务角色。SequoiaDB分别以data/coord/catalog/standalone代表：数据节点/协调节点/编目节点/单机。 2.如果不指定则默认为单机。
  --catalogaddr         -t     str       1.指定编目节点的地址。配置形式为"hostname1:catalogname1,hostname2:catalogname2,..."。 2.需要至少指定一个编目节点的地址。
  --logfilesz           -f     num       1.指定同步日志文件的大小。合法输入为64（MB）- 2048（MB）。 2.如果不指定，则默认为64（MB）。
  --logfilenum          -n     num       1.指定同步日志文件的数量。 2.如果不指定，则默认为20。
  --transactionon       -e     boolean   1.指定是否打开事务。2.如果不指定，则默认为false。
  --numpreload          --     num       页面预加载代理数据，默认值为0，取值范围：[0,100]
  --maxprefpool         --     num       数据预取代理池最大数量,默认值:0,取值范围:[0,1000]
  --maxreplsync         --     num       日志同步最大并发数量，默认值:10,取值范围:[0,200], 0表示不启用日志并发同步
  --logbuffsize         --     num       复制日志内存页面数,默认值:1024,取值范围:[512,1024000],但日志总内存大小不能超过日志总文件大小;每个页面大小为64KB
  --tmppath             --     num       数据库临时文件目录，默认为'数据库路径'+'/tmp'
  --sortbuf             --     num       排序缓存大小(MB),默认值256,最小值128
  --hjbuf               --     num       哈希连接缓存大小(MB),默认值128,最小值64
  --syncstrategy        --     str       副本组之间数据同步控制策略,取值:none,keepnormal,keepall,默认为keepnormal。
  --preferedinstance    --     str       1.指定执行读请求时优先选择的实例 2.如果不指定，则默认为随机选择任意实例。 3.取值列表： M--可读写实例 S--只读实例 A--任意实例 1-7--第n个实例
  --numpagecleaners     --     num       数据库启动时需要开启的脏页清除器数量 0意味着不启动任何脏页清除器，默认为1，取值范围：[0, 50]。
  --pagecleaninterval   --     num       对每个集合空间的进行脏页清除的最小时间间隔 单位：毫秒，默认：10000，最小：1000
  --lobpath             --     str       1.指定大对象存放目录。 2.如果不指定，则默认为：数据文件路径
  --directioinlob       --     boolean   在大对象功能中关闭文件系统缓存，如果不指定，默认值为"false"
  --sparsefile          --     boolean   当扩展文件时，使用稀疏文件功能，如果不指定，默认值为"false"
  --weight              --     num       节点选举权重，默认值为10，取值范围[1, 100]
  --usessl              --     boolean   允许客户端使用 SSL 连接（仅限企业版），默认为 false
  --auth                --     boolean   开启鉴权功能，默认为 true
  --planbuckets         --     num       访问计划缓存内桶的个数。当其为零时Sdb将不会缓存任何访问计划
  --arbiter             --     boolean   将节点设置成为一个仲裁节点。默认为false。
  --transactiontimeout  --     num       事务锁等待超时时间（单位：秒）,默认为:60,取值范围[0,3600]

**Note:**

SequoiaDB支持命令行方式及配置文件方式。当两种方式并存时，命令行参数将会覆盖配置文件中的相同的配置项。

同步日志的总大小（logfilesz * logfilenum）决定了在同步过程中的容错能力。日志越大则进行全量恢复的可能性越小。
##概念##

引擎调度单元（Engine Dispatchable Unit）是 SequoiaDB 数据库中任务运行的载体，一般来说一个 EDU 意味着一个单独的线程。

每个 EDU 可以用来执行用户的请求，或者执行系统内部的维护任务。

EDU 之间相互独立，不同 EDU 单独负责不同的用户会话。一个用户会话与一个 EDU，在一个数据节点中相互绑定。

每个 EDU 拥有一个进程内唯一的64位整数标示，称作“EDU ID”。

EDU 可以分为用户 EDU 与系统 EDU，分别代表执行用户任务的线程，与执行系统任务的线程。

##用户 EDU##

用户 EDU 为执行用户任务的线程，一般又叫作代理（Agent）线程。

在 SequoiaDB 中，主要存在下列代理线程类型：

  名称         类型        描述
  ------------ ----------- ----------------------------------------------------------------------------------------------------
  Agent        代理        代理线程负责由 svcname 服务传入的请求，一般来说该请求由用户直接传入
  ShardAgent   分区代理    分区代理线程负责由 shardname 服务传入的请求，一般来说该请求由协调节点传入数据节点
  CoordAgent   协调代理    协调代理线程负责由svcname服务传入的请求，一般来说该请求由用户直接传入，仅作用于协调节点
  ReplAgent    复制代理    复制代理线程负责由 replname 服务传入的请求，一般来说该请求由数据主节点传向从节点，多作用于非协调节点
  HTTPAgent    HTTP 代理   HTTP 代理线程负责由 httpname 服务传入的 REST 请求，一般来说该请求由用户直接传入

##系统 EDU##

系统 EDU 为系统内部维护数据结构及一致性的线程，一般来说对用户完全透明。

在 SequoiaDB 中，存在但不局限于下列系统 EDU：

  名称              类型               描述
  ----------------- ------------------ --------------------------------------------------------------------------------------------------------
  TCPListener       服务监听           该线程负责监听 svcname 服务，并启动 Agent 代理线程
  HTTPListener      HTTP 监听          该线程负责监听 httpname 服务，并启动 Agent 代理线程
  Cluster           集群管理           集群管理线程用于维护集群的基本框架，启动 ReplReader 与 ShardReader 线程
  ReplReader        复制监听           复制监听线程负责由 replname 服务传入的请求，并启动 ReplAgent 代理线程
  ShardReader       分区监听           分区监听线程负责由 shardname 服务传入的请求，并启动 ShardAgent 代理线程
  LogWriter         日志写             日志写线程用于将日志缓冲区中的数据写入日志文件
  WindowsListener   Windows 事件监听   Windows 环境特有，用于监听 Windows 中 SequoiaDB 定义事件
  Task              后台任务处理       后台任务处理线程，一般来说用于处理后台任务请求，例如[数据切分](SdbDoc_Cn/basic_operation/sharding/data_split.html)
  CatalogMC         编目主控           编目主控线程用于接收和分发编目节点接收到的请求
  CatalogNM         编目节点控制       编目节点控制线程用于处理编目节点内部集群信息相关的请求
  CatalogManager    编目控制           编目控制线程用于处理编目节点内部元数据相关的请求
  CatalogNetwork    编目网络监听       编目网络监听线程用于监听编目服务 catalogname 下的请求
  CoordNetwork      协调网络监听       协调网络监听线程用于监听分区的请求

##监控##

用户可以使用[会话快照](SdbDoc_Cn/database_management/monitoring/snapshot.html)监控系统与用户 EDU。
##日志文件##

SequoiaDB 采用日志方式进行副本间的数据同步。日志文件存在于 replicalog 目录中。文件大小和个数可以分别通过 logfilesz 和 logfilenum 参数进行设置。默认分别为64M（不包含头大小）和20。参数生效后无法修改。（如果要修改必须离线删除全部日志文件，重新配置参数并启动 SequoiaDB。但此举通常会引起全量同步。）

##同步##

数据组内所有备节点会定期将其他数据节点日志打包下载到本地进行日志回放。同步源并不限于主节点。因为我们期望所有节点的数据版本差距在一个很小的窗口内。当处于这个窗口内时，所有备节点向主节点同步数据。但是当某些节点的数据版本与主节点相差过大时，则选择其他备节点进行同步。当发生版本冲突时，以当前主节点数据版本为准。如果冲突不能解决则进入全量同步。当组内不存在主节点时，同步无法进行。

##全量同步##

触发全量同步的原因有：

	1.宕机重启。

	2.节点数据版本与其他节点相差过大。

	3.数据不一致并且无法修复。

**Note: **

正常重启后，如果数据版本仍在可同步范围内则不会触发全量同步。

发生全量同步的节点会清空本地所有数据及日志，同时将组内另一个节点（不限于主节点）的数据全部复制到本地。期间同步源发生的数据改变同样会被复制到本地。全量同步期间本节点对外不提供服务。当组内不存在主节点时，全量同步无法进行。全量同步会极大地影响整个组的性能，甚至导致其他备节点同步性能降低。建议通过增加分区及日志容量来避免全量同步。
##数据迁移 — 导入##

sdbimprt是SequoiaDB的数据导入工具，它可以将JSON格式或CSV格式的数据导入到SequoiaDB数据库中。

###JSON###

JSON格式的记录必须符合JSON的定义，以左右花括号作为记录的分界符，并且字符串类型的数据必须包含在两个双引号之间，转义字符为反斜杠'\\'。

###CSV###

CSV(Comma Separated Value)格式以逗号分隔数值。默认情况下记录以换行符分隔，字段以逗号分隔。用户能够指定字符串分隔符、字段分隔符以及记录分隔符。

###分隔符###

 | 类型 | 默认值 | 
 | --- | ------ | 
 | 字符串分隔符 | " | 
 | 字段分隔符 | , | 
 | 记录分隔符 | 换行符('\\n') | 

注意：

- 分隔符可以使用ASCII码中的不可见字符，通过转义字符'\\'输入ASCII码的十进制数值（0~127），例如'\\30'。回车符、换行符、制表符、转义字符'\\'可以直接使用'\\r'，'\\n'，'\\t'，'\\\\'。
- 可以使用UTF-8字符作为分隔符。
- 可以使用多个字符作为分隔符。

###CSV类型###

 | 类型 | 别名 | 说明 | 
 | --- | ---- | --- | 
 | int | integer | 十进制整型，取值范围为-2147483648~2147483647 | 
 | long | - | 十进制长整型，取值范围-9223372036854775808~9223372036854775807 | 
 | double | - | 双精度浮点型，取值范围为-1.7E+308~1.7E+308 | 
 | number | - | 数值类型，自动判断数值的具体类型(int, long, double) | 
 | bool | boolean | 布尔型，取值为true/false/t/f/yes/no/y/n，不区分大小写 | 
 | string | - | 字符串 | 
 | null | - | 空值 | 
 | oid | - | OID类型，长度必须为24个字符，不支持类型自动判断 | 
 | date | - | 日期类型，取值范围为1900-01-01~9999-12-31，不支持类型自动判断 |
 | autodate | - | 日期类型，取值范围为1900-01-01~9999-12-31，不支持类型自动判断 |
 | timestamp | - | 时间戳类型，取值范围为1902-01-01-00.00.00.000000~2037-12-31-23.59.59.999999，不支持类型自动判断 | 
 | autotimestamp | - | 时间戳类型，取值范围为1902-01-01-00.00.00.000000~2037-12-31-23.59.59.999999，不支持类型自动判断 | 
 | binary | - | 二进制类型，使用base64编码，不支持类型自动判断 | 
 | regex | - | 正则表达式类型，不支持类型自动判断 | 
 | skip | - | 忽略指定的列，该列的数据不导入到数据库 |

注意：

- int、long、double支持以‘#’开头的数字，例如#123.456。
- double支持科学计数法，例如1.23e-4，-1.23E+4。
- double支持忽略小数点前的‘0’，例如.123。
- autodate类型支持使用整数，表示自1970-01-01-00.00.00.000000以来的秒数, 取值范围为-377705145943~253402271999。
- autotimestamp类型支持使用整数，表示自1970-01-01-00.00.00.000000以来的毫秒数，取值范围为-2147414400000~2147443199000。

###CSV类型自动判断

在不指定CSV字段类型时，导入工具会自动判断类型。其中oid、date、timestamp、binary、regex不支持自动类型判断，会被识别为string类型。超出long类型范围的数值会作为double类型处理。

例如：

 | CSV数据 | 判断类型 | 实际数据 | 
 | ------ | ------- | ------- | 
 | 123 | int | 123 | 
 | 123. | int | 123 | 
 | +123 | int | 123 | 
 | -123 | int | -123 | 
 | 0123 | int | 123 | 
 | #-123. | int | -123 | 
 | 2147483648 | long | 2147483648 | 
 | 123.1 | double | 123.1 | 
 | .123 | double | 0.123 | 
 | 9223372036854775808 | double | 9.223372036854776e+18 | 
 | true | bool | true | 
 | false | bool | false | 
 | "123" | string | "123" | 
 | 123a | string | "123a" | 
 | "true" | string | "true" | 
 | "false" | string | "false" | 
 | "null" | string | "null" | 
 | null | null | null | 

###CSV类型转换

在指定CSV字段类型时，导入工具会将字段转换为指定的类型。如果字段的实际类型不是指定的类型，则转换可能失败。具体参考下表，最左边一列是指定的类型，Y表示可以转换，N表示不能转换。

 | 指定类型\\实际类型 | int | long | double | bool | string | null | oid | date | timestamp | binary | regex | 
 | - | --- | ---- | ------ | ------ | ---- | ------ | ---- | --- | ---- | --------- | ------ | ----- | 
 | int | Y | 可能溢出 | 可能丢失精度 | Y | 支持数值字符串 | N | N | N | N | N | N | 
 | long | Y | Y | 可能丢失精度 | Y | 支持数值字符串 | N | N | N | N | N | N | 
 | double | Y | Y | Y | N | 支持数值字符串 | N | N | N | N | N | N | 
 | number | Y | Y | Y | Y | 支持数值字符串 | N | N | N | N | N | N | 
 | bool | Y | Y | N | Y | 支持bool字符串 | N | N | N | N | N | N | 
 | string | Y | Y | Y | Y | Y | Y | Y | Y | Y | Y | Y | 
 | null | Y | Y | Y | Y | Y | Y | Y | Y | Y | Y | Y | 
 | oid | N | N | N | N | 支持OID字符串 | N | Y | N | N | N | N | 
 | date | Y | Y | N | N | 支持date字符串 | N | N | Y | Y | N | N | 
 | timestamp | Y | Y | N | N | 支持timestamp字符串 | N | N | Y | Y | N | N | 
 | binary | N | N | N | N | 支持binary字符串 | N | N | N | N | Y | N | 
 | regex | N | N | N | N | 支持regex字符串 | N | N | N | N | N | Y | 

注意：

- 指定类型为bool，实际类型为int、long时，0值转为false，非0值转为true。
- 指定类型为int、long，实际类型为bool时，true/t/yes/y转为1，false/f/no/n转为0。
- 参数--cast可以指定数值转换时是否允许精度损失或数值溢出。

###命令选项

**通用选项**

 | 选项 | 缩写 | 说明 | 
 | --- | ---- | --- | 
 | --help | -h | 显示帮助信息 | 
 | --version | -V | 显示版本号 | 
 | --hosts | - | 指定主机地址(hostname:svcname)，用','分隔多个地址，默认为'localhost:11810' | 
 | --user | -u | 数据库用户名 | 
 | --password | -w | 数据库用户名对应的密码 | 
 | --csname | -c | 集合空间的名字 | 
 | --clname | -l | 集合的名字 | 
 | --errorstop | - | 如果遇到解析错误就停止，默认为false | 
 | --ssl | - | 使用SSL连接，默认为false | 
 | --verbose | -v | 显示详细的执行信息 | 

**输入选项**

 | 选项 | 缩写 | 说明 | 
 | --- | ---- | --- | 
 | --file | - | 要导入的数据文件的名称，使用','分隔多个文件或目录。如果--file和--exec都没有指定，则从标准输入读取数据 | 
 | --exec | - | 执行外部程序来获取数据，外部程序必须将数据输出到标准输出 | 
 | --type | - | 导入数据格式，可以是csv或json，默认为csv | 
 | --linepriority | - | 当前分隔符默认的优先级为：记录分隔符，字符串分隔符，字段分隔符，默认值是true；如果设置为 false，那么分隔符的优先级为：字符串分隔符，记录分隔符，字段分隔符 | 
 | --delrecord | -r | 指定记录分隔符，默认是换行符'\\n' | 
 | --force | - | 如果数据中有非UTF-8的字符，强制导入数据，默认为false | 

注意:

- linepriority参数需要被特别关注，如果设置不当，可能会导入数据失败。当记录中包含“记录分隔符”并且 linepriority为 true 时，工具会优先按照“记录分隔符”解析，而导致导入失败。比如：如果记录为 {"name": "Mike\\n"}，应当设置 linepriority为false。
- 使用file参数指定文件或目录时，重复出现的文件会被忽略。
- 使用hosts指定地址时，重复出现的地址会被忽略。

**CSV选项**

 | 选项 | 缩写 | 说明 | 
 | --- | ---- | --- | 
 | --delchar | -a | 指定字符串分隔符，默认是双引号'"' | 
 | --delfield | -e | 指定字段分隔符,默认是逗号',' | 
 | --fields | - | 指定导入数据的字段名、类型、默认值 | 
 | --datefmt | - | 指定日期格式，默认为YYYY-MM-DD | 
 | --timestampfmt | - | 指定时间戳格式，默认为YYYY-MM-DD-HH.mm.ss.ffffff | 
 | --trim | - | 删除字符串左右两侧的空格，取值可以是[no|right|left|both]， 默认为no | 
 | --headerline | - | 指定导入数据首行是否作为字段名，默认为false | 
 | --sparse | - | 指定导入数据时是否自动添加字段名，默认为true。字段名按"filed1"、"field2"顺序增加 | 
 | --extra | - | 指定导入数据时是否自动添加值，默认为false | 
 | --cast | - | 指定是否允许数值类型转换时丢失精度或数值溢出，默认为false | 

注意：

- fields语法：fieldName [type [default &lt;value>], ...]
	- type支持所有的CSV类型
	- type可不写，由导入工具自动判断
	- 指定字段可以用命令行指定，也可以在导入文件的首行指定。如果在命令行指定了--fields，并且--headerline设为true，导入工具将会优先使用命令行指定字段并且跳过导入文件的首行
	- 字段名不能以'$'开头，中间不能有'.'，不能有不可见字符，包含空格时需要将字段名用单引号或双引号引起来
	- 例如：--fields='name string default "Jack", age int default 18, phone'
- datefmt格式包括年、月、日、通配符以及特定字符
	- 年：YYYY
	- 月：MM
	- 日：DD
	- 通配符：*
	- 特定字符：任意UTF-8字符
	- 其中年、月、日必须是整数，并且符合日期类型的范围
	- 指定通配符时，日期字段上对应的位置可以为任意字符
	- 指定特定字符时，日期字段上对应的位置必须为该指定字符
	- 例如：--datefmt="MM/DD, YYYY"，字段"3/15, 2015"与该格式匹配
- timestamp格式包括年、月、日、时、分、秒、微秒或毫秒、通配符以及特定字符
	- 年：YYYY
	- 月：MM
	- 日：DD
	- 时：HH
	- 分：mm
	- 秒：ss
	- 微秒：ffffff
	- 毫秒：SSS
	- 通配符：*
	- 特定字符：任意UTF-8字符
	- 其中年、月、日、时、分、秒、微秒、毫秒必须是整数，并且符合时间戳类型的范围
	- 微秒和毫秒不能同时出现，只能出现其中一个
	- 指定通配符时，时间戳字段上对应的位置可以为任意字符
	- 指定特定字符时，时间戳字段上对应的位置必须为该指定字符
	- 例如：--timestampfmt="MM/DD/YYYY T mm.ss.SSS"，字段"3/15/2015 T 12.30.123"与该格式匹配

**导入选项**

 | 选项 | 缩写 | 说明 | 
 | --- | ---- | --- | 
 | --insertnum | -n | 指定每次导入的记录数，取值范围为1~100000，默认为100 | 
 | --jobs | -j | 指定导入连接数（每个连接一个线程），取值范围为1~1000，默认为1 | 
 | --coord | - | 指定是否自动查找协调节点，默认为true | 
 | --sharding | - | 指定是否按分区信息重新打包记录，默认为true | 
 | --transaction | - | 指定导入数据时是否开启事务，默认为false。注意此功能需要服务端开启事务。| 
 | --allowkeydup | - | 指定是否允许唯一索引的键出现重复时忽略错误继续导入，默认为true |

###示例

1. 将数据导入到本地数据库11810中集合空间foo的集合bar，导入格式是csv，数据文件为test.csv，第一行为字段定义

<pre class="prettyprint lang-javascript">
$ sdbimprt --hosts=localhost:11810 --type=csv --file=test.csv -c foo -l bar --headerline=true</pre>

2. 导入格式是csv，文件名是test.csv，导入至集合空间foo的集合bar中

以下是导入文件的内容：

<pre class="prettyprint lang-diy">
"Jack",18,"China"
"Mike",20,"USA"</pre>

导入命令：

<pre class="prettyprint lang-javascript">
$ sdbimprt --hosts=localhost:11810 --type=csv --file=test.csv -c foo -l bar --fields='name string default "Anonymous", age int, country'</pre>

3. 导入格式是csv，文件名是test.csv，导入至集合空间foo的集合bar中

以下是导入文件的内容：

<pre class="prettyprint lang-diy">
name, age, country
"Jack",18,"China"
"Mike",20,"USA"</pre>

文件第一行是字段定义，需要跳过。
导入命令：

<pre class="prettyprint lang-javascript">
$ sdbimprt --hosts=localhost:11810 --type=csv --file=test.csv -c foo -l bar --fields='name string default "Anonymous", age int, country' --headerline=true</pre>

4. 导入格式是csv，导入文件是目录../data中的所有文件，导入至集合空间foo的集合bar中

<pre class="prettyprint lang-javascript">
$ sdbimprt --hosts=localhost:11810 --type=csv --file=../data -c foo -l bar</pre>

5. 导入格式是csv，导入文件是目录../data中的所有文件以及./foo_bar_data.csv，导入至集合空间foo的集合bar中，有11810和11910两个协调节点，记录中时间戳类型的数据类似于"2015-10-01 T 12.31.15.123 T"，使用两个连接同时导入

<pre class="prettyprint lang-javascript">
$ sdbimprt --type=csv --file=../data,./foo_bar_data.csv -c foo -l bar --timestampfmt="YYYY-MM-DD T HH.mm.ss.SSS T" --hosts=localhost:11810,localhost:11910 -j 2</pre>

6. 导入格式是json，通过管道从其它工具other获取数据，导入至集合空间foo的集合bar中，第一行为字段定义

<pre class="prettyprint lang-javascript">
$ other | sdbimprt --hosts=localhost:11810 --type=json -c foo -l bar --headerline=true</pre>

##数据迁移 — 导出##
sdbexprt 是一个实用的工具。它可以从 SequoiaDB 数据库导出一个 JSON 格式或者 CSV 格式的数据存储文件。

###选项###

  参数          缩写   描述
  ------------- ------ -------------------------------------------------------------------------------
  --help        -h     返回基本帮助和用法文本。
  --version            返回版本信息。
  --hostname    -s     从指定主机名的 SequoiaDB 中导出数据。默认情况下 sdbexprt 尝试连接到本地主机。
  --svcname     -p     指定的端口号。默认情况下 sdbexprt 尝试连接到端口号11810的主机。
  --user        -u     数据库用户名。
  --password    -w     数据库密码。
  --delchar     -a     指定字符分隔符。默认是（"），csv 格式有效。
  --delfield    -e     指定字段分隔符。默认是（,），csv 格式有效。
  --delrecord   -r     指定记录分隔符。默认是（\\n）。
  --csname      -c     指定导出数据的集合空间名。
  --clname      -l     指定导出数的集合名。
  --fields             指定一个或多个字段来导出数据，使用逗号分隔多个字段。csv 格式有效。
  --included           指定是否导出字段名到 csv 首行，默认 true，csv 格式有效。
  --file               指定要导出的文件名。
  --type               指定的导出数据格式。默认 csv，数据格式可以是 csv 或 json。
  --errorstop          如果遇到错误就停止，默认 false。
  --ssl                使用 SSL 连接，默认 false。

###返回值###

-   0：成功

-   1：成功但有警告

-   2：失败

-   127：参数错误

###用法###

在下面的例子，sdbexprt 从本地数据库端口11810中导出集合空间 foo 的集合 bar 的数据，导出类型是 csv，导出文件为 contact，导出字段是 field1 和 field2。

<pre class="prettyprint lang-javascript">
$ sdbexprt -s localhost -p 11810 --type csv --file contace --fields field1,field2 -c foo -l bar</pre>
sdbdmsdump（1.8 版本前名为 sdbinspt，1.8 版本后更名为 sdbdmsdump）是一个 SequoiaDB 数据库的数据文件检测工具。它可以检查数据库文件结构的正确性，并且给出结果报告。

##权限需求##

运行 sdbdmsdump 命令的用户必须对数据库的数据与索引文件拥有读权限。

##连接需求##

sdbdmsdump 不需要与数据库连接。

##选项##
+-------------+------+-----------------------------------------------------------------------------------+
| 参数        | 参数 | 描述                                                                              |
+=============+======+===================================================================================+
| --help      | -h   | 返回基本帮助和用法文本                                                            |
+-------------+------+-----------------------------------------------------------------------------------+
| --dbpath    | -d   | 指定数据库文件所在目录，默认为当前目录                                            |
+-------------+------+-----------------------------------------------------------------------------------+
| --output    | -o   | 指定输出文件，默认为屏幕输出                                                      |
+-------------+------+-----------------------------------------------------------------------------------+
| --verbose   | -v   | 是否进行 ASCII 文本输出（true/false），默认为 true                                |
+-------------+------+-----------------------------------------------------------------------------------+
| --csname    | -c   | 指定集合空间名，如果未指定则为全部集合空间                                        |
+-------------+------+-----------------------------------------------------------------------------------+
| --clname    | -l   | 指定集合名，如果未指定则为全部集合                                                |
+-------------+------+-----------------------------------------------------------------------------------+
| --action    | -a   | 指定操作，为（inspect/dump/all）之一，**必须指定**                                |
|             |      |                                                                                   |
|             |      | -    inspect：检测并报告任何数据损坏                                              |
|             |      | -    dump：将数据页格式化并输出                                                   |
|             |      | -    all：检测数据页损坏，并格式化输出数据页                                      |
+-------------+------+-----------------------------------------------------------------------------------+
| --dumpdata  | -t   | 设定操作数据文件（ture/false），默认为 false                                      |
+-------------+------+-----------------------------------------------------------------------------------+
| --dumpindex | -i   | 设定操作索引文件（true/false），默认为 false                                      |
+-------------+------+-----------------------------------------------------------------------------------+
| --pagestart | -s   | 指定起始数据页，默认为-1                                                          |
+-------------+------+-----------------------------------------------------------------------------------+
| --numpage   | -n   | 指定需要检测或格式化的数据页数量，当指定 -s 参数为非负值时，该参数生效。默认值为1 |
+-------------+------+-----------------------------------------------------------------------------------+
| --record    | -p   | 指定显示格式化输出数据或索引内容（true/false），默认为 false                      |
+-------------+------+-----------------------------------------------------------------------------------+

##用法##

使用 sdbdmsdump 工具时，请务必保证数据库进程已经停止。

在下面的例子，sdbdmsdump 在当前目录下检测并格式化输出所有集合空间与集合的数据与索引至 output.txt 文件。

<pre class="prettyprint lang-javascript">
> sdbdmsdump -d . -o output.txt -v true -a all -t true -i true -p true</pre>
sdbtop 是一个 SequoiaDB 数据库的性能监控工具。通过它，可以监控和查看集群中各个节点的监视信息。

##选项##

参数            缩写   描述
--------------- ------ ---------------------------------------------------------------------------------
--help          -h     返回基本帮助和用法文本
--confpath      -c     sdbtop 的配置文件，sdbtop 界面形态以及输出字段都依赖该文件（缺省使用默认配置文件）
--hostname      -i     指定需要监控的主机名
--servicename   -s     指定监控的端口服务名
--usrname       -u     数据库用户名
--password      -p     数据库密码
--ssl                  使用 SSL 连接。

**Note: **

对于 Ubuntu 等系统，需要安装 Ncurses 库，否则将会提示“Error opening terminal: TERM”

方式一： 联网安装

<pre class="prettyprint lang-javascript">
$ sudo apt-get install libncurses5-dev</pre>

方式二： 源码安装

解压 tar -xvzf ncurses-5.5.tar.gz，进入 ncurses-5.5 目录

<pre class="prettyprint lang-javascript">
$ ./configure
$ sudo make && make install</pre>

##用法##

在下面的例子，sdbtop 使用配置文件为“/opt/sequoiadb/conf/sample/sdbtop.xml”，监控主机名为 sdbserver3，端口服务名为11810，用户名为 test，密码为 test 的数据库集群中的一个节点。

<pre class="prettyprint lang-javascript">
$ sdbtop -c /opt/sequoiadb/conf/sample/sdbtop.xml -i sdbserver3 -s 11810 -u test -p test</pre>

接着进入主窗口：

<pre class="prettyprint lang-diy">
refresh= 3 secs           sdbtop 1.0       snapshotMode: GLOBAL
displayMode: ABSOLUTE     Main Window      snapshotModeInput: NULL
hostname: sdbserver3                       filtering Number: 0
servicename: 11810                         sortingWay: NULL sortingField: NULL
usrName: test                              Refresh: F5, Quit: q, Help: h

 #####  ######  ######  #######  #####  ######   For help type h or ...
#       #     # #     #    #    #     # #     #  sdbtop -h: usage
#       #     # #     #    #    #     # #     #
 #####  #     # ######     #    #     # ######
      # #     # #     #    #    #     # #
      # #     # #     #    #    #     # #
 #####  ######  ######     #     #####  #

SDB Interactive Snapshot Monitor V2.0
Use these keys to navigate:
  m   -  Main Window            s   -  Sessions               c   -  CollectionSpaces
  t   -  System                 d   -  Database               G   -  GLOBAL_SNAPSHOT
  g   -  GROUP_SNAPSHOT         n   -  NODE_SNAPSHOT          r   -  reset refreshInterval
  A   -  Ascending order        D   -  Descending order       C   -  filter condition
  Q   -  no filter condition    N   -  filter number          W   -  no filter number
Licensed Materials - Property of SequoiaDB
Copyright SequoiaDB Corp. 2013-2014 All Rights Reserved.</pre>

**Note:** 在主窗口中按‘**h**’键可以查看所有工具支持的按键

-   主窗口按键说明

参数    描述
------- ---------------------------------------------------
m       返回主窗口
s       列出数据库节点上的所有会话
c       列出数据库节点上的所有集合空间
t       列出数据库节点上的系统资源使用情况
d       列出数据库节点的数据库监视信息
G       global_snapshot，监控所有的数据节点组
g       group_snapshot，指定监控某个数据节点组
n       node_snapshot，列出指定的数据库节点的监视信息
r       设置刷屏的时间间隔，单位秒/s
A       将监视信息按照某列进行顺序排序
D       将监视信息按照某列进行逆序排序
C       将监视信息按照某个条件进行筛选
Q       返回没有使用条件进行筛选前的监视信息
N       将监视信息中对应行号的记录过滤不显示
W       返回没有使用行号进行过滤前的监视信息
h       查看使用帮助
Esc     取消已进入的操作
Enter   返回上一次监视界面，（在已进入 help 帮助输出中有效）
F5      强制刷新后台监视信息
<       向左移动，以查看隐藏的左边列的监视信息
\>      向右移动，以查看隐藏的右边列的监视信息
q       退出程序
Tab     切换数据计算的模式（绝对值，平均值，差值三个模式）

**例如：**

1.进入主窗口后，按‘s’键，列出数据库节点的所有会话信息

<pre class="prettyprint lang-diy">
refresh= 3 secs              sdbtop 1.0         snapshotMode: GLOBAL
displayMode: ABSOLUTE        Sessions           snapshotModeInput: NULL
hostname: sdbserver3                            filtering Number: 0
servicename: 11810                              sortingWay: NULL sortingField: NULL
usrName: test                                   Refresh: F5, Quit: q, Help: h

SessionID                           TID Type               Name
&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;   &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
1  sdbserver3:11820:1                10732 LogWriter          ""
2  sdbserver3:11820:10               10741 Task               Job[Prefetcher]
3  sdbserver3:11820:11               10742 Task               Job[Prefetcher]
4  sdbserver3:11820:12               10743 Task               Job[Prefetcher]
5  sdbserver3:11820:13               10744 Cluster            ""
6  sdbserver3:11820:14               10745 ClusterShard       ""
7  sdbserver3:11820:15               10746 ClusterLogNotify   ""
8  sdbserver3:11820:16               10747 ShardReader        ""
9  sdbserver3:11820:17               10748 ReplReader         ""
10  sdbserver3:11820:18              10749 SyncClockWorker    ""
11  sdbserver3:11820:19              10750 TCPListener        ""
12  sdbserver3:11820:2               10733 DpsRollback        ""
13  sdbserver3:11820:20              10751 RestListener       ""
14  sdbserver3:11820:21              10752 Task               Job[PageCleaner]
15  sdbserver3:11820:3               10734 Task               Job[Prefetcher]
16  sdbserver3:11820:4               10735 Task               Job[Prefetcher]
17  sdbserver3:11820:42              10847 ReplAgent          NodeID:1000,TID:1,Start:active
18  sdbserver3:11820:5               10736 Task               Job[Prefetcher]
19  sdbserver3:11820:59              23263 ShardAgent         NetID:1,TID:23262
20  sdbserver3:11820:6               10737 Task               Job[Prefetcher]
21  sdbserver3:11820:7               10738 Task               Job[Prefetcher]
22  sdbserver3:11820:8               10739 Task               Job[Prefetcher]</pre>

2.按‘Tab’键，可以看到屏幕左上方的‘displayMode’的值会发生切换

3.按‘r’键，在屏幕最下方输入‘2’，回车，设置刷新间隔时间，可以看到屏幕左上方的‘refresh’的值变为 2

<pre class="prettyprint lang-diy">
refresh= 2 secs              sdbtop 1.0         snapshotMode: GLOBAL
displayMode: ABSOLUTE        Sessions           snapshotModeInput: NULL
hostname: sdbserver3                            filtering Number: 0
servicename: 11810                              sortingWay: NULL sortingField: NULL
usrName: test                                   Refresh: F5, Quit: q, Help: h

SessionID                           TID Type               Name
&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;   &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
1  sdbserver3:11820:1                10732 LogWriter          ""
2  sdbserver3:11820:10               10741 Task               Job[Prefetcher]
3  sdbserver3:11820:11               10742 Task               Job[Prefetcher]
4  sdbserver3:11820:12               10743 Task               Job[Prefetcher]
5  sdbserver3:11820:13               10744 Cluster            ""
6  sdbserver3:11820:14               10745 ClusterShard       ""
7  sdbserver3:11820:15               10746 ClusterLogNotify   ""
8  sdbserver3:11820:16               10747 ShardReader        ""
9  sdbserver3:11820:17               10748 ReplReader         ""
10  sdbserver3:11820:18              10749 SyncClockWorker    ""
11  sdbserver3:11820:19              10750 TCPListener        ""
12  sdbserver3:11820:2               10733 DpsRollback        ""
13  sdbserver3:11820:20              10751 RestListener       ""
14  sdbserver3:11820:21              10752 Task               Job[PageCleaner]
15  sdbserver3:11820:3               10734 Task               Job[Prefetcher]
16  sdbserver3:11820:4               10735 Task               Job[Prefetcher]
17  sdbserver3:11820:42              10847 ReplAgent          NodeID:1000,TID:1,Start:active
18  sdbserver3:11820:5               10736 Task               Job[Prefetcher]
19  sdbserver3:11820:59              23263 ShardAgent         NetID:1,TID:23262
20  sdbserver3:11820:6               10737 Task               Job[Prefetcher]
21  sdbserver3:11820:7               10738 Task               Job[Prefetcher]
22  sdbserver3:11820:8               10739 Task               Job[Prefetcher]</pre>

4.按‘A’键，并输入‘TID’，列表结果按照 TID 进行顺序排序

<pre class="prettyprint lang-diy">
refresh= 2 secs              sdbtop 1.0         snapshotMode: GLOBAL
displayMode: ABSOLUTE        Sessions           snapshotModeInput: NULL
hostname: sdbserver3                            filtering Number: 0
servicename: 11810                              sortingWay: NULL sortingField: NULL
usrName: test                                   Refresh: F5, Quit: q, Help: h

SessionID                           TID Type               Name
&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;   &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
1  sdbserver3:11820:1                10732 LogWriter          ""
2  sdbserver3:11820:10               10741 Task               Job[Prefetcher]
3  sdbserver3:11820:11               10742 Task               Job[Prefetcher]
4  sdbserver3:11820:12               10743 Task               Job[Prefetcher]
5  sdbserver3:11820:13               10744 Cluster            ""
6  sdbserver3:11820:14               10745 ClusterShard       ""
7  sdbserver3:11820:15               10746 ClusterLogNotify   ""
8  sdbserver3:11820:16               10747 ShardReader        ""
9  sdbserver3:11820:17               10748 ReplReader         ""
10  sdbserver3:11820:18              10749 SyncClockWorker    ""
11  sdbserver3:11820:19              10750 TCPListener        ""
12  sdbserver3:11820:2               10733 DpsRollback        ""
13  sdbserver3:11820:20              10751 RestListener       ""
14  sdbserver3:11820:21              10752 Task               Job[PageCleaner]
15  sdbserver3:11820:3               10734 Task               Job[Prefetcher]
16  sdbserver3:11820:4               10735 Task               Job[Prefetcher]
17  sdbserver3:11820:42              10847 ReplAgent          NodeID:1000,TID:1,Start:active
18  sdbserver3:11820:5               10736 Task               Job[Prefetcher]
19  sdbserver3:11820:59              23263 ShardAgent         NetID:1,TID:23262
20  sdbserver3:11820:6               10737 Task               Job[Prefetcher]
21  sdbserver3:11820:7               10738 Task               Job[Prefetcher]
please input the displayName which need order by asc : TID</pre>

5.按‘N’键，并输入1，列表中将原来行号为1的记录过滤不显示

6.按‘W’键，返回没有按行号进行过滤前的列表信息

7.按‘C’键，并输入‘TID：10732”进行筛选，则只显示 TID 值为10732的记录

<pre class="prettyprint lang-diy">
refresh= 2 secs              sdbtop 1.0         snapshotMode: GLOBAL
displayMode: ABSOLUTE        Sessions           snapshotModeInput: NULL
hostname: sdbserver3                            filtering Number: 0
servicename: 11810                              sortingWay: NULL sortingField: NULL
usrName: test                                   Refresh: F5, Quit: q, Help: h

SessionID                           TID Type               Name
&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;   &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
1  sdbserver3:11820:1                10732 LogWriter          ""
2  sdbserver3:11820:10               10741 Task               Job[Prefetcher]
3  sdbserver3:11820:11               10742 Task               Job[Prefetcher]
4  sdbserver3:11820:12               10743 Task               Job[Prefetcher]
5  sdbserver3:11820:13               10744 Cluster            ""
6  sdbserver3:11820:14               10745 ClusterShard       ""
7  sdbserver3:11820:15               10746 ClusterLogNotify   ""
8  sdbserver3:11820:16               10747 ShardReader        ""
9  sdbserver3:11820:17               10748 ReplReader         ""
10  sdbserver3:11820:18              10749 SyncClockWorker    ""
11  sdbserver3:11820:19              10750 TCPListener        ""
12  sdbserver3:11820:2               10733 DpsRollback        ""
13  sdbserver3:11820:20              10751 RestListener       ""
14  sdbserver3:11820:21              10752 Task               Job[PageCleaner]
15  sdbserver3:11820:3               10734 Task               Job[Prefetcher]
16  sdbserver3:11820:4               10735 Task               Job[Prefetcher]
17  sdbserver3:11820:42              10847 ReplAgent          NodeID:1000,TID:1,Start:active
18  sdbserver3:11820:5               10736 Task               Job[Prefetcher]
19  sdbserver3:11820:59              23263 ShardAgent         NetID:1,TID:23262
20  sdbserver3:11820:6               10737 Task               Job[Prefetcher]
21  sdbserver3:11820:7               10738 Task               Job[Prefetcher]
please input the filter condition : TID:10732</pre>

8.按‘Q’键，返回没有按照筛选条件前的列表信息

9.按‘<’或者‘>’键，可以查看隐藏在左边或者右边的列
sdbinspect 是一个 SequoiaDB 数据库的数据节点间数据一致性检测工具。它可以检查节点间数据是否完全一致，并且给出结果报告。

##权限需求##

无

##连接需求##

sdbinsepct 需要与数据库（coord 节点）连接。

##选项##

  参数                缩写   描述
  ------------------- ------ ------------------------------------------------------------------------------------------------------------------------
  --help              -h     返回基本帮助和用法文本
  --version           -v     返回当前工具所附属的数据库的版本
  --action            -a     指定检查数据或对已经存在的中间文件生成 report，inspect 和 report 可选，默认是 inspect
  --coord             -d     指定 coord 节点的 hostname 和服务端口，格式为 hostname:servicename，必须指定
  --loop              -t     指定迭代检查的次数，默认是5（次）
  --group             -g     指定要检查的 group 的名字，若不指定，则检查所有的 group
  --collectionspace   -c     指定检查的集合空间名字，不指定则检查所有集合空间
  --collection        -l     指定检查的集合名字，不指定则检查所有集合，指定集合时，必须制定集合空间
  --file              -f     指定从已存在的（上一次检查的）结果文件开始检查，当指定此选择时，其它选项（除 -o 外）均失效，生效的为文件中保存的 command 选项
  --output            -o     指定输出的文件名，默认是 inspect.bin，报告文件为 inspect.bin.report
  --view              -w     指定生成 report 文件的内容按 group 查看和按 collection 查看，默认为 group

##用法##

在下面的例子，sdbinspect 检查协调节点 ubuntu-dev9:11810 下的全部集群（5次），并将中间文件结果输出到 item.bin 中，同时会解析 item.bin 文件，把文本结果按（默认的）group 划分，输出到 item.bin.report 文件中。

<pre class="prettyprint lang-javascript">
sdbinspect -d ubuntu-dev9:11810 –o item.bin</pre>

在下面的例子，sdbinspect 检查协调节点 ubuntu-dev9:11810 下的全部集群中的集合空间 sports（3次），并将中间文件结果输出到 item.bin 中，同时会解析 item.bin 文件，把文本结果按 collection 划分，输出到
item.bin.report 文件中。

<pre class="prettyprint lang-javascript">
sdbinspect -d ubuntu-dev9:11810 –o item.bin –c sports –w collection –t 3</pre>

在下面的例子，sdbinspect 检查协调节点 ubuntu-dev9:11810 下的 data_group 集群中的名为 sports 的集合空间，名为 item 的集合（5次），并将中间文件结果输出到 inspect.bin 中，同时会解析 inspect.bin 文件，把文本结果按（默认的）group 划分，输出到 inspect.bin.report 文件中。

<pre class="prettyprint lang-javascript">
sdbinspect -d ubuntu-dev9:11810 –g data_group –c sports –l item</pre>
sdbsupport 是用于收集 SequoiaDB 相关信息的工具，位于目录 /opt/sequoiadb/tools 下面。此工具收集的信息包括：数据库配置信息、数据库日志信息、数据库所在主机的硬件信息和数据库、操作系统信息以及数据库快照信息。

使用此工具需要先为 sdbsupport.sh 赋执行权限：

<pre class="prettyprint lang-javascript">
$ chmod 755 sdbsupport.sh</pre>

##权限需求##

数据库用户权限。

##选项##

  参数                缩写     描述
  ------------------- -------- ------------------------------------------
  --help                       帮助选项
  --hostname          -s       所需要收集的信息的主机名字
  --svcname           -p       指定特定端口收集其配置、日志及快照信息
  --user              -u       数据库用户名
  --password          -w       数据库用户密码
  --snapshot          -n       收集快照信息
  --osinfo            -o       收集操作系统信息
  --hardware          -h       收集硬件信息
  --all                        指定收集数据库所有信息
  --conf                       指定收集配置文件的信息
  --log                        指定收集日志文件信息
  --cm                         指定收集 CM 配置、日志信息
  --cpu                        指定收集 CPU 信息
  --memory                     指定收集内存信息
  --disk                       指定收集硬盘信息
  --netcard                    指定收集网卡信息
  --mainboard                  指定收集主板信息
  --catalog                    指定收集编目节点快照
  --group                      指定收集数据库集群组的信息
  --context                    指定收集上下文快照信息
  --session                    指定收集会话快照信息
  --collection                 指定收集集合快照信息
  --collectionspace            指定收集集合空间信息
  --database                   指定收集数据库快照信息
  --system                     指定收集系统快照信息
  --diskmanage                 指定收集操作系统硬盘管理信息
  --basicsys                   指定收集操作系统基本信息
  --module                     指定收集内核加载模块信息
  --env                        指定收集操作系统环境变量信息
  --network                    指定收集 IP 地址等网络信息
  --process                    指定收集操作系统进程信息
  --login                      指定收集用户登陆此机所进行操作的历史信息
  --limit                      指定收集操作系统用户限制信息
  --vmstat                     指定收集给定时间间隔内的服务状态值信息

##用法示例##

1.获取参数信息。

<pre class="prettyprint lang-javascript">
$ ./sdbsupport.sh --help</pre>

2.收集本机的数据信息。【包括配置、日志、硬件、操作系统及快照信息】

<pre class="prettyprint lang-javascript">
$ ./sdbsupport.sh</pre>

3.收集整个数据库集群信息

<pre class="prettyprint lang-javascript">
$ ./sdbsupport.sh --all</pre>

4.收集指定主机信息。

<pre class="prettyprint lang-javascript">
$ ./sdbsuport.sh -s htest2</pre>

5.收集指定主机指定端口信息。

<pre class="prettyprint lang-javascript">
$ ./sdbsuppor.sh -s htest3 -p 50000</pre>

6.收集操作系统信息。

<pre class="prettyprint lang-javascript">
$ ./sdbsupport.h --osinfo</pre>

7.收集特定主机特定端口的日志信息及快照信息。

<pre class="prettyprint lang-javascript">
$ ./sdbsupport.sh -s htest2 -p 11810 --snapshot --log</pre>

##信息归类##

通过执行 ./sdbsupport.sh xxx xxx….
收集的数据库信息信息，会全部收集到本地的 log
文件夹中。收集的信息是以主机为单位打成的压缩包，名称以“主机名-年月日-时分秒”命名。将此文件解压缩后会得四个文件夹：SDBNODES，SDBSNAPS，OSINFO，HARDINFO。

-   SDBNODES：存放收集的数据库配置、日志信息

-   SDBSNAPS：存放收集的数据库快照信息

-   OSINFO：存放收集的操作系统信息

-   HARDINFO：存放收集的硬件信息

**Note:**

数据库集群内的机器，如果没有配置信任关系，在收集时，需要输入密码，如：

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/tools/sdbsupport/expect/expect

Success to export System environment variable : /opt/sequoiadb/tools/sdbsupport/expect/
Check over Environment!
Complete database database cluster
The host sdbadmin@htest2's password :</pre>

【此时需要输入 htest2 机器，sdbadmin 用户的密码，然后 Enter】
sdblobtool 是一款用于管理集合大对象的工具。

##功能列表##

  功能   描述
  ------ ----------------------------------------
  导出   将集合中的大对象导出至本地文件
  导入   将本地文件中的大对象导入至集合
  迁移   将一个集合中的大对象复制到另一个集合中

##选项##

**导出选项**

  名称         描述                 默认值      是否必填
  ------------ -------------------- ----------- ------------
  hostname     Coord 地址           localhost   否
  svcname      Coord 服务名         11810       否
  usrname      用户名               -           否
  passwd       密码                 -           否
  operation    操作类型             -           是
  file         本地文件全路径       -           是
  collection   需要导出的集合全名   -           是
  prefer       优先选择的实例       A           否
  ssl          使用 SSL 连接。      false       否

**导入选项**

  名称         描述                                                              默认值      是否必填
  ------------ ----------------------------------------------------------------- ----------- ------------
  hostname     Coord 地址                                                        localhost   否
  svcname      Coord 服务名                                                      11810       否
  usrname      用户名                                                            -           否
  passwd       密码                                                              -           否
  operation    操作类型                                                          -           是
  file         本地文件全路径                                                    -           是
  collection   需要导出的集合全名                                                -           是
  ignorefe     当前大对象如果已经存在于集合中，忽略这个错误并开始导入下一条记录   false      否
  ssl          使用 SSL 连接。                                                   false       否

**Note:**

-   当需要开启 ignorefe 时只需要添加 --ignorefe 即可，不需要为其制定具体值。下同。
-   本地文件必须为导出生成的文件。

**迁移选项**

  名称            描述                                                              默认值      是否必填
  --------------- ----------------------------------------------------------------- ----------- ---------------
  hostname        Coord 地址                                                        localhost   否
  svcname         Coord 服务名                                                      11810       否
  usrname         用户名                                                            -           否
  passwd          密码                                                              -           否
  operation       操作类型                                                          -           是
  file            本地文件全路径                                                    -           是
  collection      需要导出的集合全名                                                -           是
  ignorefe        当前大对象如果已经存在于集合中，忽略这个错误并开始导入下一条记录   false      否
  dsthost         目标 Coord 地址                                                   localhost   否
  dstservice      目标服务名                                                        11810       否
  dstusrname      目标用户名                                                        -           否
  dstpasswd       目标密码                                                          -           否
  dstcollection   目标集合全名                                                      -           是
  ssl             使用 SSL 连接。                                                   false       否

##日志##

使用 sdblobtool 时会在用户的当前运行目录产生日志文件“sdblobtool.log”，当发生错误时可以在日志中查看详细信息。

##常见错误##

+--------------+-----------------------------------------------+--------------------------------------------------------------------------+
| 错误码（rc） | 描述                                          | 应对措施                                                                 |
+==============+===============================================+==========================================================================+
|              |                                               |                                                                          |
| -5           | 本地文件已存在或者集合中存在相同 OID 的大对象 | -    如果是导出操作，检查本地文件是否已经存在                            |
|              |                                               | -    如果是导入或者迁移操作，检查目标集合中是否已存在相同 OID 的大对象   |
+--------------+-----------------------------------------------+--------------------------------------------------------------------------+
| -6           | 参数不合法                                    | 检查参数是否输入错误                                                     |
+--------------+-----------------------------------------------+--------------------------------------------------------------------------+
| -10          | 系统错误                                      | 需要根据日志进行错误排查                                                 |
+--------------+-----------------------------------------------+--------------------------------------------------------------------------+
|              |                                               |                                                                          |
| -15          | 无法连接到指定地址                            | -    检查地址相关参数是否填写正确                                        |
|              |                                               | -    检查数据库是否正常启动                                              |
|              |                                               | -    如果使用主机名作为参数，检查本地主机名列表是否配置正确              |
|              |                                               | -    检查防火墙是否开启                                                  |
+--------------+-----------------------------------------------+--------------------------------------------------------------------------+
|              |                                               |                                                                          |
| -23          | 集合不存在                                    | -    检查集合相关参数是否填写正确                                        |
|              |                                               | -    检查相关集合是否存在                                                |
+--------------+-----------------------------------------------+--------------------------------------------------------------------------+

##示例##

将集合 foo.bar 中的大对象导出至本地文件 mylob 中。

<pre class="prettyprint lang-javascript">
$ ./sdblobtool --operation export --hostname 192.168.1.1 --svcname 11810 --collection foo.bar --file /opt/mylob</pre>

将本地文件 mylob 中的大对象导入至集合 foo.bar 中，当遇到已存在的大对象时直接跳过。

<pre class="prettyprint lang-javascript">
$ ./sdblobtool --operation import --hostname 192.168.1.1 --svcname 11810 --collection foo.bar --file /opt/mylob --ignorefe</pre>

将集合中的大对象复制到另一个集合中。

<pre class="prettyprint lang-javascript">
$ ./sdblobtool --operation migration --hostname 192.168.1.1 --svcname 11810 --collection foo.bar --dsthost 192.168.1.2 --dstservice 11810 --dstcollection foo.bar1</pre>
##操作系统重启动##

操作系统启动后会自动启动服务 sdbcm（sequoiadb cluster manager）。该服务启动后会自动启动该物理机中所有注册在 /opt/sequoiadb/conf/local 目录下的节点。使用命令 **ps –elf | grep sequoiadb** 能看到当前正在启动的节点与启动完毕的节点。启动完毕的进程名为：sequoiadb（服务名）正在启动的进程名一般为：

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sequoiadb –c /opt/sequoiadb/conf/local/（服务名）</pre>

##手工启动特定节点##

当集群中某个节点失效后，用户可以在 sdb 命令行使用如下步骤启动节点。假设 SequoiaDB 的安装路径为**/opt/sequoiadb**

1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost", 11810) ;</pre>

2.得到分区组

<pre class="prettyprint lang-javascript">
> dataRG = db.getRG ( "&lt;datagroup1&gt;" ) ;</pre>

3.得到数据节点

<pre class="prettyprint lang-javascript">
> dataNode = dataRG.getNode ( "&lt;hostname1&gt;", "&lt;servicename1&gt;" ) ;</pre>

4.启动节点

<pre class="prettyprint lang-javascript">
> dataNode.start() ;</pre>

##手工启动数据组##

当集群中某个数据组被停止后，用户可以在 sdb 命令行使用如下步骤启动数据组。该操作会启动数据组中全部数据节点。

1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost", 11810) ;</pre>

2.得到分区组

<pre class="prettyprint lang-javascript">
> dataRG = db.getRG ( "&lt;datagroup1&gt;" ) ;</pre>

3.启动数据组

<pre class="prettyprint lang-javascript">
> dataRG.start();</pre>
##手工停止特定节点##

用户可以在 sdb 命令行使用如下步骤停止数据节点。

1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost", 11810) ;</pre>

2.得到分区组

<pre class="prettyprint lang-javascript">
> dataRG = db.getRG ( "&lt;datagroup1&gt;" ) ;</pre>

3.得到数据节点

<pre class="prettyprint lang-javascript">
> dataNode = dataRG.getNode ( "&lt;hostname1&gt;", "&lt;servicename1&gt;" ) ;</pre>

4.停止节点

<pre class="prettyprint lang-javascript">
> dataNode.stop() ;</pre>

##手工停止数据组##

用户可以在 sdb 命令行使用如下步骤停止数据组。该操作会停止数据组中全部数据节点。

1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost", 11810) ;</pre>

2.得到分区组

<pre class="prettyprint lang-javascript">
> dataRG = db.getRG ( "&lt;datagroup1&gt;" ) ;</pre>

3.停止数据组

<pre class="prettyprint lang-javascript">
> dataRG.stop() ;</pre>

##使用kill命令停止数据节点##

用户可以使用 **kill -15 &lt;pid&gt;** 正常停止数据节点。以该方式停止的数据节点被认为正常停止。用户使用 **kill -9 &lt;pid&gt;** 强行停止数据节点。以该方式停止的数据节点被认为非正常停止。如果该节点非正常停止，则会被 sdbcm 进程尝试重新启动。启动后会与当前数据组中其它节点进行同步。
当前版本中，数据备份支持离线备份，即数据备份期间需要中断插入、更新、删除等变更操作，只支持查询操作。当前备份支持两种方式：全量备份和增量备份

-   全量备份：备份整个数据库的配置、数据和日志；
-   增量备份：在上一个全量备份或增量备份的基础上备份新增的日志和配置；

##离线备份参数说明##

  参数          说明
  ------------- ---------------------------------------------------------------------------------------------------------------
  Name          备份名称，缺省则以当前时间格式命名，如“2013-11-13-15:00:00”。
  Description   备份用户描述信息。
  Path          本次备份的指定路径，缺省为配置参数“bkuppath”中指定的路径。
  EnsureInc     备份方式，true 表示增量备份，false 表示全量备份，缺省为 false。
  OverWrite     对于同名备份是否覆盖，true 表示覆盖，false 表示不覆盖，如果同名则报错；缺省为 true。
  GroupName     对指定组进行备份，缺省为对全系统备份，当需要对多个组进行备份可以指定为数组类型，如：["datagroup1","datagroup2"]。

##备份整个数据库##

1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost",11810);</pre>

2.执行备份命令

<pre class="prettyprint lang-javascript">
> db.backupOffline({Name:"backupName",Description:"backup for all"})</pre>

##备份指定组的数据库##

1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost",11810);</pre>

2.执行备份命令

<pre class="prettyprint lang-javascript">
> db.backupOffline({Name:"backupName",Description:"backup group1",GroupName:"datagroup1"})</pre>

##备份指定节点的数据库##

1.连接到指定节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var dbdata = new Sdb("hostname1","servicename1");</pre>

2.执行备份命令

<pre class="prettyprint lang-javascript">
> dbdata.backupOffline({Name:"backupName",Description:"backup data node"})</pre>

**Note:**

catalog 编目组的名称固定为 SYSCatalogGroup
备份信息查看可以通过客户端和手工查看。

##查看备份信息参数说明##

  参数        说明
  ----------- -----------------------------------------------------------------------------------------------------------------------------
  Name        备份名称，缺省则查看目录下所有备份信息。
  Path        查看备份的指定路径，缺省为配置参数“bkuppath”中指定的路径。
  GroupName   查看指定组的备份信息，缺省为查看全系统备份信息，当需要查看多个组的备份信息可以指定为数组类型，如：["datagroup1","datagroup2"]。

##查看全系统备份信息##

1.连接到协调节点

  <pre class="prettyprint lang-javascript">
  $ /opt/sequoiadb/bin/sdb
  > var db = new Sdb("localhost",11810);</pre>

2.执行查看备份信息命令

  <pre class="prettyprint lang-javascript">
  > db.listBackup()
  {
    "Name": "test_bk",
    "NodeName": "vmsvr2-suse-x64-1:11800",
    "GroupName": "SYSCatalogGroup",
    "EnsureInc": false,
    "BeginLSNOffset": 0,
    "EndLSNOffset": 18744,
    "StartTime": "2013-11-13-16:06:31",
    "HasError": false
  }
  {
    "Name": "test_bk",
    "NodeName": "vmsvr2-suse-x64-1:11820",
    "GroupName": "db1",
    "EnsureInc": false,
    "BeginLSNOffset": 0,
    "EndLSNOffset": 920424,
    "StartTime": "2013-11-13-16:06:31",
    "HasError": false
  }</pre>

##查看指定名称的备份信息##

1.连接到协调节点

  <pre class="prettyprint lang-javascript">
  $ /opt/sequoiadb/bin/sdb
  > var db = new Sdb("localhost",11810);</pre>

2.执行查看备份信息命令

  <pre class="prettyprint lang-javascript">
  > db.listBackup({Name:"backup1"})
  {
    "Name": "backup1",
    "NodeName": "vmsvr2-suse-x64-1:11820",
    "GroupName": "group1",
    "EnsureInc": false,
    "BeginLSNOffset": 0,
    "EndLSNOffset": 108744,
    "StartTime": "2013-11-13-16:06:31",
    "HasError": false
  }</pre>

##手工查看备份信息##

手工查看备份信息直接通过终端登入指定机器，并进入到相应的备份目录中，执行“ls -l”

  <pre class="prettyprint lang-javascript">
  use@vmsvr2-suse-x64-1:/opt/sequoiadb/database/11820/bakfile> ls -l
  total 37328
  -rw-r----- 1 sdbadmin sdbadmin  38157784 Nov 13 16:06 test_bk.1
  -rw-r----- 1 sdbadmin sdbadmin     65536 Nov 13 16:06 test_bk.bak</pre>
使用备份的数据恢复某个分区组。执行数据恢复必须确保相应组已停止运行，数据恢复首先会清空原节点的所有数据和日志，然后从备份的数据中恢复配置、数据和日志。

##数据恢复工具参数说明##

  参数            缩写   说明
  --------------- ------ ---------------------------------------------------------------------------------------
  --bkpath        -p     备份源数据所在路径。
  --increaseid    -i     需要恢复到第几次增量备份，缺省恢复到最后一次。
  --bkname        -n     需要恢复的备份名。
  --action        -a     恢复行为，“restore”表示恢复，“list”表示查看备份信息，缺省为“restore”。
  --isSelf               是否为恢复本节点数据，缺省为“true”；当取值为“false”时，根据如下参数将数据恢复至指定路径：
  --dbpath               必须配置，数据文件目录。
  --confpath             必须配置，配置文件路径。
  --svcname              必须配置，本地服务名或端口。
  --indexpath            索引文件目录。
  --logpath              日志文件目录。
  --diagpath             诊断日志文件目录。
  --bkuppath             备份文件目录。
  --replname             复制通讯服务名或端口。
  --shardname            分区通讯服务名或端口。
  --catalogname          编目通讯服务名或端口。
  --httpname             REST 服务名或端口。

##恢复数据##

**Note:**

如果一个分区组包含多个数据节点，必须停止该组中每个数据节点并进行恢复。如果将备份的数据恢复至非备份数据节点，须使用
--isSelf false 配置参数，同时设置相关的配置参数。

1.连接到协调节点

  <pre class="prettyprint lang-javascript">
  $ /opt/sequoiadb/bin/sdb
  > var db = new Sdb("localhost", 11810) ;</pre>

2.得到分区组

  <pre class="prettyprint lang-javascript">
  > dataRG = db.getRG ( "data" ) ;</pre>

3.停止分区组

  <pre class="prettyprint lang-javascript">
  > dataRG.stop()</pre>

4.通过终端登入相应分区组的数据节点，执行数据恢复。

  <pre class="prettyprint lang-javascript">
  sdbadmin@vmsvr2-suse-x64-1:/opt/sequoiadb> bin/sdbrestore -p database/11820/bakfile -n test_bk
  Begin to clean dps logs...
  Begin to clean dms storages...
  Begin to init dps logs...
  Begin to restore...
  Begin to restore data file: 11820/bakfile/test_bk.1 ...
  Begin to restore su: test.1.data ...
  Begin to restore su: test.1.idx ...
  Begin to restore dps logs...
  *****************************************************
  Restore succeed!
  *****************************************************</pre>

5.到数据节点目录检查文件是否恢复。

  <pre class="prettyprint lang-javascript">
  sdbadmin @vmsvr2-suse-x64-1:/ opt/sequoiadb /database/11820> ls -l
  total 299156
  drwxr-xr-x 2 sdbadmin sdbadmin      4096 Nov 13 16:06 bakfile
  drwxr-xr-x 2 sdbadmin sdbadmin      4096 Nov 13 15:48 diaglog
  drwxr-xr-x 2 sdbadmin sdbadmin      4096 Nov 13 17:39 replicalog
  -rw-r----- 1 sdbadmin sdbadmin 155254784 Nov 13 17:39 test.1.data
  -rw-r----- 1 sdbadmin sdbadmin 151060480 Nov 13 17:39 test.1.idx</pre>

6.删除该分区组中其它数据节点的所有数据（或者将该节点的所有 .data 和 .idx 文件拷贝至其它数据节点的数据目录和索引目录下，以及将该节点 replicalog 所有日志拷贝至其它数据节点的日志目录下，或者将备份文件拷贝至其它数据节点，并通过 restored 工具恢复）；重新启动系统。
监控是一种监视当前系统状态的方式。在 SequoiaDB 中，用户可以使用快照（SNAPSHOT）与列表（LIST）命令进行系统监控。

**Note:** 

如果在集群环境下查询快照，连接协调节点就可以获取。

连接协调节点，默认是获取整个集群的快照信息，如：

<pre class="prettyprint lang-javascript">
snapshot(SDB_SNAP_SYSTEM )</pre>

要获取指定分区组的快照信息，使用条件查询，如：

<pre class="prettyprint lang-javascript">
snapshot(SDB_SNAP_SYSTEM,{ GroupName: "group1" } )</pre>

要获取指定节点的快照信息，如：

<pre class="prettyprint lang-javascript">
snapshot(SDB_SNAP_SYSTEM,{ HostName: "host1", svcname: "11820' } )</pre>

##快照##

快照是一种得到系统当前状态的命令，主要分为以下类型：

+-------------------------------------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------------------------+
| 快照标示                                                                            | 快照类型       | 描述                                                                   |
+=====================================================================================+================+========================================================================================================================+
| [SDB_SNAP_CONTEXTS](SdbDoc_Cn/database_management/monitoring/snapshot.html)         | 上下文         | 上下文快照列出当前数据库节点中所有的会话所对应的上下文                 |
+-------------------------------------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------------------------+
| [SDB_SNAP_CONTEXTS_CURRENT](SdbDoc_Cn/database_management/monitoring/snapshot.html) | 当前会话上下文 | 当前上下文快照列出当前数据库节点中当前会话所对应的上下文               |
+-------------------------------------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------------------------+
| [SDB_SNAP_SESSIONS](SdbDoc_Cn/database_management/monitoring/snapshot.html)         | 会话           | 会话快照列出当前数据库节点中所有的会话                                 |
+-------------------------------------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------------------------+
| [SDB_SNAP_SESSIONS_CURRENT](SdbDoc_Cn/database_management/monitoring/snapshot.html) | 当前会话       | 当前会话快照列出当前数据库节点中当前的会话                             |
+-------------------------------------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------------------------+
| [SDB_SNAP_COLLECTIONS](SdbDoc_Cn/database_management/monitoring/snapshot.html)      | 集合           | 集合快照列出当前数据库节点或集群中所有非临时集合                       |
+-------------------------------------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------------------------+
| [SDB_SNAP_COLLECTIONSPACES](SdbDoc_Cn/database_management/monitoring/snapshot.html) | 集合空间       | 集合空间快照列出当前数据库节点或集群中所有集合空间（编目集合空间除外） |
+-------------------------------------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------------------------+
| [SDB_SNAP_DATABASE](SdbDoc_Cn/database_management/monitoring/snapshot.html)         | 数据库         | 数据库快照列出当前数据库节点的数据库监视信息                           |
+-------------------------------------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------------------------+
| [SDB_SNAP_SYSTEM](SdbDoc_Cn/database_management/monitoring/snapshot.html)           | 系统           | 系统快照列出当前数据库节点的系统监视信息                               |
+-------------------------------------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------------------------+
| [SDB_SNAP_CATALOG](SdbDoc_Cn/database_management/monitoring/snapshot.html)          | 编目信息       | 用于查看编目信息                                                       |
+-------------------------------------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------------------------+

##列表##

列表是一种轻量级的得到系统当前状态的命令，主要分为以下类型：

+---------------------------------------------------------------------------------+----------------+----------------------------------------------------------------------------------------------------------------------------+
| 列表标示                                                                        | 列表类型       | 描述                                                                   |
+=================================================================================+================+============================================================================================================================+
| [SDB_LIST_CONTEXTS](SdbDoc_Cn/database_management/monitoring/list.html)         | 上下文         | 上下文列表列出当前数据库节点中所有的会话所对应的上下文                 |
+---------------------------------------------------------------------------------+----------------+----------------------------------------------------------------------------------------------------------------------------+
| [SDB_LIST_CONTEXTS_CURRENT](SdbDoc_Cn/database_management/monitoring/list.html) | 当前会话上下文 | 当前上下文列表列出当前数据库节点中当前会话所对应的上下文               |
+---------------------------------------------------------------------------------+----------------+----------------------------------------------------------------------------------------------------------------------------+
| [SDB_LIST_SESSIONS](SdbDoc_Cn/database_management/monitoring/list.html)         | 会话           | 会话列表列出当前数据库节点中所有的会话                                 |
+---------------------------------------------------------------------------------+----------------+----------------------------------------------------------------------------------------------------------------------------+
| [SDB_LIST_SESSIONS_CURRENT](SdbDoc_Cn/database_management/monitoring/list.html) | 当前会话       | 当前会话列表列出当前数据库节点中当前的会话                             |
+---------------------------------------------------------------------------------+----------------+----------------------------------------------------------------------------------------------------------------------------+
| [SDB_LIST_COLLECTIONS](SdbDoc_Cn/database_management/monitoring/list.html)      | 集合           | 集合列表列出当前数据库节点或集群中所有非临时集合                       |
+---------------------------------------------------------------------------------+----------------+----------------------------------------------------------------------------------------------------------------------------+
| [SDB_LIST_COLLECTIONSPACES](SdbDoc_Cn/database_management/monitoring/list.html) | 集合空间       | 集合空间列表列出当前数据库节点或集群中所有集合空间（编目集合空间除外） |
+---------------------------------------------------------------------------------+----------------+----------------------------------------------------------------------------------------------------------------------------+
| [SDB_LIST_STORAGEUNITS](SdbDoc_Cn/database_management/monitoring/list.html)     | 存储单元       | 存储单元列表列出当前数据库节点的全部存储单元信息                       |
+---------------------------------------------------------------------------------+----------------+----------------------------------------------------------------------------------------------------------------------------+
| [SDB_LIST_GROUPS](SdbDoc_Cn/database_management/monitoring/list.html)           | 分区组         | 分区组列表列出当前集群中的所有分区信息                                 |
+---------------------------------------------------------------------------------+----------------+----------------------------------------------------------------------------------------------------------------------------+
##上下文快照##

###描述###

上下文快照列出当前数据库节点中所有的会话所对应的上下文。

每一个会话为一条记录，如果一个会话中包括一个或一个以上的上下文时，其
Contexts 数组字段对每个上下文产生一个对象。

**Note:**

快照操作自身需产生一个上下文，因此结果集中至少会返回一个当前快照的上下文信息。

###标示###

SDB_SNAP_CONTEXTS

###字段信息###

  字段名                    类型     描述
  ------------------------- -------- ----------------------------------------
  SessionID                 字符串   会话 ID（主机名：端口号：ID）
  Contexts.ContextID        长整型   上下文 ID
  Contexts.Type             字符串   上下文类型，如：DUMP
  Contexts.Description      字符串   上下文的描述信息，如：包含当前的查询条件
  Contexts.DataRead         长整型   所读数据
  Contexts.IndexRead        长整型   所读索引
  Contexts.QueryTimeSpent   浮点数   查询总时间（秒）
  Contexts.StartTimestamp   时间戳   创建时间

###示例###

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS)
{
  "SessionID": "vmsvr2-suse-x64:11820:28",
  "Contexts": [
    {
      "ContextID": 12,
      "Type": "DUMP",
      "Description": "BufferSize:0",
      "DataRead": 0,
      "IndexRead": 0,
      "QueryTimeSpent": 0,
      "StartTimestamp": "2013-09-27-18.06.37.079570"
    }
  ]
}</pre>

##当前上下文快照##

###描述###

当前上下文快照列出数据库节点中，当前连接所对应的会话中的上下文。

返回一条记录，其中 Contexts 数组字段中包含当前会话中所有的上下文。

**Note:** 

快照操作自身需产生一个上下文，因此结果集中至少包含一个上下文。

###标示###

SDB_SNAP_CONTEXTS_CURRENT

###字段信息###

  字段名                    类型     描述
  ------------------------- -------- ----------------------------------------
  SessionID                 字符串   会话 ID（Hostname:Port:ID）
  Contexts.ContextID        长整型   上下文 ID
  Contexts.Type             字符串   上下文类型，如：DUMP
  Contexts.Description      字符串   上下文的描述信息，如：包含当前的查询条件
  Contexts.DataRead         长整型   所读数据
  Contexts.IndexRead        长整型   所读索引
  Contexts.QueryTimeSpent   浮点数   查询总时间（秒）
  Contexts.StartTimestamp   时间戳   创建时间

###示例###

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS_CURRENT)
{
  "SessionID": vmsvr2-suse-x64:11820:28,
  "Contexts": [
    {
      "ContextID": 13,
      "Type": "DUMP",
      "Description": "BufferSize:0",
      "DataRead": 0,
      "IndexRead": 0,
      "QueryTimeSpent": 0,
      "StartTimestamp": "2013-09-27-18.25.17.311168"
    }
  ]
}</pre>

##会话快照##

###描述###

会话快照列出当前数据库节点中所有的用户与系统会话，每一个会话为一条记录。

###标示###

SDB_SNAP_SESSIONS

###字段信息###


+-------------------+---------------+----------------------------------------------------+
| 字段名            | 类型          | 描述                                               |
+===================+===============+====================================================+
| SessionID         | 字符串        | 会话 ID（主机名：端口号：ID）                      |
+-------------------+---------------+----------------------------------------------------+
| TID               | 整型          | 该会话所对应的系统线程 ID                          |
+-------------------+---------------+----------------------------------------------------+
| Status            | 字符串        | 会话状态                                           |
|                   |               |                                                    |
|                   |               |  -   Creating：创建状态                            |
|                   |               |  -   Running：运行状态                             |
|                   |               |  -   Waiting：等待状态                             |
|                   |               |  -   Idle：线程池待机状态                          |
|                   |               |  -   Destroying：销毁状态                          |
+-------------------+---------------+----------------------------------------------------+
| Type              | 字符串        | [EDU 类型](SdbDoc_Cn/database_management/EDU.html) |
+-------------------+---------------+----------------------------------------------------+
| Name              | 字符串        | EDU 名，一般系统 EDU 名为空                        |
+-------------------+---------------+----------------------------------------------------+
| QueueSize         | 整型          | 等待处理请求的队列长度                             |
+-------------------+---------------+----------------------------------------------------+
| ProcessEventCount | 长整型        | 已经处理请求的数量                                 |
+-------------------+---------------+----------------------------------------------------+
| Contexts          | 长整型数组    | 上下文 ID 数组，为该会话所包含的所有上下文列表     |
+-------------------+---------------+----------------------------------------------------+
| TotalDataRead     | 长整型        | 数据记录读                                         |
+-------------------+---------------+----------------------------------------------------+
| TotalIndexRead    | 长整型        | 索引读                                             |
+-------------------+---------------+----------------------------------------------------+
| TotalDataWrite    | 长整型        | 数据记录写                                         |
+-------------------+---------------+----------------------------------------------------+
| TotalIndexWrite   | 长整型        | 索引写                                             |
+-------------------+---------------+----------------------------------------------------+
| TotalUpdate       | 长整型        | 总更新记录数量                                     |
+-------------------+---------------+----------------------------------------------------+
| TotalDelete       | 长整型        | 总删除记录数量                                     |
+-------------------+---------------+----------------------------------------------------+
| TotalInsert       | 长整型        | 总插入记录数量                                     |
+-------------------+---------------+----------------------------------------------------+
| TotalSelect       | 长整型        | 总选取记录数量                                     |
+-------------------+---------------+----------------------------------------------------+
| TotalRead         | 长整型        | 总数据读                                           |
+-------------------+---------------+----------------------------------------------------+
| TotalReadTime     | 长整型        | 总数据读时间（毫秒）                               |
+-------------------+---------------+----------------------------------------------------+
| TotalWriteTime    | 长整型        | 总数据写时间（毫秒）                               |
+-------------------+---------------+----------------------------------------------------+
| ReadTimeSpent     | 长整型        | 读取记录的时间（毫秒）                             |
+-------------------+---------------+----------------------------------------------------+
| WriteTimeSpent    | 长整型        | 写入记录的时间（毫秒）                             |
+-------------------+---------------+----------------------------------------------------+
| ConnectTimestamp  | 时间戳        | 连接发起时间                                       |
+-------------------+---------------+----------------------------------------------------+
| LastOpType        | 字符串        | 最后一次操作的类型，如：insert，update             |
+-------------------+---------------+----------------------------------------------------+
| LastOpBegin       | 字符串        | 最后一次操作的起始时间                             |
+-------------------+---------------+----------------------------------------------------+
| LastOpEnd         | 字符串        | 最后一次操作的结束时间                             |
+-------------------+---------------+----------------------------------------------------+
| LastOpInfo        | 字符串        | 最后一次操作的详细信息                             |
+-------------------+---------------+----------------------------------------------------+
| UserCPU           | 浮点数        | 用户 CPU（秒）                                     |
+-------------------+---------------+----------------------------------------------------+
| SysCPU            | 浮点数        | 系统 CPU（秒）                                     |
+-------------------+---------------+----------------------------------------------------+

###示例###

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_SESSIONS)
{
  "SessionID": "vmsvr2-suse-x64:11820:1",
  "TID": 8680,
  "Status": "Running",
  "Type": "LogWriter",
  "Name": "",
  "QueueSize": 0,
  "ProcessEventCount": 1,
  "Contexts": [],
  "TotalDataRead": 0,
  "TotalIndexRead": 0,
  "TotalDataWrite": 0,
  "TotalIndexWrite": 0,
  "TotalUpdate": 0,
  "TotalDelete": 0,
  "TotalInsert": 0,
  "TotalSelect": 0,
  "TotalRead": 0,
  "TotalReadTime": 0,
  "TotalWriteTime": 0,
  "ReadTimeSpent": 0,
  "WriteTimeSpent": 0,
  "ConnectTimestamp": "2013-09-27-13.28.38.927465",
  "LastOpType": "unknow",
  "LastOpBegin": "--",
  "LastOpEnd": "--",
  "LastOpInfo": "",
  "UserCPU": "0.410000",
  "SysCPU": "0.150000"
}</pre>

##当前会话快照##

###描述###

当前会话快照列出数据库节点中的当前用户会话，返回一条记录。

###标示###

SDB_SNAP_SESSIONS_CURRENT

###字段信息###
+-------------------+---------------+----------------------------------------------------+
| 字段名            | 类型          | 描述                                               |
+===================+===============+====================================================+
| SessionID         | 字符串        | 会话 ID（主机名：端口号：ID）                      |
+-------------------+---------------+----------------------------------------------------+
| TID               | 整型          | 该会话所对应的系统线程 ID                          |
+-------------------+---------------+----------------------------------------------------+
| Status            | 字符串        | 会话状态                                           |
|                   |               |                                                    |
|                   |               |  -   Creating：创建状态                            |
|                   |               |  -   Running：运行状态                             |
|                   |               |  -   Waiting：等待状态                             |
|                   |               |  -   Idle：线程池待机状态                          |
|                   |               |  -   Destroying：销毁状态                          |
+-------------------+---------------+----------------------------------------------------+
| Type              | 字符串        | [EDU 类型](SdbDoc_Cn/database_management/EDU.html) |
+-------------------+---------------+----------------------------------------------------+
| Name              | 字符串        | EDU 名，一般系统 EDU 名为空                        |
+-------------------+---------------+----------------------------------------------------+
| QueueSize         | 整型          | 等待处理请求的队列长度                             |
+-------------------+---------------+----------------------------------------------------+
| ProcessEventCount | 长整型        | 已经处理请求的数量                                 |
+-------------------+---------------+----------------------------------------------------+
| Contexts          | 长整型数组    | 上下文 ID 数组，为该会话所包含的所有上下文列表     |
+-------------------+---------------+----------------------------------------------------+
| TotalDataRead     | 长整型        | 数据记录读                                         |
+-------------------+---------------+----------------------------------------------------+
| TotalIndexRead    | 长整型        | 索引读                                             |
+-------------------+---------------+----------------------------------------------------+
| TotalDataWrite    | 长整型        | 数据记录写                                         |
+-------------------+---------------+----------------------------------------------------+
| TotalIndexWrite   | 长整型        | 索引写                                             |
+-------------------+---------------+----------------------------------------------------+
| TotalUpdate       | 长整型        | 总更新记录数量                                     |
+-------------------+---------------+----------------------------------------------------+
| TotalDelete       | 长整型        | 总删除记录数量                                     |
+-------------------+---------------+----------------------------------------------------+
| TotalInsert       | 长整型        | 总插入记录数量                                     |
+-------------------+---------------+----------------------------------------------------+
| TotalSelect       | 长整型        | 总选取记录数量                                     |
+-------------------+---------------+----------------------------------------------------+
| TotalRead         | 长整型        | 总数据读                                           |
+-------------------+---------------+----------------------------------------------------+
| TotalReadTime     | 长整型        | 总数据读时间（毫秒）                               |
+-------------------+---------------+----------------------------------------------------+
| TotalWriteTime    | 长整型        | 总数据写时间（毫秒）                               |
+-------------------+---------------+----------------------------------------------------+
| ReadTimeSpent     | 长整型        | 读取记录的时间（毫秒）                             |
+-------------------+---------------+----------------------------------------------------+
| WriteTimeSpent    | 长整型        | 写入记录的时间（毫秒）                             |
+-------------------+---------------+----------------------------------------------------+
| ConnectTimestamp  | 时间戳        | 连接发起时间                                       |
+-------------------+---------------+----------------------------------------------------+
| LastOpType        | 字符串        | 最后一次操作的类型，如：insert，update             |
+-------------------+---------------+----------------------------------------------------+
| LastOpBegin       | 字符串        | 最后一次操作的起始时间                             |
+-------------------+---------------+----------------------------------------------------+
| LastOpEnd         | 字符串        | 最后一次操作的结束时间                             |
+-------------------+---------------+----------------------------------------------------+
| LastOpInfo        | 字符串        | 最后一次操作的详细信息                             |
+-------------------+---------------+----------------------------------------------------+
| UserCPU           | 浮点数        | 用户 CPU（秒）                                     |
+-------------------+---------------+----------------------------------------------------+
| SysCPU            | 浮点数        | 系统 CPU（秒）                                     |
+-------------------+---------------+----------------------------------------------------+

###示例###

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_SESSIONS_CURRENT)
{
  "SessionID": "vmsvr2-suse-x64:11820:28",
  "TID": 9430,
  "Status": "Running",
  "Type": "Agent",
  "Name": "127.0.0.1:60309",
  "QueueSize": 0,
  "ProcessEventCount": 12,
  "Contexts": [
    15
  ],
  "TotalDataRead": 0,
  "TotalIndexRead": 0,
  "TotalDataWrite": 0,
  "TotalIndexWrite": 0,
  "TotalUpdate": 0,
  "TotalDelete": 0,
  "TotalInsert": 0,
  "TotalSelect": 0,
  "TotalRead": 0,
  "TotalReadTime": 0,
  "TotalWriteTime": 0,
  "ReadTimeSpent": 10,
  "WriteTimeSpent": 0,
  "ConnectTimestamp": "2013-09-27-18.06.25.961090",
  "LastOpType": "unknow",
  "LastOpBegin": "2014-08-07-14.25.23.550216",
  "LastOpEnd": "--",
  "LastOpInfo": "",
  "UserCPU": "0.910000",
  "SysCPU": "2.060000"
}</pre>

##集合快照##

###描述###

集合快照列出当前数据库节点中所有的非临时集合（协调节点中列出用户集合），每个集合为一条记录。

###标示###

SDB_SNAP_COLLECTIONS

###字段信息###

由于数据节点与编目节点保存的集合信息不同，集合快照在协调节点与其它节点所返回的结构有所不同：

###非协调节点字段信息###

+---------------------+---------------+---------------------------------------------------------+
| 字段名              | 类型          | 描述                                                    |
+=====================+===============+=========================================================+
| Name                | 字符串        | 集合完整名                                              |
+---------------------+---------------+---------------------------------------------------------+
| Details.ID          | 整型          | 集合 ID，范围0\~4095，集合空间内唯一                    |
+---------------------+---------------+---------------------------------------------------------+
| Details.LogicalID   | 整型          | 集合逻辑 ID                                             |
+---------------------+---------------+---------------------------------------------------------+
| Details.Sequence    | 整型          | 序列号                                                  |
+---------------------+---------------+---------------------------------------------------------+
| Details.Indexes     | 整型          | 该集合所包含的索引数量                                  |
+---------------------+---------------+---------------------------------------------------------+
| Details.Status      | 字符串        | 集合当前状态                                            |
|                     |               |                                                         |
|                     |               |  -   Free：空闲                                         |
|                     |               |  -   Normal：正常                                       |
|                     |               |  -   Dropped：被删除                                    |
|                     |               |  -   Offline Reorg Shadow Copy Phase：离线重组复制阶段  |
|                     |               |  -   Offline Reorg Truncate Phase：离线重组清除阶段     |
|                     |               |  -   Offline Reorg Copy Back Phase：离线重组重入阶段    |
|                     |               |  -   Offline Reorg Rebuild Phase：离线重组重建索引阶段  |
+---------------------+---------------+---------------------------------------------------------+
| TotalRecords        | 长整型        | 集合的记录总数                                          |
+---------------------+---------------+---------------------------------------------------------+
| TotalDataPages      | 整型          | 集合的数据页总数                                        |
+---------------------+---------------+---------------------------------------------------------+
| TotalIndexPages     | 整型          | 集合的索引页总数                                        |
+---------------------+---------------+---------------------------------------------------------+
| TotalLobPages       | 整型          | 集合的大对象页总数                                      |
+---------------------+---------------+---------------------------------------------------------+
| TotalDataFreeSpace  |长整型         | 集合的数据空闲空间                                      |
+---------------------+---------------+---------------------------------------------------------+
| TotalIndexFreeSpace | 长整型        | 集合的索引空闲空间                                      |
+---------------------+---------------+---------------------------------------------------------+


###协调节点字段信息###

+-----------------------------------+---------------+---------------------------------------------------------+
| 字段名                            | 类型          | 描述                                                    |
+===================================+===============+=========================================================+
| Name                              | 字符串        | 集合完整名                                              |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.GroupName                 | 字符串        | 节点所在分区组名                                        |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.Group.ID                  | 整型          | 集合 ID，范围0\~4096，集合空间内唯一                    |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.Group.LogicalID           | 整型          | 集合逻辑 ID                                             |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.Group.Sequence            | 整型          | 序列号                                                  |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.Group.Indexes             | 整型          | 该集合所包含的索引数量                                  |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.Status                    | 字符串        | 集合当前状态                                            |
|                                   |               |                                                         |
|                                   |               |  -   Free：空闲                                         |
|                                   |               |  -   Normal：正常                                       |
|                                   |               |  -   Dropped：被删除                                    |
|                                   |               |  -   Offline Reorg Shadow Copy Phase：离线重组复制阶段  |
|                                   |               |  -   Offline Reorg Truncate Phase：离线重组清除阶段     |
|                                   |               |  -   Offline Reorg Copy Back Phase：离线重组重入阶段    |
|                                   |               |  -   Offline Reorg Rebuild Phase：离线重组重建索引阶段  |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.Group.TotalRecords        | 长整型        | 集合的记录总数                                          |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.Group.TotalDataPages      | 整型          | 集合的数据页总数                                        |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.Group.TotalIndexPages     | 整型          | 集合的索引页总数                                        |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.Group.TotalDataFreeSpace  | 长整型        | 集合的数据空闲空间                                      |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.Group.TotalIndexFreeSpace | 长整型        | 集合的索引空闲空间                                      |
+-----------------------------------+---------------+---------------------------------------------------------+
| Details.Group.NodeName            | 字符串        | 节点名（主机名 + 端口）                                 |
+-----------------------------------+---------------+---------------------------------------------------------+


###非协调节点示例###

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_COLLECTIONS)
{
  "Name": "foo.bar",
  "Details": [
    {
      "ID": 0,
      "LogicalID": 0,
      "Sequence": 1,
      "Indexes": 8,
      "Status": "Normal",
      "TotalRecords": 0,
      "TotalDataPages": 0,
      "TotalIndexPages": 6,
      "TotalLobPages": 0,
      "TotalDataFreeSpace": 0,
      "TotalIndexFreeSpace": 196545
    }
  ]
}</pre>

###协调节点示例###

<pre class="prettyprint lang-javascript">
> coord.snapshot(SDB_SNAP_COLLECTIONS)
{
  "Name": "susefoo.susebar",
  "Details": [
    {
      "GroupName": "datagroup1",
      "Group": [
        {
          "ID": 0,
          "LogicalID": 0,
          "Sequence": 1,
          "Indexes": 1,
          "Status": "Normal",
          "TotalRecords": 1,
          "TotalDataPages": 1,
          "TotalIndexPages": 2,
          "TotalLobPages": 0,
          "TotalDataFreeSpace": 4004,
          "TotalIndexFreeSpace": 4046,
          "NodeName": "vmsvr2-suse-x64:11820"
        }
      ]
    }
  ]
}</pre>

##集合空间快照##

###描述###

集合空间快照列出当前数据库节点中所有的集合空间，每个集合空间为一条记录。

###标示###

SDB_SNAP_COLLECTIONSPACES

###字段信息###

由于数据节点与编目节点保存的集合空间信息不同，集合空间快照在协调节点与其它节点所返回的结构有所不同：

###非协调节点字段信息###

  字段名            类型         描述
  ----------------- ------------ --------------------------------
  Name              字符串       集合空间名
  Collection        字符串数组   集合空间中所包含的所有集合
  PageSize          整型         集合空间数据页大小
  LobPageSize       整型         集合空间大对象数据页大小
  MaxCapacitySize   长整型       集合空间的最大容量上限
  MaxDataCapSize    长整型       集合空间数据文件最大容量上限
  MaxIndexCapSize   长整型       集合空间索引文件最大容量上限
  MaxLobCapSize     长整型       集合空间大对象文件最大容量上限
  NumCollections    整型         集合数量
  TotalRecords      整型         集合空间的记录总数
  TotalSize         长整型       集合空间的总大小
  FreeSize          长整型       集合空间的空闲大小
  TotalDataSize     长整型       集合空间数据文件总大小
  FreeDataSize      长整型       集合空间数据文件空闲空间大小
  TotalIndexSize    长整型       集合空间索引文件总大小
  FreeIndexSize     长整型       集合空间索引文件空闲空间大小
  TotalLobSize      长整型       集合空间大对象文件总大小
  FreeLobSize       长整型       集合空间大对象文件空闲空间大小

###协调节点字段信息###

  字段名            类型         描述
  ----------------- ------------ --------------------------------
  Name              字符串       集合空间名
  Collection        字符串数组   集合空间中所包含的所有集合
  PageSize          整型         集合空间数据页大小
  LobPageSize       整型         集合空间大对象数据页大小
  TotalSize         长整型       集合空间的总大小
  FreeSize          长整型       集合空间的空闲大小
  TotalDataSize     长整型       集合空间数据文件总大小
  FreeDataSize      长整型       集合空间数据文件空闲空间大小
  TotalIndexSize    长整型       集合空间索引文件总大小
  FreeIndexSize     长整型       集合空间索引文件空闲空间大小
  TotalLobSize      长整型       集合空间大对象文件总大小
  FreeLobSize       长整型       集合空间大对象文件空闲空间大小
  Group.GroupName   字符串       该集合空间所在的分区组名列表

###非协调节点示例###

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_COLLECTIONSPACES)
{
  "Collection": [
    {
      "Name": "bar"
    }
  ],
  "PageSize": 65536,
  "LobPageSize": 262144,
  "MaxCapacitySize": 26388279066624,  
  "MaxDataCapSize": 8796093022208,
  "MaxIndexCapSize": 8796093022208,
  "MaxLobCapSize": 8796093022208,
  "NumCollections": 4,
  "TotalRecords": 2,
  "TotalSize": 306315264,
  "FreeSize": 265551224,
  "TotalDataSize": 155254784,
  "FreeDataSize": 133627904,
  "TotalIndexSize": 151060480,
  "FreeIndexSize": 134152171,
  "TotalLobSize": 352714752,
  "FreeLobSize": 140771328,
  "Name": "foo"
}</pre>

###协调节点示例###

<pre class="prettyprint lang-javascript">
> coord.snapshot(SDB_SNAP_COLLECTIONSPACES)
{
  "Name": "foo",
  "PageSize": 4096,  
  "LobPageSize": 262144,
  "TotalSize": 918945792,
  "FreeSize": 805183062,  
  "TotalDataSize": 155254784,
  "FreeDataSize": 133627904,
  "TotalIndexSize": 151060480,
  "FreeIndexSize": 134152171,
  "TotalLobSize": 352714752,
  "FreeLobSize": 140771328,
  "Collection": [
    {
      "Name": "bar"
    }
  ],
  "Group": [
    "db2"
  ]
}</pre>

##数据库快照##

###描述###

数据库快照列出当前数据库节点中主要的状态与性能监控参数，输出一条记录。

###标示###

SDB_SNAP_DATABASE

###非协调节点字段信息###

  字段名                  类型     描述
  ----------------------- -------- -----------------------------------------------------------------------------------------------------------------
  HostName                字符串   数据库节点所在物理节点的主机名
  ServiceName             字符串   svcname 所指定的服务名，与 HostName 共同作为一个逻辑节点的标示
  NodeName                字符串   节点名，为<HostName>:<ServiceName>
  GroupName               字符串   该逻辑节点所属的分区组名，standalone 模式下该字段为空字符串
  IsPrimary               布尔     该节点是否为主节点，standalone 模式下该字段为false
  ServiceStatus           布尔     是否为可提供服务状态。一些特殊状态，例如[全量同步](SdbDoc_Cn/infrastructure/replication/replicate.html)会使该状态为 false
  BeginLSN.Offset         长整型   起始 LSN 的偏移
  BeginLSN.Version        整型     起始 LSN 的版本号
  CurrentLSN.Offset       长整型   当前 LSN 的偏移
  CurrentLSN.Version      整型     当前 LSN 的版本号
  TransInfo.BeginLSN      长整型   事务起始 LSN 的偏移
  NodeID                  数组     [ 分区组 ID，节点 ID ]，在 standalone 模式下，该字段为 [ 0，0 ]
  Version.Major           整型     数据库主版本号
  Version.Minor           整型     数据库子版本号
  Version.Release         整型     数据库发行版本号
  Version.Build           字符串   数据库编译时间
  CurrentActiveSessions   整型     当前活动会话，该数量包括用户 EDU 与系统 EDU
  CurrentIdleSessions     整型     当前非活动会话，一般来说非活动会话意味着 EDU 存在线程池中等待分配
  CurrentSystemSessions   整型     当前系统会话，为当前活动用户 EDU 数量
  CurrentContexts         整型     当前上下文数量
  ReceivedEvents          整型     当前分区接收到的事件请求总数
  Role                    字符串   当前节点角色
  Disk.DatabasePath       字符串   数据库所在路径
  Disk.LoadPercent        整型     数据库路径磁盘占用率百分比
  Disk.TotalSpace         长整型   数据库路径总空间（字节）
  Disk.FreeSpace          长整型   数据库路径空闲空间（字节）**重要：该字段以及以上所有字段仅在数据节点和编目节点显示，协调节点不显示**
  TotalNumConnects        整型     数据库连接请求数量
  TotalDataRead           长整型   总数据读请求
  TotalIndexRead          长整型   总索引读请求
  TotalDataWrite          长整型   总数据写请求
  TotalIndexWrite         长整型   总索引写请求
  TotalUpdate             长整型   总更新记录数量
  TotalDelete             长整型   总删除记录数量
  TotalInsert             长整型   总插入记录数量
  ReplUpdate              长整型   复制更新记录数量
  ReplDelete              长整型   复制删除记录数量
  ReplInsert              长整型   复制插入记录数量
  TotalSelect             长整型   总选择记录数量
  TotalRead               长整型   总读取记录数量
  TotalReadTime           长整型   总读取时间（毫秒）
  TotalWriteTime          长整型   总写入时间（毫秒）
  ActivateTimestamp       时间戳   数据库节点启动时间
  UserCPU                 浮点数   用户 CPU（秒）
  SysCPU                  浮点数   系统 CPU（秒）
  freeLogSpace            长整型   空闲日志空间
  vsize                   长整型   虚拟内存使用量
  rss                     长整型   物理内存使用量
  fault                   长整型   每秒访问失败数（仅支持 Linux），数据被交换出物理内存，放到 swap
  TotalMapped             长整型   mmap 的总数据量
  svcNetIn                长整型   本地服务端口收到的网络流量
  svcNetOut               长整型   本地服务端口发送的网络流量

###协调节点字段信息###

  字段名              类型     描述
  ------------------- -------- --------------------------------------------------------------------------------
  TotalNumConnects    整型     数据库连接请求数量
  TotalDataRead       长整型   总数据读请求
  TotalIndexRead      长整型   总索引读请求
  TotalDataWrite      长整型   总数据写请求
  TotalIndexWrite     长整型   总索引写请求
  TotalUpdate         长整型   总更新记录数量
  TotalDelete         长整型   总删除记录数量
  TotalInsert         长整型   总插入记录数量
  ReplUpdate          长整型   复制更新记录数量
  ReplDelete          长整型   复制删除记录数量
  ReplInsert          长整型   复制插入记录数量
  TotalSelect         长整型   总选择记录数量
  TotalRead           长整型   总读取记录数量
  TotalReadTime       长整型   总读取时间（毫秒）
  TotalWriteTime      长整型   总写入时间（毫秒）
  freeLogSpace        长整型   空闲日志空间
  vsize               长整型   虚拟内存使用量
  rss                 长整型   物理内存使用量
  fault               长整型   每秒访问失败数（仅支持 Linux），数据被交换出物理内存，放到 swap
  TotalMapped         长整型   mmap 的总数据量
  svcNetIn            长整型   本地服务端口收到的网络流量
  svcNetOut           长整型   本地服务端口发送的网络流量
  shardNetIn          长整型   shard 平面端口收到的网络流量
  shardNetOut         长整型   shard 平面端口发送的网络流量
  replNetIn           长整型   数据同步平面端口收到的网络流量
  replNetOut          长整型   数据同步平面端口发送的网络流量
  ErrNodes.NodeName   字符串   返回异常节点名（主机名 + 端口）**重要：此字段仅在协调节点上并且有异常节点时显示**
  ErrNodes.Flag       整型     错误码 **重要：此字段仅在协调节点上并且有异常节点时显示**

###非协调节点示例###

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_DATABASE)
{
  "NodeName": "ubuntu-dev12:11810",
  "HostName": "ubuntu-dev12",
  "ServiceName": "11810",
  "GroupName": "",
  "IsPrimary": false,
  "ServiceStatus": true,
  "BeginLSN": {
    "Offset": 0,
    "Version": 1
  },
  "CurrentLSN": {
    "Offset": -1,
    "Version": 0
  },
  "TransInfo": {
    "BeginLSN": -1
  },
  "NodeID": [
    0,
    0
  ],
  "Version": {
    "Major": 1,
    "Minor": 8,
    "Release": 13971,
    "Build": "2014-08-07-11.04.12(Debug)"
  },
  "CurrentActiveSessions": 18,
  "CurrentIdleSessions": 0,
  "CurrentSystemSessions": 5,
  "CurrentContexts": 1,
  "ReceivedEvents": 0,
  "Role": "standalone",
  "Disk": {
    "DatabasePath": "/home/users/hejiawen/sequoiadb_new/sequoiadb/trunk/bin",
    "LoadPercent": 46,
    "TotalSpace": 84543193088,
    "FreeSpace": 45332840448
  },
  "TotalNumConnects": 11,
  "TotalDataRead": 0,
  "TotalIndexRead": 0,
  "TotalDataWrite": 0,
  "TotalIndexWrite": 0,
  "TotalUpdate": 0,
  "TotalDelete": 0,
  "TotalInsert": 0,
  "ReplUpdate": 0,
  "ReplDelete": 0,
  "ReplInsert": 0,
  "TotalSelect": 0,
  "TotalRead": 0,
  "TotalReadTime": 0,
  "TotalWriteTime": 0,
  "ActivateTimestamp": "2014-08-07-13.04.16.248083",
  "UserCPU": "7.980000",
  "SysCPU": "10.700000",
  "freeLogSpace": 1342177280,
  "vsize": 1745002496,
  "rss": 12929,
  "fault": 12,
  "TotalMapped": 918945792,
  "svcNetIn": 3051,
  "svcNetOut": 9245
}</pre>

##协调节点示例##

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_DATABASE)
{
  "TotalNumConnects": 0,
  "TotalDataRead": 4,
  "TotalIndexRead": 0,
  "TotalDataWrite": 3,
  "TotalIndexWrite": 3,
  "TotalUpdate": 0,
  "TotalDelete": 0,
  "TotalInsert": 3,
  "ReplUpdate": 0,
  "ReplDelete": 0,
  "ReplInsert": 2,
  "TotalSelect": 606,
  "TotalRead": 4,
  "TotalReadTime": 0,
  "TotalWriteTime": 0,
  "freeLogSpace": 5368709120,
  "vsize": 5660057600,
  "rss": 44765,
  "fault": 25,
  "TotalMapped": 2144206848,
  "svcNetIn": 0,
  "svcNetOut": 0,
  "shardNetIn": 38228,
  "shardNetOut": 393997,
  "replNetIn": 40743956,
  "replNetOut": 40743956,
  "ErrNodes": []
}</pre>

##操作系统快照##

###描述###

操作系统快照列出当前数据库节点所在操作系统中主要的状态与性能监控参数，输出一条记录。

###标示###

SDB_SNAP_SYSTEM

###非协调节点字段信息###

  字段名                类型     描述
  --------------------- -------- -----------------------------------------------------------------------------------------------------------------
  HostName              字符串   数据库节点所在物理节点的主机名
  ServiceName           字符串   svcname 所指定的服务名，与 HostName 共同作为一个逻辑节点的标示
  NodeName              字符串   节点名，为< HostName > : < ServiceName >
  GroupName             字符串   该逻辑节点所属的分区组名，standalone 模式下，该字段为空字符串
  IsPrimary             布尔     该节点是否为主节点，standalone 模式下，该字段为 false
  ServiceStatus         布尔     是否为可提供服务状态。一些特殊状态，例如[全量同步](SdbDoc_Cn/infrastructure/replication/replicate.html)会使该状态为 false
  BeginLSN.Offset       长整型   起始 LSN 的偏移
  BeginLSN.Version      整型     起始 LSN 的版本号
  CurrentLSN.Offset     整型     当前 LSN 的偏移
  TransInfo.BeginLSN    长整型   事务起始 LSN 的偏移
  NodeID                数组     [ 分区组ID，节点ID ]，standalone 模式下，该字段为 [ 0，0 ]
  CurrentLSN.Version    整型     当前 LSN 的版本号
  CPU.User              浮点数   操作系统启动后所消耗的总用户 CPU（秒）
  CPU.Sys               浮点数   操作系统启动后所消耗的总系统 CPU（秒）
  CPU.Idle              浮点数   操作系统启动后所消耗的总空闲 CPU（秒）
  CPU.Other             浮点数   操作系统启动后所消耗的总其它 CPU（秒）
  Memory.LoadPercent    整型     当前操作系统的内存使用百分比（包括文件系统缓存） **重要：该字段仅在数据节点和编目节点显示，协调节点不显示**
  Memory.TotalRAM       长整型   当前操作系统的总内存空间（字节）
  Memory.FreeRAM        长整型   当前操作系统的空闲内存空间（字节）
  Memory.TotalSwap      长整型   当前操作系统的总交换空间（字节）
  Memory.FreeSwap       长整型   当前操作系统的空闲交换空间（字节）
  Memory.TotalVirtual   长整型   当前操作系统的总虚拟空间（字节）
  Memory.FreeVirtual    长整型   当前操作系统的空闲虚拟空间（字节）
  Disk.DatabasePath     字符串   数据库路径 **重要：该字段及以上字段仅在数据节点和编目节点显示，协调节点不显示**
  Disk.LoadPercent      整型     数据库路径所在文件系统的空间占用百分比 **重要：该字段及以上字段仅在数据节点和编目节点显示，协调节点不显示**
  Disk.TotalSpace       长整型   数据库路径总空间（字节）
  Disk.FreeSpace        长整型   数据库路径空闲空间（字节）

###协调节点字段信息###

  字段名                类型     描述
  --------------------- -------- -----------------------------------------------------------------------------------------------------------
  CPU.User              浮点数   操作系统启动后所消耗的总用户 CPU（秒）
  CPU.Sys               浮点数   操作系统启动后所消耗的总系统 CPU（秒）
  CPU.Idle              浮点数   操作系统启动后所消耗的总空闲 CPU（秒）
  CPU.Other             浮点数   操作系统启动后所消耗的总其它 CPU（秒）
  Memory.LoadPercent    整型     当前操作系统的内存使用百分比（包括文件系统缓存） **重要：该字段仅在数据节点和编目节点显示，协调节点不显示**
  Memory.TotalRAM       长整型   当前操作系统的总内存空间（字节）
  Memory.FreeRAM        长整型   当前操作系统的空闲内存空间（字节）
  Memory.TotalSwap      长整型   当前操作系统的总交换空间（字节）
  Memory.FreeSwap       长整型   当前操作系统的空闲交换空间（字节）
  Memory.TotalVirtual   长整型   当前操作系统的总虚拟空间（字节）
  Memory.FreeVirtual    长整型   当前操作系统的空闲虚拟空间（字节）
  Disk.DatabasePath     字符串   数据库路径 **重要：该字段及以上字段仅在数据节点和编目节点显示，协调节点不显示**
  Disk.LoadPercent      整型     数据库路径所在文件系统的空间占用百分比 **重要：该字段及以上字段仅在数据节点和编目节点显示，协调节点不显示**
  Disk.TotalSpace       长整型   数据库路径总空间（字节）
  Disk.FreeSpace        长整型   数据库路径空闲空间（字节）
  ErrNodes.NodeName     字符串   返回异常节点名（主机名 + 端口）**重要：此字段仅在协调节点上显示，并且有异常节点时才显示**
  ErrNodes.Flag         整型     错误码 **重要：此字段仅在协调节点上显示，并且有异常节点时才显示**

###非协调节点示例###

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_SYSTEM)
{
  "NodeName": "vmsvr2-suse-x64:11820",
  "HostName": "vmsvr2-suse-x64",
  "ServiceName": "11820",
  "GroupName": "datagroup1",
  "IsPrimary": false,
  "ServiceStatus": true,
  "BeginLSN": {
    "Offset": 0,
    "Version": 1
  },
  "CurrentLSN": {
    "Offset": 3764,
    "Version": 1
  },
  "NodeID": [
    1000,
    1000
  ],
  "TransInfo": {
    "BeginLSN": -1
    },
  "NodeID": [
    0,
    0
    ],
  "CPU": {
    "User": 3947.31,
    "Sys": 715.11,
    "Idle": 331196.41,
    "Other": 771.14
  },
  "Memory": {
    "LoadPercent": 95,
    "TotalRAM": 4155072512,
    "FreeRAM": 202219520,
    "TotalSwap": 2153771008,
    "FreeSwap": 2137071616,
    "TotalVirtual": 6308843520,
    "FreeVirtual": 2339291136
  },
  "Disk": {
    "DatabasePath": "/opt/sequoiadb/database/data/11820",
    "LoadPercent": 78,
    "TotalSpace": 40704466944,
    "FreeSpace": 8615747584
  }
}</pre>

###协调节点示例###

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_SYSTEM)
{
  "CPU": {
    "User": 36280.72,
    "Sys": 5046.23,
    "Idle": 7560242.4,
    "Other": 5887.24
  },
  "Memory": {
    "TotalRAM": 8403730432,
    "FreeRAM": 3075035136,
    "TotalSwap": 25757204480,
    "FreeSwap": 25663799296,
    "TotalVirtual": 34160934912,
    "FreeVirtual": 28738834432
  },
  "Disk": {
    "TotalSpace": 338172772352,
    "FreeSpace": 181331296256
  },
  "ErrNodes": []
}</pre>


##编目信息快照##

###描述###

编目信息快照列出当前数据库中所有集合的编目信息，每个集合一条记录。

###标示###

SDB_SNAP_CATALOG

Note: 只能在协调节点执行。

###协调节点字段信息###

+---------------------+---------------+--------------------------------------------------------------------------------------------------------------------+
| 字段名              | 类型          | 描述                                                                                                               |
+=====================+===============+====================================================================================================================+
| Name                | 字符串        | 集合完整名                                                                                                         |
+---------------------+---------------+--------------------------------------------------------------------------------------------------------------------+
| EnsureShardingIndex | 布尔类型      | 是否自动为分区键字段创建索引                                                                                       |
+---------------------+---------------+--------------------------------------------------------------------------------------------------------------------+
| ReplSize            | 整型          | 执行修改操作时需要同步的副本数。当执行更新，插入，删除记录等操作时，仅当指定副本数的节点都完成操作时才返回操作结果 |
+---------------------+---------------+--------------------------------------------------------------------------------------------------------------------+
| ShardingKey         | 对象          | 数据分区类型：                                                                                                     |
|                     |               |                                                                                                                    |
|                     |               |     -   range：数据按分区键值的范围进行分区存储                                                                    |
|                     |               |     -   hash：数据按分区键的哈希值进行分区存储                                                                     |
+---------------------+---------------+--------------------------------------------------------------------------------------------------------------------+
| Version             | 整型          | 集合版本号，当对集合的元数据执行修改操作时递增该版本号（例如数据切分）                                             |
+---------------------+---------------+--------------------------------------------------------------------------------------------------------------------+
| CataInfo.GroupID    | 整型          | 分区组 ID                                                                                                          |
+---------------------+---------------+--------------------------------------------------------------------------------------------------------------------+
| CataInfo.GroupName  | 字符串        | 分区组名                                                                                                           |
+---------------------+---------------+--------------------------------------------------------------------------------------------------------------------+
| CataInfo.LowBound   | 对象          | 数据分区区间的上限                                                                                                 |
+---------------------+---------------+--------------------------------------------------------------------------------------------------------------------+
| CataInfo.UpBound    | 对象          | 数据分区区间的下限                                                                                                 |
+---------------------+---------------+--------------------------------------------------------------------------------------------------------------------+

###示例###

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CATALOG)
{
  "_id": {
    "$oid": "5247a2bc60080822db1cfba2"
  },
  "Name": "foo.bar",
  "Version": 1,
  "ReplSize": 1,
  "ShardingKey": {
    "age": 1
  },
  "EnsureShardingIndex": true,
  "ShardingType": "range",
  "CataInfo": [
    {
      "GroupID": 1000,
      "GroupName": "datagroup1",
      "LowBound": {
        "": {
          "$minKey": 1
        }
      },
      "UpBound": {
        "": {
          "$maxKey": 1
        }
      }
    }
  ]
}</pre>
##上下文列表##

###描述###

上下文列表列出当前数据库节点中所有的会话所对应的上下文。

每一个会话为一条记录，如果一个会话中包括一个或一个以上的上下文时，其 Contexts 数组字段对每个上下文产生一个对象。

**Note:**

列表操作自身需产生一个上下文，因此结果集中至少会返回一个当前列表的上下文信息。

###标示###

SDB_LIST_CONTEXTS

###字段信息###

  字段名      类型         描述
  ----------- ------------ -----------------------------------------------
  SessionID   长整型       会话 ID
  Contexts    长整型数组   上下文 ID 数组，为该会话所包含的所有上下文列表

###示例###

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_CONTEXTS)
{
  "SessionID": 21,
  "Contexts": [
    182
  ]
}</pre>

##当前上下文列表##

###描述###

当前上下文列表列出数据库节点中，当前连接所对应的会话中的上下文。

返回一条记录，其中 Contexts 数组字段中包含当前会话中所有的上下文。

**Note:** 

列表操作自身需产生一个上下文，因此结果集中至少包含一个上下文。

###标示###

SDB_LIST_CONTEXTS_CURRENT

###字段信息###

  字段名      类型         描述
  ----------- ------------ -----------------------------------------------
  SessionID   长整型       会话 ID
  Contexts    长整型数组   上下文 ID 数组，为该会话所包含的所有上下文列表

###示例###

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_CONTEXTS_CURRENT)
{
  "SessionID": 21,
  "Contexts": [
    183
  ]
}</pre>

##会话列表##

###描述###

会话列表列出当前数据库节点中所有的用户与系统会话，每一个会话为一条记录。

###标示###

SDB_LIST_SESSIONS

###字段信息###

+-------------------+---------------+----------------------------------------------------+
| 字段名            | 类型          | 描述                                               |
+===================+===============+====================================================+
| SessionID         | 整型或长整型  | 会话 ID                                            |
+-------------------+---------------+----------------------------------------------------+
| TID               | 整型          | 该会话所对应的系统线程 ID                          |
+-------------------+---------------+----------------------------------------------------+
| Status            | 字符串        | 会话状态                                           |
|                   |               |                                                    |
|                   |               |  -   Creating：创建状态                            |
|                   |               |  -   Running：运行状态                             |
|                   |               |  -   Waiting：等待状态                             |
|                   |               |  -   Idle：线程池待机状态                          |
|                   |               |  -   Destroying：销毁状态                          |
+-------------------+---------------+----------------------------------------------------+
| Type              | 字符串        | [EDU 类型](SdbDoc_Cn/database_management/EDU.html) |
+-------------------+---------------+----------------------------------------------------+
| Name              | 字符串        | EDU 名，一般系统 EDU 名为空                        |
+-------------------+---------------+----------------------------------------------------+

###示例###

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_SESSIONS)
{
  "SessionID": 1,
  "TID": 6168,
  "Status": "Running",
  "Type": "TCPListener",
  "Name": ""
}
{
  "SessionID": 2,
  "TID": 6169,
  "Status": "Running",
  "Type": "HTTPListener",
  "Name": ""
}
...
{
  "SessionID": 21,
  "TID": 6691,
  "Status": "Running",
  "Type": "Agent",
  "Name": "192.168.20.101:52741"
}</pre>

##当前会话列表##

###描述###

当前会话列表列出数据库节点中的当前用户会话，返回一条记录。

###标示###

SDB_LIST_SESSIONS_CURRENT

###字段信息###

+-------------------+---------------+----------------------------------------------------+
| 字段名            | 类型          | 描述                                               |
+===================+===============+====================================================+
| SessionID         | 整型或长整型  | 会话 ID                                            |
+-------------------+---------------+----------------------------------------------------+
| TID               | 整型          | 该会话所对应的系统线程 ID                          |
+-------------------+---------------+----------------------------------------------------+
| Status            | 字符串        | 会话状态                                           |
|                   |               |                                                    |
|                   |               |  -   Creating：创建状态                            |
|                   |               |  -   Running：运行状态                             |
|                   |               |  -   Waiting：等待状态                             |
|                   |               |  -   Idle：线程池待机状态                          |
|                   |               |  -   Destroying：销毁状态                          |
+-------------------+---------------+----------------------------------------------------+
| Type              | 字符串        | [EDU 类型](SdbDoc_Cn/database_management/EDU.html) |
+-------------------+---------------+----------------------------------------------------+
| Name              | 字符串        | EDU 名，一般系统 EDU 名为空                        |
+-------------------+---------------+----------------------------------------------------+

###示例###

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_SESSIONS_CURRENT)
{
  "SessionID": 21,
  "TID": 6691,
  "Status": "Running",
  "Type": "Agent",
  "Name": "192.168.20.101:52741"
}</pre>

##集合列表##

###描述###

集合快照列出当前数据库节点中所有的非临时集合（协调节点中列出用户集合），每个集合为一条记录。

###标示###

SDB_LIST_COLLECTIONS

###字段信息###

由于数据节点与编目节点保存的集合信息不同，集合列表在协调节点与其它节点所返回的结构有所不同：

###非协调节点字段信息###

+-------------------+---------------+---------------------------------------------------------+
| 字段名            | 类型          | 描述                                                    |
+===================+===============+=========================================================+
| Name              | 字符串        | 集合完整名                                              |
+-------------------+---------------+---------------------------------------------------------+
| Details.ID        | 整型          | 集合 ID，范围0\~4095，集合空间内唯一                    |
+-------------------+---------------+---------------------------------------------------------+
| Details.LogicalID | 整型          | 集合逻辑 ID                                             |
+-------------------+---------------+---------------------------------------------------------+
| Details.Sequence  | 整型          | 序列号                                                  |
+-------------------+---------------+---------------------------------------------------------+
| Details.Indexes   | 整型          | 该集合所包含的索引数量                                  |
+-------------------+---------------+---------------------------------------------------------+
| Details.Status    | 字符串        | 集合当前状态                                            |
|                   |               |                                                         |
|                   |               |  -   Free：空闲                                         |
|                   |               |  -   Normal：正常                                       |
|                   |               |  -   Dropped：被删除                                    |
|                   |               |  -   Offline Reorg Shadow Copy Phase：离线重组复制阶段  |
|                   |               |  -   Offline Reorg Truncate Phase：离线重组清除阶段     |
|                   |               |  -   Offline Reorg Copy Back Phase：离线重组重入阶段    |
|                   |               |  -   Offline Reorg Rebuild Phase：离线重组重建索引阶段  |
+-------------------+---------------+---------------------------------------------------------+

###协调节点字段信息###

  字段名   类型     描述
  -------- -------- ------------
  Name     字符串   集合完整名

###非协调节点示例###

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_COLLECTIONS)
{
  "Name": "foo.test",
  "Details": [
    {
      "ID": 0,
      "Logical ID": 0,
      "Sequence": 1,
      "Indexes": 2,
      "Status": "Normal"
    }
  ]
}</pre>

###协调节点示例###

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_COLLECTIONS)
{
  "Name": "foo.bar"
}</pre>

##集合空间列表##

###描述###

集合空间列表列出当前数据库节点中所有的集合空间，每个集合空间为一条记录。

###标示###

SDB_LIST_COLLECTIONSPACES

###字段信息###

由于数据节点与编目节点保存的集合空间信息不同，集合空间列表在协调节点与其它节点所返回的结构有所不同：

###非协调节点字段信息###

  字段名       类型         描述
  ------------ ------------ ----------------------------
  Name         字符串       集合空间名
  Collection   字符串数组   集合空间中所包含的所有集合
  PageSize     整型         集合空间数据页大小

###协调节点字段信息###

  字段名   类型     描述
  -------- -------- ------------
  Name     字符串   集合空间名

###非协调节点示例###

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_COLLECTIONSPACES)
{
  "Collection": [
    {
      "Name": "test"
    }
  ],
  "Name": "foo",
  "PageSize": 4096
}</pre>

###协调节点示例###

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_COLLECTIONSPACES)
{
  "Name": "foo"
}</pre>

##存储单元列表##

###描述###

存储单元列表列出当前数据库节点的全部存储单元信息。

###标示###

SDB_LIST_STORAGEUNITS

###字段信息###

  字段名           类型     描述
  ---------------- -------- -----------------------------------------------------------------------------
  Name             字符串   集合空间名
  ID               整型     该集合空间 ID
  LogicalID        字符串   集合空间逻辑 ID，为递增顺序
  PageSize         整型     集合空间数据页大小
  Sequence         整型     序列号，当前版本中为1
  NumCollections   整型     集合空间下的集合个数
  CollectionHWM    整型     集合高水位，一般来说意味着该集合空间中总共创建过的集合数量（包括被删除的集合）
  Size             长整型   存储单元大小（字节）

###示例###

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_STORAGEUNITS)
{
  "Name": "testCS",
  "ID": 4095,
  "LogicalID": 0,
  "PageSize": 4096,
  "Sequence": 1,
  "NumCollections": 1,
  "CollectionHWM": 1,
  "Size": 172032000
}
{
  "Name": "foo",
  "ID": 4094,
  "LogicalID": 1,
  "PageSize": 4096,
  "Sequence": 1,
  "NumCollections": 2,
  "CollectionHWM": 3,
  "Size": 172032000
}</pre>


##分区组列表##

###描述###

分区组列表列出当前集群中的所有分区信息。

###标示###

SDB_LIST_GROUPS

###字段信息###

+--------------------+---------------+----------------------------------------------------------------------+
| 字段名             | 类型          | 描述                                                                 |
+====================+===============+======================================================================+
| Group.dbpath       | 字符串        | 分区组中节点的数据文件存放路径                                       |
+--------------------+---------------+----------------------------------------------------------------------+
| Group.HostName     | 字符串        | 分区组中节点的主机名                                                 |
+--------------------+---------------+----------------------------------------------------------------------+
| Group.Service.Type | 整型          | 分区组中节点的服务类型                                               |
|                    |               |                                                                      |
|                    |               |  -   0：直连服务，对应数据库参数 svcname                             |
|                    |               |	-   1：复制服务，对应数据库参数 replname                            |
|                    |               |	-   2：分区服务，对应数据库参数 shardname                           |
|                    |               |	-   3：编目服务，对应数据库参数 catalogname                         |
+--------------------+---------------+----------------------------------------------------------------------+
| Group.Service.Name | 字符串        | 分区组中节点的服务名，服务名可以为端口号，或 services 文件中的服务名 |
+--------------------+---------------+----------------------------------------------------------------------+
| Group.NodeID       | 整型          | 分区组中节点的 ID                                                    |
+--------------------+---------------+----------------------------------------------------------------------+
| GroupID            | 整型          | 分区组 ID                                                            |
+--------------------+---------------+----------------------------------------------------------------------+
| GroupName          | 字符串        | 分区组名称                                                           |
+--------------------+---------------+----------------------------------------------------------------------+
| PrimaryNode        | 整型          | 主节点 ID                                                            |
+--------------------+---------------+----------------------------------------------------------------------+
| Role               | 整型          | 分区组角色，可以为：                                                 |
|                    |               |                                                                      |
|                    |               |  -   0：数据节点                                                     |
|                    |               |  -   2：编目节点                                                     |
+--------------------+---------------+----------------------------------------------------------------------+
| Status             | 字符串        | 分区组状态                                                           |
|                    |               |                                                                      |
|                    |               |  -   1：已激活分区组                                                 |
|                    |               |  -   0：未激活分区组                                                 |
|                    |               |  -   不存在：未激活分区组                                            |
+--------------------+---------------+----------------------------------------------------------------------+
| Version            | 整型          |                                                                      |
+--------------------+---------------+----------------------------------------------------------------------+

###示例###

<pre class="prettyprint lang-javascript">
db.list(SDB_LIST_GROUPS)
{
  "Group":[
    {
      "dbpath": "/home/users/chenzichuan/sequoiadb/cata",
      "HostName": "ubuntu-dev2",
      "Service": [
        {
          "Type": 0,
          "Name": "11800"
        },
        {
          "Type": 1,
          "Name": "11801"
        },
        {
          "Type": 2,
          "Name": "11802"
        },
        {
          "Type": 3,
          "Name": "11803"
        }
      ],
      "NodeID": 1
    }
  ],
  "GroupID": 1,
  "GroupName": "SYSCatalogGroup",
  "PrimaryNode": 1,
  "Role": 2,
  "Status": 1,
  "Version": 1,
  "_id": {
    "$oid": "51710981d8cb8fbc163d6350"
  }
}</pre>
用户可以使用 snapshot 监控每个节点的状态。

1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost", 11810) ;</pre>

2.得到分区组

<pre class="prettyprint lang-javascript">
> datarg = db.getRG ( "< datagroup1 >" ) ;</pre>

3.得到数据节点

<pre class="prettyprint lang-javascript">
> datanode = datarg.getNode ( "< hostname1 >", "< servicename1 >" ) ;</pre>

4.得到该节点的快照

<pre class="prettyprint lang-javascript">
> datanode.connect().snapshot(SDB_SNAP_DATABASE)</pre>

快照类型分为：

[SDB_SNAP_CONTEXTS](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_CONTEXTS_CURRENT](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_SESSIONS](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_SESSIONS_CURRENT](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_COLLECTIONS](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_COLLECTIONSPACES](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_DATABASE](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_SYSTEM](SdbDoc_Cn/database_management/monitoring/snapshot.html)

[SDB_SNAP_CATALOG](SdbDoc_Cn/database_management/monitoring/snapshot.html)

用户可以使用 Shell 脚本监控，例如：

<pre class="prettyprint lang-javascript">
[sequoiadb@vmsvr1-rhel-x64 sequoiadb]$ cat monitor_insert.sh
#!/bin/bash
~/sequoiadb/bin/sdb "db=new Sdb('localhost', 11810)" > /dev/null
~/sequoiadb/bin/sdb "db.getRG('foo').getNode('vmsvr1-rhel-x64',11820).connect().snapshot(SDB_SNAP_DATABASE)" | grep TotalInsert
~/sequoiadb/bin/sdb "quit"
[sequoiadb@vmsvr1-rhel-x64 sequoiadb]$ ./monitor_insert.sh
"TotalInsert": 0,</pre>
1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost", 11810) ;</pre>

2.得到集群状态

<pre class="prettyprint lang-javascript">
> db.listReplicaGroups()
{
  "Group": [
    {
      "dbpath": "/opt/sequoiadb/database/cata/11800",
      "HostName": "vmsvr1",
      "Service": [
        {
          "Type": 0,
          "Name": "11800"
        },
        {
          "Type": 1,
          "Name": "11801"
        },
        {
          "Type": 2,
          "Name": "11802"
        },
        {
          "Type": 3,
          "Name": "11803"
        }
      ],
      "NodeID": 1
    },
    {
      "HostName": "vmsvr2",
      "dbpath": "/opt/sequoiadb/database/cata/11800",
      "Service": [
        {
          "Type": 0,
          "Name": "11800"
        },
        {
          "Type": 1,
          "Name": "11801"
        },
        {
          "Type": 2,
          "Name": "11802"
        },
        {
          "Type": 3,
          "Name": "11803"
        }
      ],
      "NodeID": 2
    },
    {
      "HostName": "vmsvr3",
      "dbpath": "/opt/sequoiadb/database/cata/11800",
      "Service": [
        {
          "Type": 0,
          "Name": "11800"
        },
        {
          "Type": 1,
          "Name": "11801"
        },
        {
          "Type": 2,
          "Name": "11802"
        },
        {
          "Type": 3,
          "Name": "11803"
        }
      ],
      "NodeID": 3
    }
  ],
  "GroupID": 1,
  "GroupName": "SYSCatalogGroup",
  "PrimaryNode": 1,
  "Role": 2,
  "Status": 1,
  "Version": 3,
  "_id": {
    "$oid": "558b9264de349a1b87451a1d"
  }
}
{
  "Group": [
    {
      "HostName": "vmsvr1",
      "dbpath": "/opt/sequoiadb/database/data/21100",
      "Service": [
        {
          "Type": 0,
          "Name": "21100"
        },
        {
          "Type": 1,
          "Name": "21101"
        },
        {
          "Type": 2,
          "Name": "21102"
        }
      ],
      "NodeID": 1000
    },
    {
      "HostName": "vmsvr2",
      "dbpath": "/opt/sequoiadb/database/data/21100",
      "Service": [
        {
          "Type": 0,
          "Name": "21100"
        },
        {
          "Type": 1,
          "Name": "21101"
        },
        {
          "Type": 2,
          "Name": "21102"
        }
      ],
      "NodeID": 1001
    },
    {
      "HostName": "vmsvr3",
      "dbpath": "/opt/sequoiadb/database/data/21100",
      "Service": [
        {
          "Type": 0,
          "Name": "21100"
        },
        {
          "Type": 1,
          "Name": "21101"
        },
        {
          "Type": 2,
          "Name": "21102"
        }
      ],
      "NodeID": 1002
    }
  ],
  "GroupID": 1000,
  "GroupName": "group1",
  "PrimaryNode": 1001,
  "Role": 0,
  "Status": 1,
  "Version": 4,
  "_id": {
    "$oid": "558b9295de349a1b87451a21"
  }
}
{
  "Group": [
    {
      "HostName": "vmsvr3",
      "dbpath": "/opt/sequoiadb/database/data/22100",
      "Service": [
        {
          "Type": 0,
          "Name": "22100"
        },
        {
          "Type": 1,
          "Name": "22101"
        },
        {
          "Type": 2,
          "Name": "22102"
        }
      ],
      "NodeID": 1003
    },
    {
      "HostName": "vmsvr1",
      "dbpath": "/opt/sequoiadb/database/data/22100",
      "Service": [
        {
          "Type": 0,
          "Name": "22100"
        },
        {
          "Type": 1,
          "Name": "22101"
        },
        {
          "Type": 2,
          "Name": "22102"
        }
      ],
      "NodeID": 1004
    },
    {
      "HostName": "vmsvr2",
      "dbpath": "/opt/sequoiadb/database/data/22100",
      "Service": [
        {
          "Type": 0,
          "Name": "22100"
        },
        {
          "Type": 1,
          "Name": "22101"
        },
        {
          "Type": 2,
          "Name": "22102"
        }
      ],
      "NodeID": 1005
    }
  ],
  "GroupID": 1001,
  "GroupName": "group2",
  "PrimaryNode": 1004,
  "Role": 0,
  "Status": 1,
  "Version": 4,
  "_id": {
    "$oid": "558b92b3de349a1b87451a22"
  }
}
Return 3 row(s).
Takes 0.12157s.</pre>
##概要说明##

可以通过安全功能来指定登陆系统的权限。如果没有合法的用户名及密码则无法访问数据库。

##详细说明##

1.系统启动时默关闭安全功能。只有在至少创建一个用户后，安全功能才自动激活。

2.创建用户需要用户指定用户名及密码。用户名全系统唯一。

3.删除用户需要用户指定用户名及密码。

4.安全功能激活后，访问数据库需要指定用户名及密码。在一次会话中进行一次登录验证。开启新的会话需要重新进行验证。

5.用户之间属于平级关系。没有超级用户的概念。用户可以创建删除任意用户。

6.当删除所有用户后，安全功能自动关闭。

7.所有密码均以密文形式传输，存储。
SequoiaDB 通过配置可以支持 SSL。SequoiaDB 客户端和 SequoiaDB 实例直接可以使用 SSL 加密连接。

##SequoiaDB 支持##

若要使用 SSL 加密连接，需要 SequoiaDB 1.12 或之后的版本。

目前该特性仅限于 SequoiaDB 企业版，社区版暂不支持。

##客户端支持##

所有官方支持的客户端驱动都支持 SSL，包括 C、C++、Java、Python、C#、PHP、REST API 及 SDB Shell。

##配置 SequoiaDB 使用 SSL##

在安装部署时，通过配置参数开启 SequoiaDB 对 SSL 连接的支持：

--usessl，默认值为 false，设为 true 时开启 SSL，允许客户端通过 SSL 加密连接，同时仍然接受非 SSL 加密连接。

参考如下：

snapshot
sequoiadb --usessl=true</pre>

也支持使用配置文件配置，参见[数据库配置](SdbDoc_Cn/database_management/runtime_configuration.html)。

SequoiaDB 在开启 SSL 后会自动创建证书，不需要用户指定。

##客户端使用 SSL##

客户端必须与开启 SSL 的 SequoiaDB 配合才能使用 SSL加密连接。所有官方支持的客户端驱动都支持 SSL。

-   C

    C 驱动接口使用 sdbSecureConnect() 和 sdbSecureConnect1() 建立 SSL连接，使用方式与 sdbConnect() 和 sdbConnect1() 相同。

    参见 [C 驱动](SdbDoc_Cn/driver/c_driver/c_driver.html)。

-   C++

    C++ 驱动中类 sdb 的构造函数有参数 useSSL，设为 true 时使用 SSL 连接。

    参见 [C++驱动](SdbDoc_Cn/driver/cpp_driver/cpp_driver.html)。

-   Java

    Java 驱动中类 com.sequoiadb.net.ConfigOptions 有接口 setUseSSL(boolean useSSL)，设为 true 时使用 SSL 连接。

    参见 [Java驱动](SdbDoc_Cn/driver/java_driver/java_driver.html)。

-   Python

    Python 驱动中类 client 构造函数有可选参数 ssl，设为 true 时使用 SSL连接。

    参见 [Python驱动](SdbDoc_Cn/driver/python_driver/python_driver.html)。

-   C#

    C# 驱动中类 SequoiaDB.ConfigOptions 有属性 UseSSL，设为 true 时使用SSL 连接。

    参见 [C#驱动](SdbDoc_Cn/driver/csharp_driver/csharp_driver.html)。

-   PHP

    PHP 驱动中有类 SecureSdb，该类是 SequoiaDB 的子类，类 SecureSdb的对象使用 SSL 连接。

    参见 [PHP驱动](SdbDoc_Cn/driver/php_driver/php_driver.html)。

-   REST API

    REST API 支持 https。

    参见 [REST接口](SdbDoc_Cn/driver/rest/overview.html)。

-   sdb shell

    sdb shell 中共有类 SecureSdb，该类是 Sdb 的子类，类 SecureSdb的对象使用 SSL 连接。


##工具支持##

sdbexprt、sdbimprt、sdblobtool、sdbtop 支持 SSL连接。

参见：[sdbexprt](SdbDoc_Cn/database_management/tools/data_migration_tool.html),[sdbimprt](SdbDoc_Cn/database_management/tools/data_migration_tool.html),[sdblobtool](SdbDoc_Cn/database_management/tools/sdblob.html),[sdbtop](SdbDoc_Cn/database_management/tools/sdbtop.html)。
SequoiaDB 与 Hadoop 在物理上部署方案简易如下图所示，部署建议如下：

 * SequoiaDB 与 Hadoop 部署在相同的物理设备上，以减少 Hadoop 与 SequoiaDB 之间的网络数据传输

 * 每个物理设备上都部署一个协调节点和多个数据节点，编目节点可选在任意三台物理设备各部署一个编目节点

![](hadoop.jpg)
## 搭建 Hadoop 环境##

我们支持 hadoop 1.x 和 hadoop 2.x。先安装配置好 Hadoop

## 配置连接环境##

与 MapReduce 对接，需要准备 hadoop-connector.jar 和 sequoiadb.jar，这两个 jar 可以在 SequoiaDB 安装目录下面的 hadoop 目录中找到。

因为不同版本的 Hadoop 的 classpath 不一样，所以先查看 hadoop 的 classpath，输入 hadoop classpath，在classpath 中选择一个目录，把 hadoop-connector.jar 和 sequoiadb.jar 放在目录里面，重启 hadoop 集群。

## 编写 MapReduce##

**hadoop-connector.jar 中一些重要的类：**

SequoiadbInputFormat：读取SequoiaDB的数据

SequoiadbOutputFormat：向SequoiaDB中写入数据

BSONWritable：BSONObject 的包装类，实现了 WritableComparable 接口。用于序列化 BSONObject 对象。

**SequoiaDB 和 MapReduce 的配置：**

sequoiadb-hadoop.xml 是配置文件，放在你编写的 MapReduce 工程的源码根目录下面。

sequoiadb.input.url：指定作为输入的 SequoiaDB 的 URL 路径，格式为：hostname1:port1,hostname2:port2,

sequoiadb.input.user：指定输入源的 SequoiaDB 用户，默认为 null。

sequoiadb.input.passwd：指定输入源的 SequoiaDB 连接密码，默认为 null。

sequoiadb.in.collectionspace：指定作为输入的集合空间。

sequoiadb.in.collection：指定作为输入的集合。

sequoiadb.query.json：指定输入源的查询条件，使用 json 结构，默认为 null。

sequoiadb.selector.json：指定输入源的字段筛选，使用 json 结构，默认为 null。

sequoiadb.preferedinstance：指定从 SequoiaDB 中获取数据时，连接哪个数据节点，默认为 anyone，可填值：[slave/master/anyone/node(1-7)]。

sequoiadb.output.url：指定作为输出的 SequoiaDB 的 URL 路径。

sequoiadb.output.user：指定输出源的 SequoiaDB 用户，默认为 null。

sequoiadb.output.passwd：指定输出源的 SequoiaDB 连接密码，默认为 null。

sequoiadb.out.collectionspace：指定作为输出的集合空间。

sequoiadb.out.collection：指定作为输出的集合。

sequoiadb.out.bulknum：指定每次向 SequoiaDB 写入的记录条数，对写入性能进行优化。

## 示例

（1）下面是读取 HDFS 文件，处理后写入到 SequoiaDB 中去：
<pre class="prettyprint lang-javascript">
public class HdfsSequoiadbMR {
    static class MobileMapper extends  Mapper&lt;LongWritable,Text,Text,IntWritable&gt;{
        private static final IntWritable ONE=new IntWritable(1);
        @Override
        protected void map(LongWritable key, Text value, Context context)
                throws IOException, InterruptedException {
            String valueStr=value.toString();

            String mobile_prefix=valueStr.split(",")[3].substring(0,3);
            context.write(new Text(mobile_prefix), ONE);
        }

    }

    static class MobileReducer extends Reducer&lt;Text, IntWritable, NullWritable, BSONWritable&gt;{

        @Override
        protected void reduce(Text key, Iterable&lt;IntWritable&gt; values,Context context)
                throws IOException, InterruptedException {
                Iterator&lt;IntWritable&gt; iterator=values.iterator();
                long sum=0;
                while(iterator.hasNext()){
                    sum+=iterator.next().get();
                }
                BSONObject bson=new BasicBSONObject();
                bson.put("prefix", key.toString());
                bson.put("count", sum);
                context.write(null,new BSONWritable(bson));
        }

    }

    public static void main(String[] args) throws IOException, InterruptedException, ClassNotFoundException {
        if(args.length &lt; 1){
            System.out.print("please set input path ");
            System.exit(1);
        }
        Configuration conf=new Configuration();
        conf.addResource("sequoiadb-hadoop.xml"); //加载配置文件
        Job job=Job.getInstance(conf);
        job.setJarByClass(HdfsSequoiadbMR.class);
        job.setJobName("HdfsSequoiadbMR");
        job.setInputFormatClass(TextInputFormat.class);
        job.setOutputFormatClass(SequoiadbOutputFormat.class); //reduce 输出写入到 SequoiaDB 中
        TextInputFormat.setInputPaths(job, new Path(args[0]));

        job.setMapperClass(MobileMapper.class);
        job.setReducerClass(MobileReducer.class);

        job.setMapOutputKeyClass(Text.class);
        job.setMapOutputValueClass(IntWritable.class);

        job.setOutputKeyClass(NullWritable.class);
        job.setOutputValueClass(BSONWritable.class);

        job.waitForCompletion(true);
    }
}</pre>

（2） 读取 SequoiaDB 中数据处理后写入到 HDFS 中。
<pre class="prettyprint lang-javascript">
public class SequoiadbHdfsMR {
    static class ProvinceMapper extends Mapper&lt;Object, BSONWritable,IntWritable,IntWritable&gt;{
        private static final IntWritable ONE=new IntWritable(1);
        @Override
        protected void map(Object key, BSONWritable value, Context context)
                throws IOException, InterruptedException {
                       BSONObject obj = value.getBson();
            int province=(Integer) obj.get("province_code");
            context.write(new IntWritable(province), ONE);
        }

    }

    static class ProvinceReducer extends Reducer&lt;IntWritable,IntWritable,IntWritable,LongWritable&gt;{

        @Override
        protected void reduce(IntWritable key, Iterable&lt;IntWritable&gt; values,
                Context context)
                throws IOException, InterruptedException {
            Iterator&lt;IntWritable&gt; iterator=values.iterator();
            long sum=0;
            while(iterator.hasNext()){
                sum+=iterator.next().get();
            }
            context.write(key,new LongWritable(sum));
        }

    }


    public static void main(String[] args) throws IOException, InterruptedException, ClassNotFoundException {
        if(args.length&lt;1){
            System.out.print("please set  output path ");
            System.exit(1);
        }
        Configuration conf=new Configuration();
        conf.addResource("sequoiadb-hadoop.xml");
        Job job=Job.getInstance(conf);
        job.setJarByClass(SequoiadbHdfsMR.class);
        job.setJobName("SequoiadbHdfsMR");
        job.setInputFormatClass(SequoiadbInputFormat.class);
        job.setOutputFormatClass(TextOutputFormat.class);


        FileOutputFormat.setOutputPath(job, new Path(args[0]+"/result"));

        job.setMapperClass(ProvinceMapper.class);
        job.setReducerClass(ProvinceReducer.class);

        job.setMapOutputKeyClass(IntWritable.class);
        job.setMapOutputValueClass(IntWritable.class);

        job.setOutputKeyClass(IntWritable.class);
        job.setOutputValueClass(LongWritable.class);

        job.waitForCompletion(true);
    }
}</pre>

**配置信息：**
<pre class="prettyprint lang-diy">
&lt;?xml version="1.0" encoding="UTF-8"?&gt;
&lt;configuration&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.input.url&lt;/name&gt;
     &lt;value&gt;localhost:11810&lt;/value&gt;
  &lt;/property&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.output.url&lt;/name&gt;
     &lt;value&gt;localhost:11810&lt;/value&gt;
  &lt;/property&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.in.collectionspace&lt;/name&gt;
     &lt;value&gt;default&lt;/value&gt;
  &lt;/property&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.in.collect&lt;/name&gt;
     &lt;value&gt;student&lt;/value&gt;
  &lt;/property&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.out.collectionspace&lt;/name&gt;
     &lt;value&gt;default&lt;/value&gt;
  &lt;/property&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.out.collect&lt;/name&gt;
     &lt;value&gt;result&lt;/value&gt;
  &lt;/property&gt;
    &lt;property&gt;
     &lt;name&gt;sequoiadb.out.bulknum&lt;/name&gt;
     &lt;value&gt;10&lt;/value&gt;
  &lt;/property&gt;
&lt;/configuration&gt;</pre>
##SequoiaDB 支持的 Hive 版本列表##

 > * Apache Hive 0.12.0
 > * Apache Hive 0.11.0
 > * Apache Hive 0.10.0
 > * CDH-5.0.0-beta-2 Hive 0.12.0

**Note:**

（1）hive-sequoiadb-apache.jar 为支持 Apache 版 Hive 的 SequoiaDB-Hive-Connector

（2）hive-sequoiadb-cdh-5.0.0-beta-2.jar 为支持 CDH5.0.0-beta-2 版本下 Hive-0.12 的 SequoiaDB-Hive-Connector

##配置方法##

（1） 安装和配置好 Hadoop/Hive 环境，启动 Hadoop 环境；

（2） 拷贝 SequoiaDB 安装目录下（默认在 /opt/sequoiadb）的 hadoop/hive-sequoiadb-{version}.jar 和 java/sequoiadb.jar 两个文件拷贝到 hive/lib 安装目录下；

（3） 修改 Hive 安装目录下的 bin/hive-site.xml 文件（如果不存在，可拷贝 $HIVE_HOME/conf/hive-default.xml.template为 hive-site.xml 文件），增加如下属性：
<pre class="prettyprint lang-diy">
&lt;property&gt;
  &lt;name&gt;hive.aux.jars.path&lt;/name&gt;
  &lt;value&gt;file://&lt;HIVE_home&gt;/lib/hive-sequoiadb-{version}.jar,file://&lt;HIVE_HOME&gt;/lib/sequoiadb.jar&lt;/value&gt;
  &lt;description&gt;Sequoiadb store handler jar file&lt;/description&gt;
&lt;/property&gt;

&lt;property&gt;
  &lt;name&gt; hive.auto.convert.join&lt;/name&gt;
  &lt;value&gt;false&lt;/value&gt;
&lt;/property&gt;</pre>

**Note:**
需要把这些 jar 包存放 HDFS 上，地址和 file 协议的地址一样。
启动 Hive Shell 命令行窗口，执行如下命令创建数据表；
<pre class="prettyprint lang-javascript">
hive> create external table sdb_tab(id INT, name STRING, value DOUBLE) stored by "com.sequoiadb.hive.SdbHiveStorageHandler" tblproperties("sdb.address" = "localhost:11810");

OK
Time taken: 0.386 seconds</pre>

其中 sdb.address 用于指定 SequoiaDB 协调节点的 IP 和端口，如果有多个协调节点，可以写入多个，之间用逗号隔开。表的数据库对应 SequoiaDB 的集合空间，表对应集合空间中的集合。
##从 HDFS 文件中导入数据到 SequoiaDB 表##
<pre class="prettyprint lang-javascript">
hive> insert overwrite table sdb_tab select * from hdfs_tab;

Total MapReduce jobs = 1
Launching Job 1 out of 1
Number of reduce tasks is set to 0 since there's no reduce operator
Starting Job = job_201310172156_0010, Tracking URL = http://bl465-5:50030/jobdetails.jsp?jobid=job_201310172156_0010
Kill Command = /opt/hadoop-hive/hadoop-1.2.1/libexec/../bin/hadoop job  -kill job_201310172156_0010
Hadoop job information for Stage-0: number of mappers: 1; number of reducers: 0
2013-10-18 04:44:47,733 Stage-0 map = 0%,  reduce = 0%
2013-10-18 04:44:49,763 Stage-0 map = 100%,  reduce = 0%, Cumulative CPU 1.85 sec
2013-10-18 04:44:50,777 Stage-0 map = 100%,  reduce = 0%, Cumulative CPU 1.85 sec
2013-10-18 04:44:51,795 Stage-0 map = 100%,  reduce = 100%, Cumulative CPU 1.85 sec
MapReduce Total cumulative CPU time: 1 seconds 850 msec
Ended Job = job_201310172156_0010
10 Rows loaded to sdb_tab
MapReduce Jobs Launched:
Job 0: Map: 1   Cumulative CPU: 1.85 sec   HDFS Read: 2301 HDFS Write: 0 SUCCESS
Total MapReduce CPU Time Spent: 1 seconds 850 msec
OK
Time taken: 12.201 seconds</pre>

**Note:**
在导入数据到 SequoiaDB 表之前，请确保已经创建基于 HDFS 文件的 hdfs_tab 数据表，并 load 了数据。
## 查询数据##
<pre class="prettyprint lang-javascript">
hive> select * from new_tab;
OK
0       false   0.0     ALGERIA
1       true    1.0     ARGENTINA
2       true    1.0     BRAZIL
3       true    1.0     CANADA
4       true    4.0     EGYPT
5       false   0.0     ETHIOPIA
6       true    3.0     FRANCE
7       true    3.0     GERMANY
8       true    2.0     INDIA
9       true    2.0     INDONESIA
Time taken: 0.306 seconds, Fetched: 10 row(s)</pre>
虽然 Hadoop 占据了大部分的分布式数据分析市场，如今也有其他的平台提供了
Hadoop 平台所没有的许多特性。

Apache 的 Spark 是一个高速的通用集群式计算系统。Spark
是一个可扩展的数据分析平台，该平台集成了原生的内存计算，因此它在使用中相比
Hadoop 的集群存储来说，会有不少的性能优势。

Apache Spark 提供了高级的 Java，Scala 和 Python APIs,
同时还拥有优化的引擎来支持常用的执行图。Spark
还支持多样化的高级工具，其中包括了处理结构化数据和 SQL 的 Spark
SQL，处理机器学习的 MLlib，图形处理的 GraphX，还有 SparkStreaming。

虽然 Spark 最初是为了分布式数据集的迭代工作而设计的，但现在 Spark
已经几乎完全拥有了 Hadoop 的所有功能，并且可以基于 Hadoop
文件系统（HDFS）使用。集群模式中，Spark 有几种不同的运行模式，包括了
YARN、Mesos 还有 Standalone。

##Spark 组成##

在集群中，Spark 应用以独立的进程集合的方式运行，并由主程序（driver
program）中的 SparkContext
对象进行统一的调度。当需要在集群上运行时，SparkContext
会连接到几个不同类的 ClusterManager（集群管理器）上（Spark 自己的
Standalone/Mesos/YARN）, 集群管理器将给各个应用分配资源。连接成功后，Spark
会请求集群各个节点的
Executor（执行器），它是为应用执行计算和存储数据的进程的总称。之后，Spark
会将应用提供的代码（应用已经提交给 SparkContext 的 JAR 或 Python
文件）交给 executor。最后，由 SparkContext 发送 tasks 提供给其执行。

![](spark_components_en.jpg)

关于这个架构的几点介绍：

1.  每一个应用有其独立的 Executor
    进程，这些进程将会在应用整个生命周期内为应用服务，并且会在多个线程中执行任务
    tasks。这种做法能有效的隔离不同的应用，在调度和执行端都能很好的隔离（每个驱动调度自己的任务，不同的任务在不懂的
    JVM
    中执行）。但是，这也意味着，如果不写入外部的存储设备，那数据就不能在不同的
    Spark 应用（SparkContext 实例）之中共享。
2.  Spark 对于下列的集群管理者是不可知的：只要 Spark 能请求 executor
    进程，且这些进程之间能互相通信，那么他就相对容易的去运行支持其他应用的集群管理器（如
    Mesos/YARN）。
3.  因为驱动在集群中调度任务，它将在 worker
    nodes（工作节点）附近运行，最好是在相同的局域网当中。如果你不喜欢远程向集群发送请求，那么最好为驱动打开一个
    RPC 然后让其在附近提交操作而不是在远离 worker nodes 处运行驱动。
##Spark 安装##

首先，你需要获取最新版本的 [Spark framework](http://spark.apache.org/downloads.html)。

以 Spark1.3+Hadoop 2.4为例，下载文件名为“spark-1.3.0-bin-hadoop2.4.tgz”。

![](spark_install_manual_step1_en.jpg)

安装包可以通过以下指令获取：

<pre class="prettyprint lang-javascript">
$ tar –zxvf spark-1.3.0-bin-hadoop2.4.tgz –directory /opt/</pre>

当安装包文件获取后，你可以 cd 至他的目录下，使用 spark-shell
进行一个简单的试用：

<pre class="prettyprint lang-javascript">
$ cd /opt/spark-1.3.0-bin-hadoop2.4
$ bin/spark-shell 2>/dev/null
Welcome to
      ____              __
     / __/__  ___ _____/ /__
    _\ \/ _ \/ _ `/ __/  '_/
   /___/ .__/\_,_/_/ /_/\_\   version 1.3.0
      /_/

Using Scala version 2.10.4 (Java HotSpot(TM) 64-Bit Server VM, Java 1.6.0_35)
Type in expressions to have them evaluated.
Type :help for more information.
Spark context available as sc.
SQL context available as sqlContext.
scala> val textFile = sc.textFile("README.md")
textFile: org.apache.spark.rdd.RDD[String] = README.md MapPartitionsRDD[1] at textFile at < console >:21
scala> textFile.count()
res0: Long = 98
scala> textFile.first()
res1: String = # Apache Spark
scala> textFile.filter(line => line.contains("Spark")).count()
res2: Long = 19</pre>

为了建立一个高可用的集群，集群至少需要3个物理主机。在这个例子中，我们将其命名为“server1”，“server2”
和 “server3”。

下载和安装需要在集群中的这三个主机下各执行一次。

##Standalone 模式下配置 Spark##

当 Spark
安装后（安装路径假设为/opt/spark-1.3.0-bin-hadoop2.4），在部署为高可用的集群之前，还需要进行几部简单的操作。

1) 下载和配置 Apache ZooKeeper.

	[Apache ZooKeeper](http://www.apache.org/dyn/closer.cgi/zookeeper) 是一个管理配置信息和命名的中央集中的服务。它提供分布式的同步和群组的服务，而所有这些服务都将会被分布式的应用所使用。

	至少需要3个节点才能构成一个高可用的 ZooKeeper 集群。

	当安装包被下载到每个主机时，以下指令会自动获取安装文件：

	<pre class="prettyprint lang-javascript">
	$ tar –zxvf zookeeper-3.4.6.tar.gz –directory /opt/</pre>

2) 调整 ZooKeeper 配置

	<pre class="prettyprint lang-javascript">
	$ cd /opt/zookeeper-3.4.6/conf
	$ cp zoo_sample.cfg zoo.cfg
	$ vi zoo.cfg</pre>

	加入以下3行代码，并将 hostname 主机名替换为真实的主机地址：

	<pre class="prettyprint lang-diy">
	server.1=server1:2888:3888
	server.2=server2:2888:3888
	server.3=server3:2888:3888</pre>

	**Note:**

	请记得修改“dataDir”（不可使用/tmp），如“/data/zookeeper”。

	接着，在每个 server 创建 myid 文件。在这个例子中，可以针对每个 server 执行以下指令：

	<pre class="prettyprint lang-diy">
	Server1: echo "1" > /data/zookeeper/myid
	Server2: echo "2" > /data/zookeeper/myid
	Server3: echo "3" > /data/zookeeper/myid</pre>

	**Note:**

	请注意，id 数需要与 zoo.cfg 中的 server.x 配置参数一致。

3) 启动 ZooKeeper 服务

	以下指令可以在每个由 ZooKeeper 配置的主机执行：

	<pre class="prettyprint lang-javascript">
	$ bin/zkServer.sh start</pre>

	你需要在3个节点都启动 ZooKeeper 服务。使用以下指令查看 ZooKeeper
	服务的状态：

	<pre class="prettyprint lang-javascript">
	$ bin/zkServer.sh status</pre>

	其中的一个节点应该会显示“Mode: leader”，其余的节点显示“Mode: follower”。

4) 在 Standalone 模式下配置 Apache Spark

	Apache Spark 可以以不同模式运行。当结合 Spark 和 SequoiaDB 集群时，我们推荐使用 Spark 的 Standalone 模式。

	因为本地数据访问优先于远程数据查找的，推荐将 Apache Spark 安装在所有的 SequoiaDB 数据库集群。

	Apache Spark Standalone 模式的部署架构如下图：

	![](spark_install_manual_step2_en.jpg)

	Spark 集群中，可能存在多于一个的 Master
	节点，而其中只有一个会是“Primary”（主要）的，其余的都是“Standby”（预备）模式。ZooKeeper
	就被用于跟踪每个 Master 节点，确保时刻都有一个“Primary”节点存在。

	**Note:**

	推荐在每个 SequoiaDB 集群的主机中，运行一个 Worker 节点。

	SequoiaDB 和 Apache Spark 对接需要相应的驱动，登录 [https://oss.sonatype.org](https://oss.sonatype.org)，搜索 sequoiadb-driver 下载 SequoiaDB 最新的 java 驱动， 搜索 spark-sequoiadb 下载 SequoiaDB 最新的 spark 驱动，注意 scala 版本，然后把他们复制集群中的每个节点 $SPARK_HOME/lib 目录。

	Apache Spark 配置文件默认位于conf/spark-env.sh，假设 SequoiaDB 安装在 /opt/sequoiadb，你可以复制一个 spark-env.sh.template 后作出以下的配置修改：

	<pre class="prettyprint lang-diy">
	SPARK_DAEMON_JAVA_OPTS="-Dspark.deploy.recoveryMode=ZOOKEEPER -Dspark.deploy.zookeeper.url=server1:2181,server2:2181,server3:2181 -Dspark.deploy.zookeeper.dir=/spark"
	SPARK_WORKER_DIR=/data/spark_works
	SPARK_LOCAL_DIRS=/data/spark_data1,/data/spark_data2
	SPARK_CLASSPATH=$SPARK_HOME/lib/sequoiadb.jar:$SPARK_HOME/lib/spark-sequoiadb_2.10-1.12.0.jar</pre>

	**Note:**

	1. 替换 $SPARK_HOME 为 Spark 的绝对路径。

	2. spark.deploy.zookeeper.dir 配置的目录必须与其他主机上的配置一致。

	下列的配置参数是非必须的，其基于硬件和负载设置。若通过 SequoiaDB Administration Center 安装，那么 SequoiaDB 将会自动设置这些参数：

	<pre class="prettyprint lang-diy">
	SPARK_WORKER_MEMORY</pre>

5) 启动 Spark 的 master 节点

	如果你想启动 Master 节点，以下指令可在你需要启动 Master 的主机执行：

	<pre class="prettyprint lang-javascript">
	$ cd /opt/spark-1.3.0-bin-hadoop2.4
	$ sbin/start-master.sh</pre>

6) 启动 Spark 的 worker 节点

	因为我们推荐在每个 SequoiaDB 集群的主机下都运行 Worker 节点，你可以运行以下指令来启动每个主机的 Worker 节点：

	<pre class="prettyprint lang-javascript">
	$ cd /opt/spark-1.3.0-bin-hadoop2.4
	$ nohup bin/spark-class org.apache.spark.deploy.worker.Worker spark://serverA:7077,serverB:7077>logs/worker.out &</pre>
##开始使用 SparkSQL##

SparkSQL 是 Spark 下处理结构化数据执行的模块，它提供了名为 DataFrames 的程序抽象工具，同时他还能作为分布式的 SQL 查询引擎。

只要 Spark 的安装配置符合要求，通过 SequoiaDB 使用 SparkSQL 也是很简单的。

假设集合名为“test.data”，协调节点在 serverX 和 serverY 上，以下指令可以在 spark-shell 执行，并创建一个临时表来对应 SequoiaDB 的 Collection（集合）：

<pre class="prettyprint lang-javascript">
scala> sqlContext.sql("CREATE TEMPORARY TABLE datatable USING com.sequoiadb.spark OPTIONS ( host 'serverX:11810,serverY:11810', collectionspace 'test', collection 'data')")</pre>

除了特别定义的表模式，其将会扫描整个表同时根据每条记录的字段信息来构建表的模式。如果集合中的记录非常多，处理速度将会很慢。

另一种构建表的方式是使用 CREATE TABLE 指令来构建表模式：

<pre class="prettyprint lang-javascript">
scala> sqlContext.sql("CREATE temporary table datatable ( c1 string, c2 int, c3 int ) using com.sequoiadb.spark OPTIONS ( host 'serverX:11810,serverY:11810', collectionspace 'test', collection 'data')")</pre>

**Note:**

临时表只在它被创建的那一个 Session 期间有效，以下 query 查询可被用于获取表中的数据

<pre class="prettyprint lang-javascript">
scala> sqlContext.sql("select * from datatable").foreach(println)</pre>

##在 SparkSQL 使用 JDBC##

此处使用的 Thrift JDBC/ODBC 服务器对应着 Hive 0.12 的 HiveServer2。你可以用直线脚本在 Hive0.12 或者 Spark 测试 JDBC 服务器。

Spark 的镜像需要在选项-Phive,-Phivethriftserver 下配置。否则 sbin/start-thriftserver.sh 将会显示以下的错误信息：

<pre class="prettyprint lang-diy">
failed to launch org.apache.spark.sql.hive.thriftserver.HiveThriftServer2:
You need to build Spark with -Phive and -Phive-thriftserver.</pre>

需要启动 JDBC/ODBC server，请执行以下的 Spark 目录内容：

<pre class="prettyprint lang-javascript">
$ ./sbin/start-thriftserver.sh</pre>

此处的脚本接收所有 bin/spark-submit 的命令行选项，同时还有 --hiveconf 选项来置顶 Hive 属性。你可以执行 ./sbin/start-thriftserver.sh –help
获取所有可用的选项。服务器默认的监听端口为 localhost:10000 你可以使用以下任意环境变量来重写它：

<pre class="prettyprint lang-javascript">
$ export HIVE_SERVER2_THRIFT_PORT=&lt;listening-port&gt;
$ export HIVE_SERVER2_THRIFT_BIND_HOST=&lt;listening-host&gt;
$ ./sbin/start-thriftserver.sh   --master &lt;master-uri&gt;   
...</pre>

或是系统属性：

<pre class="prettyprint lang-javascript">
$ ./sbin/start-thriftserver.sh  --hiveconf hive.server2.thrift.port=&lt;listening-port&gt;   --hiveconf hive.server2.thrift.bind.host=&lt;listening-host&gt;   --master &lt;master-uri&gt;   
...</pre>

现在可使用直线脚本测试 Thrift JDBC/ODBC server:

<pre class="prettyprint lang-javascript">
$ ./bin/beeline</pre>

在直线脚本连接 JDBC/ODBC server in beeline :

<pre class="prettyprint lang-javascript">
beeline> !connect jdbc:hive2://localhost:10000</pre>

Beeline 直线脚本会询问用户名和密码。在非安全模式下，简单输入 username 和空白密码即可。在安全模式下，请按照 [beeline documentation](https://cwiki.apache.org/confluence/display/Hive/HiveServer2%20Clients) 下的说明来执行。

Hive 的配置将 hive-site.xml 文件移动到 conf 目录下

你也可以使用 Hive 自带的直线脚本。
PostgreSQL 是一款开源的关系型数据库，支持标准 SQL，用户可以通过 JDBC 驱动连接 PostgreSQL 进行应用程序开发。用户通过扩展 PostgreSQL 功能，让开发者可以使用 SQL 语句访问 SequoiaDB 数据库，完成 SequoiaDB 数据库的增、删、查、改操作。

##SequoiaDB所支持的 PostgreSQL 版本##

-   PostgreSQL 9.3.4
##安装 PostgreSQL##
1) 源码编译 PostgreSQL

	下载链接：[PostgreSQL](http://www.postgresql.org/ftp/source)

	解压后编译安装（需要 root 权限）

	<pre class="prettyprint lang-javascript">
	$ tar -zxvf postgresql-9.3.4.tar.gz
	$ cd postgresql-9.3.4/
	$ ./configure && make && make install</pre>

2) 切换用户

	<pre class="prettyprint lang-javascript">
	$su - sdbadmin</pre>

3) 拷贝 PostgreSQL 文件

	<pre class="prettyprint lang-javascript">
	$cp -rf /usr/local/pgsql ~/</pre>

4) 进入 PostgreSQL 目录

	<pre class="prettyprint lang-javascript">
	$cd pgsql</pre>

5) 环境变量添加 PostgreSQL 的 lib 库

	<pre class="prettyprint lang-javascript">
	$export LD_LIBRARY_PATH=$(pwd)/lib:${LD_LIBRARY_PATH}</pre>

	**Note:**

	建议用户将 PostgreSQL 的 lib 加到 sdbadmin 用户的环境变量中，否则每次登陆 sdbadmin 使用 PostgreSQL，都需要手工添加 PostgreSQL 的 lib 到 LD_LIBRARY_PATH中。

6) 创建 PostgreSQL 的数据目录

	<pre class="prettyprint lang-javascript">
	$mkdir pg_data</pre>

7) 初始化数据目录(该操作只能操作一次)

	<pre class="prettyprint lang-javascript">
	$bin/initdb -D pg_data/</pre>

##安装 SequoiaDB-PostgreSQL 插件##

1) 创建 PostgreSQL 的 lib 目录

	获取 PostgreSQL 的 libdir 路径

	<pre class="prettyprint lang-javascript">
	$ PGLIBDIR=$(bin/pg_config --libdir)</pre>

	如果显示的 libdir 目录不存在，则需要用户自己手工创建目录

	<pre class="prettyprint lang-javascript">
	$ mkdir -p ${PGLIBDIR}</pre>

2) 创建PostgreSQL的extension目录

	获取 PostgreSQL 的 sharedir 路径

	<pre class="prettyprint lang-javascript">
	$ PGSHAREDIR=$(bin/pg_config --sharedir)</pre>

	在 shardir 目录上再创建 extension 目录

	<pre class="prettyprint lang-javascript">
	$ mkdir -p ${PGSHAREDIR}/extension</pre>

3) 从 SequoiaDB 的安装包中，拷贝 PostgreSQL 的扩展文件

	从 SequoiaDB 安装后的 postgresql 目录中拷贝 sdb_fdw.so 文件到 PostgreSQL 的 lib 目录，SequoiaDB 默认安装目录为 /opt/sequoiadb。

	<pre class="prettyprint lang-javascript">
	$ cp -f /opt/sequoiadb/postgresql/sdb_fdw.so ${PGLIBDIR}</pre>

4) 将 sdb_fdw.control 和 sdb_fdw--1.0.sql 脚本拷贝到 extension 目录中：

	<pre class="prettyprint lang-javascript">
	$ cp -f /opt/sequoiadb/postgresql/sdb_fdw.control ${PGSHAREDIR}/extension/ ;
	$ cp -f /opt/sequoiadb/postgresql/sdb_fdw--1.0.sql ${PGSHAREDIR}/extension/ ;</pre>

	sdb_fdw.control 脚本内容：

	<pre class="prettyprint lang-diy">
	# sdb_fdw extension
	comment = 'foreign data wrapper for SequoiaDB access'
	default_version = '1.0'
	module_pathname = '$libdir/sdb_fdw'
	relocatable = true</pre>

	sdb_fdw--1.0.sql 脚本内容：

	<pre class="prettyprint lang-diy">
	/* contrib/sdb_fdw/sdb_fdw--1.0.sql */
	-- complain if script is sourced in psql, rather than via CREATE EXTENSION
	\echo Use "CREATE EXTENSION sdb_fdw" to load this file. \quit
	CREATE FUNCTION sdb_fdw_handler()
	RETURNS fdw_handler
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT;
	CREATE FUNCTION sdb_fdw_validator(text[], oid)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT;
	CREATE FOREIGN DATA WRAPPER sdb_fdw
	HANDLER sdb_fdw_handler
	VALIDATOR sdb_fdw_validator;</pre>

##部署 PostgreSQL##

1) 修改PostgreSQL的日志配置，日志中增加打印时间信息、连接信息等

	<pre class="prettyprint lang-javascript">
	$ vi pg_data/postgresql.conf </pre>

	打印连接信息
	log_connections = on

	打印断连信息
	log_disconnections = on 

	日志中打印时间，进程id，客户端地址信息
	log_line_prefix = '%m %p %r'

	出现错误时，断开当前连接
	exit_on_error = on

2) 检查端口是否被占用

	PostgreSQL 默认启动端口为”5432”,检查端口是否被占用(检查操作建议使用 root 用户操作，只有检查端口需要 root 权限，其余操作还是需要在 sdbadmin 用户下操作)

	<pre class="prettyprint lang-javascript">
	$netstat -nap | grep 5432</pre>

	如果5432端口被占用或者希望修改 PostgreSQL 的启动端口，则执行：

	<pre class="prettyprint lang-javascript">
	$ sed -i "s/#port = 5432/port = 11780/g" pg_data/postgresql.conf</pre>

3) 启动 Postgresql 服务进程（需要使用 sdbadmin 用户执行以下命令）

	<pre class="prettyprint lang-javascript">
	$ bin/postgres -D pg_data/ >> logfile 2>&1 &</pre>

4) 检查 PostgreSQL 是否启动成功

	<pre class="prettyprint lang-javascript">
	$ netstat -nap | grep 5432</pre>

	结果为：

	<pre class="prettyprint lang-diy">
	tcp   0   0 127.0.0.1:5432     0.0.0.0:*         LISTEN     20502/postgres
	unix  2   [ ACC ]   STREAM    LISTENING   40776754 20502/postgres     /tmp/.s.PGSQL.5432</pre>

5) 创建 PostgreSQL 的 database

	<pre class="prettyprint lang-javascript">
	$ bin/createdb -p 5432 foo</pre>

	进入 PostgreSQL shell 环境

	<pre class="prettyprint lang-javascript">
	$ bin/psql -p 5432 foo</pre>
以下操作均在PostgreSQL shell 环境下执行。

##PostgreSQL与SequoiaDB建立关联##

1) 加载SequoiaDB连接驱动

<pre class="prettyprint lang-javascript">
foo=# create extension sdb_fdw;</pre>

2) 配置与SequoiaDB连接参数

<pre class="prettyprint lang-javascript">
foo=# create server sdb_server foreign data wrapper sdb_fdw options(address '127.0.0.1', service '11810', user 'sdbadmin', password 'mypassword');</pre>

**Note:** 

如果没有配置数据库密码验证，可以忽略user与password字段。

3) 关联SequoiaDB的集合空间与集合

<pre class="prettyprint lang-javascript">
foo=# create foreign table test (name text, id numeric) server sdb_server options ( collectionspace 'chen', collection 'test' ) ;</pre>

**Note:**

集合空间与集合必须已经存在于SequoiaDB，否则查询出错。

默认情况下，表的字段映射到SequoiaDB中为小写字符，如果强制指定字段为大写字符，创建方式参考注意事项1。

映射SequoiaDB 的数组类型，创建方式参考注意事项2。

4) 更新表的统计信息

<pre class="prettyprint lang-javascript">
foo=# analyze test;</pre>

5) 查询

<pre class="prettyprint lang-javascript">
foo=# select * from test;</pre>

6) 写入数据

<pre class="prettyprint lang-javascript">
foo=# insert into test values('one',3);</pre>

7) 更改数据

<pre class="prettyprint lang-javascript">
foo=# update test set id=9 where name='one';</pre>

8) 查看所有的表(show tables;)

<pre class="prettyprint lang-javascript">
foo=# \d</pre>

9) 查看表的描述信息

<pre class="prettyprint lang-javascript">
foo=# \d test</pre>

10) 删除表的映射关系

<pre class="prettyprint lang-javascript">
foo=# drop foreign table test;</pre>

11) 退出PostgreSQL shell环境

<pre class="prettyprint lang-javascript">
foo=# \q</pre>

##使用须知##

1) 数据类型的对应关系

SequoiaDB         PostgreSQL          注意事项
----------------- ------------------- ---------------------------------------------
int               smallint            当SequoiaDB中的值超过smallint范围时会发生截断
int               integer
long              bigint
int               serial
long              bigserial
double            real                存在精度问题，sdb存储时不是完全一致
double            double precision
string            numeric
string            decimal
string            text
string            char
string            varchar
binary(type=0)    bytea
date              date
timestamp         timestamp
array             TYPE[]              仅支持一维数组
boolean           boolean
null              text

2) 注意事项

2.1) 注意字符的大小写

SequoiaDB 中的集合空间、集合和字段名均对字母的大小写敏感。

2.1.1) 集合空间、集合名大写

假设SequoiaDB 中存在名为TEST的集合空间，CHEN的集合，在PostgreSQL中建立相应的映射表：

<pre class="prettyprint lang-javascript">
foo=# create foreign table sdb_upcase_cs_cl (name text) server sdb_server options ( collectionspace 'TEST', collection 'CHEN' ) ;</pre>

2.1.2) 字段名大写

假设SequoiaDB 中存在名为foo的集合空间，bar的集合，而且保存的数据为：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid":"53a2a0e100e75e2c53000006"
  },
  "NAME": "test"
}</pre>

在PostgreSQL中建立相应的映射表：

<pre class="prettyprint lang-javascript">
foo=# create foreign table sdb_upcase_field ("NAME" text) server sdb_server options ( collectionspace 'foo', collection 'bar' ) ;</pre>

执行查询命令：

<pre class="prettyprint lang-javascript">
foo=# select * from sdb_upcase_field;</pre>

查询结果为：

<pre class="prettyprint lang-javascript">
NAME
------
test
(1 rows)</pre>

2.2) 映射SequoiaDB中的数据类型

假设SequoiaDB中存在foo集合空间，bar集合，保存记录为：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid":"53a2de926b4715450a000001"
  },
  "name": [
    1,
    2,
    3
    ],
  "id": 123
}</pre>

在 PostgreSQL 中建立相应的映射表：

<pre class="prettyprint lang-javascript">
foo=# create foreign table bartest (name int[], id int) server sdb_server options ( collectionspace 'foo', collection 'bar' ) ;</pre>

执行查询命令：

<pre class="prettyprint lang-javascript">
foo=# select * from bartest;</pre>

查询结果：

<pre class="prettyprint lang-diy">
name    | id
--------+-----
{1,2,3} | 123</pre>

2.3) 连接 SequoiaDB 协调节点错误

如果 PostgreSQL 连接的 SequoiaDB 协调节点重启，在查询时报错：

<pre class="prettyprint lang-diy">
ERROR: Unable to get collection "chen.test", rc = -15
HINT: Make sure the collectionspace and collection exist on the remote database</pre>

解决方法：

退出 PostgreSQL shell

<pre class="prettyprint lang-javascript">
foo=# \q</pre>

重新进入 PostgreSQL shell

<pre class="prettyprint lang-javascript">
bin/psql -p 5432 foo</pre>

##调整 PostgreSQL 配置文件##

1) 查看 pg_shell 中默认的配置：

<pre class="prettyprint lang-javascript">
foo=#\set
AUTOCOMMIT = 'on'
PROMPT1 = '%/%R%# '
PROMPT2 = '%/%R%# '
PROMPT3 = '>> '
VERBOSITY = 'default'
VERSION = 'PostgreSQL 9.3.4 on x86_64-unknown-linux-gnu, compiled by gcc (SUSE Linux) 4.3.4 [gcc-4_3-branch revision 152973], 64-bit'
DBNAME = 'foo'
USER = 'sdbadmin'
PORT = '5432'
ENCODING = 'UTF8'</pre>

2) 调整 pg_shell 查询时，每次获取记录数

<pre class="prettyprint lang-javascript">
foo=#\set FETCH_COUNT 100</pre>

Note:

调整为每次 ps_shell 每次获取100 条记录立即返回记录，然后再继续获取。

直接在 pg_shell 中修改配置文件，只能在当前 pg_shell 中生效，重新登录 pg_shell 需要重新设置。

3) 修改配置文件，调整 pg_shell 查询时，每次获取记录数

<pre class="prettyprint lang-javascript">
$ ${PG_HOME}/bin/pg_config --sysconfdir</pre>

结果为：

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/pgsql/etc</pre>

**Note:**

如果显示目录不存在，自己手动创建即可。

<pre class="prettyprint lang-javascript">
mkdir -p /opt/sequoiadb/pgsql/etc</pre>

将需要修改的参数写入配置文件中(需重启psql使配置生效)：

<pre class="prettyprint lang-javascript">
$ echo "\\set FETCH_COUNT 100" >> /opt/sequoiadb/pgsql/etc/psqlrc</pre>

4) 调整 pg\shell 的日志级别

<pre class="prettyprint lang-javascript">
$ sed -i 's/#client_min_messages = notice/client_min_messages = debug1/g' pg_data/postgresql.conf</pre>

5) 调整 pg 引擎的日志级别

<pre class="prettyprint lang-javascript">
$ sed -i 's/#log_min_messages = warning/log_min_messages = debug1/g' pg_data/postgresql.conf</pre>
##修改 PostgreSQL 的连接配置##

1) 修改 PostgreSQL 的监听地址

  <pre class="prettyprint lang-javascript">
  $ sed -i "s/#listen_addresses = 'localhost'/listen_addresses = '0.0.0.0'/g" pg_data/postgresql.conf</pre>

2) 修改信任的机器列表

  <pre class="prettyprint lang-javascript">
  $ linenum=$(cat -n pg_data/pg_hba.conf | grep "# IPv4 local connections:" | awk '{print $1}'); 
  $ let "linenum=linenum+1";varStr="host   all            all            0.0.0.0/0              trust"; 
  $ sed -i "${linenum} a${varStr}" pg_data/pg_hba.conf;</pre>

3) 重启 PostgreSQL

  <pre class="prettyprint lang-javascript">
  $ bin/pg_ctl stop -s -D pg_data/ -m fast; bin/postgres -D pg_data/ >> logfile 2>&1 &</pre>

##JDBC连接程序##

<pre class="prettyprint lang-javascript">
package com.sequoiadb.sample;
import java.sql.*;
public class postgresql_sample {
   static{
       try {
           Class.forName("org.postgresql.Driver");
       } catch (ClassNotFoundException e) {
           e.printStackTrace();
       }
   }
   public static void main( String[] args ) throws SQLException{
      String pghost = "192.168.30.182";
      String port = "5432";
      String databaseName = "foo";
      // postgresql process is running in which user
      String pgUser = "sdbadmin";
      String url = "jdbc:postgresql://"+pghost+":"+port+"/" + databaseName;
      Connection conn = DriverManager.getConnection(url, pgUser, null);
      Statement stmt = conn.createStatement();
      String sql = "select * from sdb_upcase_field ";
      ResultSet rs = stmt.executeQuery(sql);
      boolean isHeaderPrint = false;
      while (rs.next()) {
           ResultSetMetaData md = rs.getMetaData();
           int col_num = md.getColumnCount();
           if (isHeaderPrint){
               for (int i = 1; i  <= col_num; i++) {
                   System.out.print(md.getColumnName(i) + "|");
                   isHeaderPrint = true;
               }
           }
           for (int i = 1; i <= col_num; i++) {
               System.out.print(rs.getString(i) + "|");
           }
           System.out.println();
       }
       stmt.close();
       conn.close();
   }
}</pre>
## 概述##

C 客户端程序主要提供了数据库，集合空间，集合，游标，副本组，节点，大对象，域这8个级别的操作。更多参考 [C API](api/c/html/index.html)

## 句柄##

C 客户端驱动的句柄分为两类。一类用于数据库操作，另一类用于集群操作。

 * 数据库操作句柄

SequoiaDB 数据库中的数据存放分为三个级别：

  1）数据库

  2）集合空间

  3）集合

每一个数据库中的集合空间没有物理上限，每个集合空间在单系统内存放为一个单独的文件，因此集合空间的数量取决于磁盘和内存的大小。

每个集合空间可以包含最多4096个集合。

每个集合可以包含多条记录。

在一台物理系统内，每个集合空间最大256GB。对比关系型数据库，可以把记录看作关系型数据库的“行”；把集合看作关系型数据库的“表”。因此，在数据库操作中，可用3个句柄分别代表连接，集合空间，集合，1个句柄代表游标，1个句柄表示大对象：

<table>
  <tr>
      <td>sdbConnectionHandle</td>
      <td>数据库连接句柄</td>
      <td>代表一个单独的数据库连接</td>
  </tr>
  <tr>
      <td>sdbCSHandle</td>
      <td>集合空间句柄</td>
      <td>代表一个单独的集合空间</td>
  </tr>
  <tr>
      <td>sdbCollectionHandle</td>
      <td>集合句柄</td>
      <td>代表一个单独的集合</td>
  </tr>
  <tr>
      <td>sdbCursorHandle</td>
      <td>游标句柄</td>
      <td>代表一个查询产生的结果集</td>
  </tr>
  <tr>
      <td>sdbLobHandle</td>
      <td>大对象句柄</td>
      <td>代表一个大对象</td>
  </tr>
</table>

C 客户端程序需要使用不同的句柄进行操作。譬如读取数据的操作需要游标句柄，而创建集合空间则需要数据库连接句柄。

**Note:**

（1）对于每一个连接，其产生的集合空间，集合，与游标句柄公用一个套接字。因此在多线程系统中，必须确保每个线程不会同时针对同一套接字，在同一时间发送或接收数据。

（2） 一般来说，不建议使用多个线程共同操作一个连接句柄与其产生的其它句柄。

（3） 如果每个线程使用自己的连接句柄以及其它产生的句柄，则可以保证线程安全。

* 集群操作句柄

SequoiaDB 数据库中的集群操作分为三个级别：

  1）分区组

  2）数据节点

  3）域

**Note:**
分区组包二种类型：编目分区组、数据分区组。

分区组实例，数据节点实例，域实例可以用以下句柄表示。

<table>
        <tr>
            <td>sdbReplicaGroupHandle</td>
            <td>分区组句柄</td>
            <td>代表一个单独的分区组</td>
        </tr>
        <tr>
            <td>sdbNodeHandle</td>
            <td>数据节点句柄</td>
            <td>代表一个单独的数据节点</td>
        </tr>
        <tr>
            <td>sdbDomainHandle</td>
            <td>域句柄</td>
            <td>代表一个单独的域</td>
        </tr>
</table>


与集群相关的操作需要使用分区组及数据节点句柄。

-   sdbReplicaGroupHandle 的实例用于管理分区组。其操作包括启动、停止分区组，获取分区组中节点的状态、名称信息、数目信息。

-   sdbNodeHandle 的实例用于管理数据节点。其操作包括启动，停止指定的数据节点，获取指定数据节点实例，获取主从数据节点实例，获取数据节点地址信息。

-   sdbDomainHandle 的实例用于修改，获取域信息。

## 错误信息##

每个函数都有返回值，返回值的定义如下：

SDB_OK （数据值为0）：表示执行成功；

< 0 ：表示数据库错误，具体的错误描述在 C 驱动开发包中 include/ossErr.h 文件中可以找到；

\> 0 ：表示系统错误，请查阅相关系统的错误码信息。
## 获取驱动开发包##

从 [SequoiaDB](http://www.sequoiadb.com) 下载对应操作系统版本的 SequoiaDB 驱动开发包。

## 配置开发环境

* Linux

（1） 解压下来的驱动开发包；

（2） 将压缩包中的 driver 目录，拷贝到开发工程目录中（建议放在第三方库目录下），并命名为 sdbdriver；

（3） 将 sdbdriver/include 目录加入到编译头目录，并将 sdbdriver/lib 目录加入连接目录。

***动态链接：***

使用 lib 目录下的 libsdbc.so 动态库，gcc 编译参数形式如：
<pre class="prettyprint lang-javascript">
$ gcc testClient.c -o testClientC -I < PATH >/sdbdriver/include -L < PATH >/sdbdriver/lib -lsdbc</pre>

其中：PATH 为 sdbdriver 放置路径；运行程序时，用户需要将 LD_LIBRARY_PATH 路径指定为包含 libsdbc.so 动态库的路径。
<pre class="prettyprint lang-javascript">
$ export LD_LIBRARY_PATH=< PATH >/sdbdriver/lib</pre>

**Note:**
如果运行程序时会出现错误提示：
<pre class="prettyprint lang-diy">
error while loading shared libraries: libsdbc.so: cannot open shared object file: No such file or directory</pre>

表示没有正确设置 LD_LIBRARY_PATH，LD_LIBRARY_PATH 是环境变量，建议设置到 /etc/profile 或者应用程序的启动脚本中，避免每次新开终端都需要重新设置。

***静态链接：***

使用 lib 目录下的 libstaticsdbc.a 静态库，gcc 编译参数形式如：
<pre class="prettyprint lang-javascript">
$ gcc testClient.c -o testClientC -I < PATH >/sdbdriver/include –L < PATH >/sdbdriver/lib/libstaticsdbc.a -lm -ldl -lpthread</pre >

* Windows

暂未推出 Windows 驱动开发包。
本节介绍使用 C 程序运行 SequoiaDB。首先安装 SequoiaDB，安装信息请查看 SequoiaDB 服务器安装章节。

这里介绍如何使用 C 客户端驱动接口编写使用 SequoiaDB 数据库的程序。为了简单起见，下面的例子不全部是完整的代码，只起示例性作用。可到 /sequoiadb/client/samples/C 下获取相应的完整的代码。

## 数据库操作##

* 数据库连接（Connecting）编写完整客户端文件 connect.c 演示连接到数据库。文件必须包含“client.h”头文件。

<pre class="prettyprint lang-javascript">
#include &lt;stdio.h>  
#include "client.h"

// Display Syntax Error
void displaySyntax ( CHAR *pCommand )
{
   printf ( "Syntax: %s&lt;hostname> &lt;servicename> &lt;username> &lt;password>"
        OSS_NEWLINE, pCommand ) ;
}

INT32 main ( INT32 argc, CHAR **argv )
{
   // define a connecion handle; use to connect to database
   sdbConnectionHandle connection    = 0 ;
   INT32 rc = SDB_OK ;

   // verify syntax
   if ( 5 != argc )
   {
      displaySyntax ( (CHAR*)argv[0] ) ;
      exit ( 0 ) ;
   }

   // read argument
   CHAR *pHostName    = (CHAR*)argv[1] ;
   CHAR *pServiceName = (CHAR*)argv[2] ;
   CHAR *pUsr         = (CHAR*)argv[3] ;
   CHAR *pPasswd      = (CHAR*)argv[4] ;

   // connect to database
   rc = sdbConnect ( pHostName, pServiceName, pUsr, pPasswd, &connection ) ;
   if( rc!=SDB_OK )
   {
      printf("Fail to connet to database, rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }

   done:
   // disconnect from database
   sdbDisconnect ( connection ) ;
   // release connection
   sdbReleaseConnection ( connection ) ;
   return 0 ;
   error:
   goto done ;
   }</pre>

在 Linux 下，可以如下编译及链接动态链接库文件 libsdbc.so。

<pre class="prettyprint lang-javascript">
$gcc -o connect connect.c -I /< PATH >/sdbdriver/include -lsdbc -L /< PATH >/sdbdriver/lib
$ ./connect localhost 11810 "" ""
connect success!</pre>

**Note:**
本例程连接到本地数据库的11810端口，使用的是空的用户名很密码。用户需要根据自己的实际情况配置参数。但如果数据库已经创建用户，可以使用已经创建的用户及密码连接到数据库。

* 创建集合空间，集合

以下创建了一个名字为“foo”的集合空间和一个名字为“bar”的集合，集合空间内的集合的数据页大小为4k。可根据实际情况选择不同大小的数据页。创建集合后，可对集合做增删改查等操作。

<pre class="prettyprint lang-javascript">
// 首先，定义集合空间、集合句柄。
sdbCSHandle collectionspace       = 0 ;
sdbConnectionHandle connection    = 0 ;
// 创建集合空间"foo"
rc = sdbCreateCollectionSpace ( connection, "foo", SDB_PAGESIZE_4K, &collectionspace ) ;  
// 在新建立的集合空间中创建集合"bar"
rc = sdbCreateCollection ( collectionspace, "bar", &collection ) ;</pre>

**Note:**
在创建集合“bar”时并没有附加分区、压缩等信息，关于创建集合的更详细情况，请参考详情请查阅 [C API](api/c/html/index.html)

* 插入数据

SequoiaDB 存储数据采用 BSON 的格式，BSON 是一种类似 JSON 的二进制对象。保存数据库中的数据，首先必须创建 bson 对象。下面会将{name:"Tom",age:24}插入到集合中。

<pre class="prettyprint lang-javascript">
// 首先，我们需要创建一个插入的 bson 对象。
INT32 rc = SDB_OK ;
bson obj ;
bson_init( &obj ) ;
bson_append_string( &obj, "name", "tom" ) ;
bson_append_int( &obj, "age", 24 ) ;
rc = bson_finish( &obj ) ;
if ( rc != SDB_OK )
printf("Error.");
// 接着，把此 bson 对象插入集合中
rc = sdbInsert ( collection, &obj ) ;</pre>

* 查询

查询操作需要一个游标句柄存放查询的结果到本地。要获得查询的结果需要使用游标操作。本例使用了游标操作的 sdbNext 接口，表示从查询结果中取到一条记录。此示例中没有设置查询条件，筛选条件，排序情况，及仅使用默认索引。

<pre class="prettyprint lang-javascript">
// 定义一个游标句柄
sdbCursorHandle cursor = 0 ;
...
// 查询所有记录，查询结果放在游标句柄中
rc = sdbQuery(collection, NULL, NULL, NULL, NULL, 0, -1, &cursor ) ;
// 从游标中显示所有记录
bson_init(obj);
while( !( rc=sdbNext( cursor, &obj ) ) )
{
  bson_print( &obj ) ;
  bson_destroy(&obj) ;
  bson_init(&obj);
}
bson_destroy(obj) ;</pre>

* 索引

此处，我们在集合句柄 collection 指定的集合中创建一个以“name”为升序，“age”为降序的索引。

<pre class="prettyprint lang-javascript">
#define INDEX_NAME "index"
...
// 首先创建一 bson 对象包含将要创建的索引的信息
bson_init( &obj ) ;
bson_append_int( &obj, "name", 1 ) ;
bson_append_int( &obj, "age", -1 ) ;
rc = bson_finish( &obj ) ;
if ( rc != SDB_OK )
printf("Error.");
// 创建一个以"name"为升序，"age"为降序的索引
rc = sdbCreateIndex ( collection, &obj, INDEX_NAME, FALSE, FALSE ) ;
bson_destroy ( &obj ) ;</pre>

* 更新

此处，我们在集合句柄 collection 指定的集合中更新记录。因为没有指定数据匹配规则，所以此示例将更新集合中所有的集合。

<pre class="prettyprint lang-javascript">
// 先创建一个包含更新规则的 bson 对象
bson_init( &rule ) ;
bson_append_start_object ( &rule, "$set" ) ;
bson_append_int ( &rule, "age", 19 ) ;
bson_append_finish_object ( &rule ) ;
rc = bson_finish ( &rule ) ;
if ( rc != SDB_OK )
printf("Error.");
// 打印出更新规则
bson_print( &rule ) ;
// 更新记录
rc = sdbUpdate( collection, &rule, NULL, NULL ) ;
bson_destroy(&rule);</pre>

此处，因为没有指定记录匹配条件，所以此示例将更新集合句柄 collection 指定的集合中所有的记录。

## 集群操作##

* 分区组操作

分区组操作包括创建分区组（sdbCreateReplicaGroup），得到分区组句柄（sdbGetReplicaGroup），启动分区组（sdbStartReplicaGroup），停止分区组（sdbStopReplicaGroup）等。以下为分区组操作示例性的例子。真正的应用应包括错误检测等。

<pre class="prettyprint lang-javascript">
// 定义一个分区组句柄
sdbReplicaGroupHandle rg = 0 ;
...
// 先建立一个编目分区组
rc = sdbCreateCataReplicaGroup ( connection, HOST_NAME, SERVICE_NAME, CATALOG_SET_PATH , NULL ) ;
// 创建数据分区组
rc = sdbCreateReplicaGroup ( connection, GROUP_NAME, &rg ) ;
// 创建数据节点
rc = sdbCreateNode ( rg, HOST_NAME1, SERVICE_NAME1, DATABASE_PATH1, NULL ) ;
// 启动分区组
rc = sdbStartReplicaGroup( rg ) ;</pre>

* 数据节点操作

数据节点操作包括创建数据节点（sdbCreateNode），得到主数据节点（sdbGetNodeMaster），得到从数据节点（sdbGetNodeSlave），启动数据节点（sdbStartNode），停止数据节点（sdbStopNode）等。以下为数据节点操作示例性的例子。真正的应用应包括错误检测等。

<pre class="prettyprint lang-javascript">
// 定义一个数据节点句柄
sdbNodeHandle masternode   = 0 ;
sdbNodeHandle slavenode    = 0 ;
...
// 获取主数据节点
rc = sdbGetNodeMaster ( rg, &masternode ) ;
//获取从数据节点
rc = sdbGetNodeSlave ( rg, &slavenode ) ;</pre>
SequoiaDB 的查询用 json（bson）对象表示，下表以例子的形式显示了 SQL 语句，SequoiaDB shell 语句和 SequoiaDB C 驱动程序语法之间的对照。

+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+
| SQL                                                             | SequoiaDB shell                                     | C Driver                                                                         |
+=================================================================+=====================================================+==================================================================================+
| insert into students(a,b) values(1,-1)                          | db.foo.bar.insert({a:1,b:-1})                       |                                                                                  |
|                                                                 |                                                     | -    const char *r ="{a:1,b:-1}";                                                |
|                                                                 |                                                     | -    jsonToBson ( &obj, r );                                                     |
|                                                                 |                                                     |      // jsonToBson 将一个 json 字符串转换为 bson 对象                            |
|                                                                 |                                                     | -    sdbInsert (collection, &obj );                                              |
|                                                                 |                                                     |      // collection 为集合 bar 的句柄                                             |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+
| select a,b from students                                        | db.foo.bar.find(null,{a:"",b:""})                   |                                                                                  |
|                                                                 |                                                     | -    const char *r ="{a:\"\",b:\"\"}";                                           |
|                                                                 |                                                     | -    jsonToBson ( & select, r );                                                 |
|                                                                 |                                                     |      // jsonToBson 将一个 json 字符串转换为 bson 对象                            |
|                                                                 |                                                     | -    sdbQuery ( collection, NULL, &select, NULL, NULL, 0, -1, cursor );          |
|                                                                 |                                                     |      // collection 为集合 bar 的句柄，cursor 为返回查询结果的句柄                |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+
|                                                                 |                                                     |                                                                                  |
| select * from students                                          | db.foo.bar.find()                                   | -    sdbQuery ( collection, NULL, NULL, NULL, NULL, 0, -1, cursor );             |
|                                                                 |                                                     |      // collection 为集合 bar 的句柄，cursor 为返回查询结果的句柄                |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+
|                                                                 |                                                     |                                                                                  |
| select * from students where age=20                             | db.foo.bar.find({age:20})                           | -    const char *r ="{age:20}";                                                  |
|                                                                 |                                                     | -    jsonToBson ( &condition, r );                                               |
|                                                                 |                                                     |      // jsonToBson 将一个 json 字符串转换为 bson 对象                            |
|                                                                 |                                                     | -    sdbQuery ( collection, & condition, NULL, NULL, NULL, 0, -1, cursor );      |
|                                                                 |                                                     |      // collection 为集合 bar 的句柄，cursor 为返回查询结果的句柄                |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+
|                                                                 |                                                     |                                                                                  |
| select * from students where age=20 order by name               | db.foo.bar.find({age:20}).sort({name:1})            | -    const char *r1 ="{age:20}";                                                 |
|                                                                 |                                                     | -    const char *r2 ="{name:1}";                                                 |
|                                                                 |                                                     | -    jsonToBson ( & condition, r1 );                                             |
|                                                                 |                                                     |      // jsonToBson 将一个 json 字符串转换为 bson 对象                            |
|                                                                 |                                                     | -    jsonToBson ( &orderBy, r2 );                                                |
|                                                                 |                                                     | -    sdbQuery ( collection, & condition, NULL, & orderBy, NULL, 0, -1, cursor ); |
|                                                                 |                                                     |      // collection 为集合 bar 的句柄，cursor 为返回查询结果的句柄                |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+
|                                                                 |                                                     |                                                                                  |
| select * from students where age > 20 and age < 30              | db.foo.bar.find({age:{\$gt:20,$lt:30}})             | -    const char *r ="{age:{\$gt:20,$lt:30}}" ;                                   |
|                                                                 |                                                     | -    jsonToBson ( &condition, r );                                               |
|                                                                 |                                                     |      // jsonToBson 将一个 json 字符串转换为 bson 对象                            |
|                                                                 |                                                     | -    sdbQuery ( collection, & condition , NULL, NULL, NULL, 0, -1, cursor );     |
|                                                                 |                                                     |      // collection 为集合 bar 的句柄，cursor 为返回查询结果的句柄                |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+
|                                                                 |                                                     |                                                                                  |
| create index testIndex on students(name)                        | db.foo.bar.createIndex("testIndex",{name:1},false)  | -    const char *r ="{name:1}" ;                                                 |
|                                                                 |                                                     | -    jsonToBson ( &obj, r );                                                     | 
|                                                                 |                                                     |      // jsonToBson 将一个 json 字符串转换为 bson 对象                            |
|                                                                 |                                                     | -    sdbCreateIndex ( collection, &obj, "testIndex", FALSE, FALSE )              |
|                                                                 |                                                     |      // collection 为集合 bar 的句柄                                             |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+
|                                                                 |                                                     |                                                                                  |
| select * from students limit 20 skip 10                         | db.foo.bar.find().limit(20).skip(10)                | -    sdbQuery ( collection, NULL, NULL, NULL, NULL, 10, 20, cursor );            |
|                                                                 |                                                     |      // collection 为集合 bar 的句柄，cursor 为返回查询结果的句柄                |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+
|                                                                 |                                                     |                                                                                  |
| select count(*) from students where age > 20                    | db.foo.bar.find({age:{$gt:20}}).count()             | -    const char *r ="{age:{$gt:20}}" ;                                           |
|                                                                 |                                                     | -    jsonToBson ( &condition, r );                                               |
|                                                                 |                                                     |      // jsonToBson 将一个 json 字符串转换为 bson 对象                            |
|                                                                 |                                                     | -    sdbGetCount ( collection, &condition, &count );                             |
|                                                                 |                                                     |      // collection 为集合 bar 的句柄，count 为返回总数                           |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+
|                                                                 |                                                     |                                                                                  |
| update students set a=a+2 where b=-1                            | db.foo.bar.update({$set:{a:2}},{b:-1})              | -    const char *r1 ="{$set:{a:2}}" ;                                            |
|                                                                 |                                                     | -    const char *r2 ="{b:-1}" ;                                                  |
|                                                                 |                                                     | -    jsonToBson ( &rule, r1 );                                                   |
|                                                                 |                                                     |      // jsonToBson 将一个 json 字符串转换为 bson 对象                            |
|                                                                 |                                                     | -    jsonToBson ( &condition, r2 );                                              |
|                                                                 |                                                     | -    sdbUpdate (collection, &rule, &condition, NULL );                           |
|                                                                 |                                                     |      // collection 为集合 bar 的句柄                                             |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+
|                                                                 |                                                     |                                                                                  |
| delete from students where a=1                                  | db.foo.bar.remove({a:1})                            | -    const char *r ="{a:1}" ;                                                    |
|                                                                 |                                                     | -    jsonToBson ( &condition, r );                                               |
|                                                                 |                                                     |      // jsonToBson 将一个 json 字符串转换为 bson 对象                            |
|                                                                 |                                                     | -    sdbDelete ( collection, &condition, NULL );                                 |
|                                                                 |                                                     |      // collection 为集合 bar 的句柄                                             |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------------------------------+此部分是相关 [C API](api/c/html/index.html) 文档。

## 历史更新情况：##

**Version 1.10**

（1） 添加获取查询访问计划的接口：

    sdbExplain，获取查询的访问计划

（2） 添加用于大对象（lob）操作的接口：

    sdbListLobs，列出集合中的所有lob
    sdbOpenLob，创建或打开一个lob
    sdbCloseLob，关闭一个lob
    sdbRemoveLob， 删除一个lob
    sdbSeekLob，设置读起始位置，该版本中，seek只用于读操作
    sdbReadLob，从lob中读取数据
    sdbWriteLob，把数据写入lob
    sdbGetLobSize，获取lob的大小
    sdbGetLobCreateTime，获取lob的创建时间

**Version 1.8**

新添加接口：

    sdbConnect1，可提供多个地址，接口随机选择一个有效的地址连接。
    sdbCreateCollectionSpaceV2，提供一个 bson 的选项，使创建集合空间更加灵活
    sdbAlterCollection，修改集合（表）属性
    sdbCreateDomain，创建域
    sdbDropDomain，删除域
    sdbGetDomain，获取域句柄
    sdbListDomains，列出所有域
    sdbReleaseDomain，删除域句柄
    sdbAlterDomain，更改域属性

**Version 1.6**

（1） 使用 sdbNodeHandle 来取代原来的 sdbReplicaNodeHandle。sdbReplicaNodeHandle 将在 version 2.x 中被弃用。

（2） 使用概念“node”取代原来的“replica node”，和“replica node”相关的 API 接口将保留，直到 version 2.x 会被弃用。

更多详情可查看辅助API [BASE64C API](api/base64c/html/index.html) 和 [JSTOBSON API](api/Jstobs/html/index.html)。
## 概述##

C++ 客户端驱动提供了数据库操作和集群操作的接口。主要包括以下8个级别的操作：

-    数据库
-    集合空间
-    集合
-    游标
-    副本组
-    节点
-    域
-    大对象

## C++ 类实例##

C++ 客户端驱动的有两种类实例。一种用于数据库操作，另一种用于集群操作。

* 数据库操作实例

SequoiaDB 数据库中的数据存放分为三个级别：

  1）数据库

  2）集合空间

  3）集合

因此，在数据库操作中，可用3个类来分别表示数据库连接，集合空间，集合，1个类表示游标，1个类表达大对象：

<table>
        <tr>
            <td>Sdb</td>
            <td>数据库类</td>
            <td>数据库类主要用于管理整个数据库，包括建立连接，创建集合空间等</td>
        </tr>
        <tr>
            <td>sdbCollectionSpace</td>
            <td>集合空间类</td>
            <td>集合空间主要用于管理集合</td>
        </tr>
        <tr>
            <td>sdbCollection</td>
            <td>集合类</td>
            <td>集合类主要用于对数据进行增删改查等操作</td>
        </tr>
        <tr>
            <td>sdbCursor</td>
            <td>游标类</td>
            <td>游标类主要用于遍历查询、快照返回的结果游标实例代表一个查询产生的游标</td>
        </tr>
        <tr>
            <td>sdbLob</td>
            <td>大对象类</td>
            <td>大对象类用于对大对象进行读写等操作</td>
        </tr>
</table>

C++ 客户端需要使用不同的实例进行操作。譬如读取数据的操作需要游标实例，而创建表空间则需要数据库实例。

**Note:**

（1） 对于每一个连接，其产生的集合空间，集合，与游标句柄公用一个套接字。因此在多线程系统中，必须确保每个线程不会同时针对同一套接字，在同一时间发送或接收数据。

（2） 一般来说，不建议使用多个线程共同操作一个连接句柄与其产生的其它句柄。

（3） 如果每个线程使用自己的连接句柄以及其它产生的句柄，则可以保证线程安全。

* 集群操作实例

SequoiaDB 数据库中的集群操作分为三个级别：

  1）分区组

  2）数据节点

  3）域

**Note:**
分区组包两种类型：编目分区组，数据分区组。

分区组实例，数据节点实例，域实例可以用以下三种类的实例表示。

<table>
        <tr>
            <td>sdbReplicaGroup  </td>
            <td>分区组类</td>
            <td>分区组实例代表一个单独的分区组</td>
        </tr>
        <tr>
            <td>sdbNode</td>
            <td>数据节点类</td>
            <td>数据节点实例代表一个单独的数据节点</td>
        </tr>
        <tr>
            <td>sdbDomain</td>
            <td>域类</td>
            <td>域实例代表一个管理若干个分区组的域</td>
        </tr>
</table>

与集群相关的操作需要使用分区组及数据节点实例。

-   sdbReplicaGroup 的实例用于管理分区组。其操作包括启动，停止分区组，获取分区组中节点的状态，名称信息，数目信息。

-   sdbNode 的实例用于管理数据节点。其操作包括启动，停止指定的数据节点，获取指定数据节点实例，获取主从数据节点实例，获取数据节点地址信息。

-   sdbDomain 的实例用于管理域。其包括修改域，获取域信息等操作。

## 错误信息##

每个函数都有返回值，返回值的定义如下：

SDB_OK（数据值为0）：表示执行成功；

< 0 ：表示数据库错误，具体的错误描述在 C++ 驱动开发包中 include/ossErr.h 文件中可以找到；

\> 0 ：表示系统错误，请查阅相关系统的错误码信息。
## 获取驱动开发包##

从 [SequoiaDB](http://www.sequoiadb.com) 下载对应操作系统版本的 SequoiaDB 驱动开发包。

## 配置开发环境##

* Linux

（1） 解压下来的驱动开发包；

（2） 将压缩包中的 driver 目录，拷贝到开发工程目录中（建议放在第三方库目录下），并命名为 sdbdriver。

（3） 将 sdbdriver/include 目录加入到编译头目录，并将 sdbdriver/lib 目录加入连接目录。

***动态链接：***

使用 lib 目录下的 libsdbcpp.so 动态库，g++ 编译参数形式如：
<pre class="prettyprint lang-javascript">
$ g++ main.cpp -o test -I &lt;PATH&gt;/sdbdriver/include -L &lt;PATH&gt;/sdbdriver/lib -lsdbcpp</pre>

其中：PATH 为 sdbdriver 放置路径；运行程序时，用户需要将 LD_LIBRARY_PATH 路径指定为包含 libsdbcpp.so 动态库的路径。
<pre class="prettyprint lang-javascript">
$ export LD_LIBRARY_PATH=&lt;PATH&gt;/sdbdriver/lib</pre>

**Note:**

如果运行程序时会出现错误提示：
<pre class="prettyprint lang-diy">
error while loading shared libraries: libsdbcpp.so: cannot open shared object file: No such file or directory</pre>

表示没有正确设置 LD_LIBRARY_PATH，LD_LIBRARY_PATH 是环境变量，建议设置到 /etc/profile 或者应用程序的启动脚本中，避免每次新开终端都需要重新设置。

***静态链接：***

使用 lib 目录下的 libstaticsdbc.a 静态库，g++ 编译参数形式如：

<pre class="prettyprint lang-javascript">
$ g++ main.c -o test -I &lt;path&gt;/sdbdriver/include –L &lt;path&gt;/sdbdriver/lib/libstaticsdbcpp.a –lm -lpthread -ldl</pre>

* Windows

暂未推出 Windows 驱动开发包。
这里介绍如何使用 C++ 客户端驱动接口编写使用 SequoiaDB 数据库的程序。为了简单起见，下面的示例不全部是完整的代码，只起示例性作用。可到 /sequoiadb/client/samples/CPP 下获取相应的完整的代码。更多查看 [C++ API](api/cpp/html/index.html)

## 数据库操作##

* 连接数据库：
connect.cpp 演示如何连接到数据库。文件应当包含“client.hpp”头文件及使用命名空间 sdbclient。

<pre class="prettyprint lang-javascript">
#include &lt;iostream&gt;;
#include "client.hpp"

using namespace std ;
using namespace sdbclient ;

// Display Syntax Error
void displaySyntax ( CHAR *pCommand ) ;

INT32 main ( INT32 argc, CHAR **argv )
{
   // verify syntax
   if ( 5 != argc )
   {
      displaySyntax ( (CHAR*)argv[0] ) ;
      exit ( 0 ) ;
   }
   // read argument
   CHAR *pHostName    = (CHAR*)argv[1] ;
   CHAR *pPort        = (CHAR*)argv[2] ;
   CHAR *pUsr         = (CHAR*)argv[3] ;
   CHAR *pPasswd      = (CHAR*)argv[4] ;

   // define local variable
   sdb connection ;
   INT32 rc = SDB_OK ;

   // connect to database
   rc = connection.connect ( pHostName, pPort, pUsr, pPasswd ) ;
   if( rc!=SDB_OK )
   {
      cout << "Fail to connet to database, rc = " << rc << endl ;
      goto error ;
   }
   else
      cout << "Connect success!" << endl ;

  done:
     // disconnect from database
     connection.disconnect () ;
     return 0 ;
  error:
     goto done ;
}

// Display Syntax Error
void displaySyntax ( CHAR *pCommand )
{
  cout << "Syntax:" << pCommand << " &lt;hostname&gt;  &lt;servicename&gt;  &lt;username&gt;  &lt;password&gt; " << endl ;
}</pre>

在 Linux下，可以如下编译及链接动态链接库文件 libsdbcpp.so:

<pre class="prettyprint lang-javascript">
$ g++ -o connect connect.cpp -I &lt;PATH&gt;/sdbdriver/include -lsdbcpp -L &lt;PATH&gt;/sdbdriver/lib  
执行结果如下：
$ ./connect localhost 11810 "" ""
Connect success!</pre>


**Note:**

本例程连接到本地数据库的11810端口，使用的是空的用户名和密码。用户需要根据自己的实际情况配置参数。譬如：`./connect localhost 11810 "sequoiadb" "sequoiadb"`。当数据库已经创建用户时，应该使用正确的用户及密码连接到数据库，否则连接失败。

* 创建集合空间和集合


首先，定义集合空间，集合对象。
<pre class="prettyprint lang-javascript">
sdbCollectionSpace collectionspace ;
sdbCollection collection ;</pre>

创建集合空间"foo"
<pre class="prettyprint lang-javascript">
rc = connection.createCollectionSpace ( "foo", SDB_PAGESIZE_4K, collectionspace ) ;</pre>

在新建立的集合空间中创建集合"bar"
<pre class="prettyprint lang-javascript">
rc = collectionspace.createCollection ( "bar", collection ) ;</pre>

以上创建了一个名字为“foo”的集合空间和一个名字为“bar”的集合，集合空间内的集合的数据页大小为4k。可根据实际情况选择不同大小的数据页。创建集合后，可对集合做增删改查等操作。

**Note:**

在创建集合“bar”时并没有附加分区，压缩等信息，详情请查阅 [C++ API](api/cpp/html/index.html)

* 插入数据：insert

首先，需要创建一个插入的 bson 对象。
<pre class="prettyprint lang-javascript">
BSONObj obj ;
obj = BSON ( "name" << "tom" << "age" << 24 ) ;</pre>

接着，把此 bson 对象插入集合中
<pre class="prettyprint lang-javascript">
collection.insert ( obj ) ;</pre>

obj 为输入参数，为要插入的数据。

* 查询：query

定义一个游标对象
<pre class="prettyprint lang-javascript">
sdbCursor cursor ;
...</pre>

查询所有记录，并把查询结果放在游标对象中
<pre class="prettyprint lang-javascript">
collection.query ( cursor ) ;</pre>

从游标中显示所有记录
<pre class="prettyprint lang-javascript">
while( !( rc=cursor.next( obj ) )
{
  cout << obj.toString() << endl ;
}</pre>

查询操作需要一个游标对象存放查询的结果到本地。要获得查询的结果需要使用游标操作。本例使用了游标操作的 next 接口，表示从查询结果中取到一条记录。此示例中没有设置查询条件，筛选条件，排序情况，及仅使用默认索引。

* 创建索引：index

<pre class="prettyprint lang-javascript">
# define INDEX_NAME "index"
...</pre>

首先创建一 BSONObj 对象包含将要创建的索引的信息
<pre class="prettyprint lang-javascript">
BSONObj obj ;
obj = BSON ( "name" << 1 << "age" << -1 ) ;</pre>


创建一个以"name"为升序，"age"为降序的索引
<pre class="prettyprint lang-javascript">
collection.createIndex ( obj, INDEX_NAME, FALSE, FALSE ) ;</pre>

集合对象 collection 中创建一个以"name"为升序，"age"为降序的索引。

* 更新：update

先创建一个包含更新规则的 BSONObj 对象
<pre class="prettyprint lang-javascript">
BSONObj rule = BSON ( "$set" << BSON ( "age" << 19 ) ) ;</pre>

打印出更新规则
<pre class="prettyprint lang-javascript">
cout << rule.toString() << endl ;</pre>

更新记录
<pre class="prettyprint lang-javascript">
collection.update( rule ) ;</pre>

在集合对象 collection 中更新了记录。实例中没有指定数据匹配规则，所以此示例将更新集合中所有的集合。

## 集群操作##

* 分区组操作

分区组操作包括创建分区组（sdb:createReplicaGroup），得到分区组实例（sdb:getReplicaGroup），启动分区组所有数据节点（sdbReplicaGroup::start），停止分区组所有数据节点（sdbReplicaGroup::stop）等。

以下为分区组操作示例性的例子。真正的应用应包括错误检测等。

定义一个分区组实例
<pre class="prettyprint lang-javascript">
sdbReplicaGroup rg  ;</pre>

定义一个空的 map 对象表示创建数据节点没有更多的配置内容
<pre class="prettyprint lang-javascript">
map&lt;string,string&gt; config ;
...</pre>

先建立一个编目分区组
<pre class="prettyprint lang-javascript">
connection.createCataReplicaGroup ( HOST_NAME, SERVICE_NAME, CATALOG_GROUP_PATH, NULL ) ;</pre>

创建数据分区组
<pre class="prettyprint lang-javascript">
connection.createRG ( REPLICA_GROUP_NAME, rg ) ;</pre>

创建第一个数据节点
<pre class="prettyprint lang-javascript">
rg.createNode ( HOST_NAME1, SERVICE_NAME1, DATABASE_PATH1, config ) ;
...</pre>

启动分区组
<pre class="prettyprint lang-javascript">
rg.start () ;</pre>

* 数据节点操作

数据节点操作包括创建数据节点（sdbReplicaGroup::createNode），得到主数据节点（sdbReplicaGroup::getMaster），得到从数据节点（sdbReplicaGroup::getSlave），启动数据节点（sdbNode::start），停止数据节点（sdbNode::Stop）等。

以下为数据节点操作示例性的例子。真正的应用应包括错误检测等。

定义一个数据节点实例
<pre class="prettyprint lang-javascript">
sdbNode masternode ;
sdbNode slavenode ;
...</pre>

获取主数据节点
<pre class="prettyprint lang-javascript">
rg.getMaster( masternode ) ;</pre>

获取从数据节点
<pre class="prettyprint lang-javascript">
rg.getSlave( slavenode ) ;</pre>
SequoiaDB 的查询用 json（bson）对象表示，下表以例子的形式显示了 SQL 语句，SequoiaDB shell 语句和 SequoiaDB C++ 驱动程序语法之间的对照。

+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
| SQL                                                             | SequoiaDB shell                                     | C++ Driver                                                         |
+=================================================================+=====================================================+====================================================================+
| insert into students(a,b) values(1,-1)                          | db.foo.bar.insert({a:1,b:-1})                       |                                                                    |
|                                                                 |                                                     | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    BSONObj obj = BSON("a" << 1 << "b" << -1) ;                   |
|                                                                 |                                                     | -    collection.insert( obj ) ;                                    |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
| select a,b from students                                        | db.foo.bar.find(null,{a:"",b:""})                   |                                                                    |
|                                                                 |                                                     | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    BSONObj selected ;                                            |
|                                                                 |                                                     | -    BSONObj obj ;                                                 |
|                                                                 |                                                     | -    selected = BSON( "a"<<""<<"b"<<"" ) ;                         |
|                                                                 |                                                     | -    collection .query( cursor, obj, selected ) ;                  |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select * from students                                          | db.foo.bar.find()                                   | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    collection .query ( cursor ) ;                                |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select * from students where age=20                             | db.foo.bar.find({age:20})                           | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    BSONObj condition ;                                           |
|                                                                 |                                                     | -    condition = BSON("age" << 20) ;                               |
|                                                                 |                                                     | -    collection .query ( cursor, condition ) ;                     |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select * from students where age=20 order by name               | db.foo.bar.find({age:20}).sort({name:1})            | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    BSONObj obj ;                                                 |
|                                                                 |                                                     | -    BSONObj condition ;                                           |
|                                                                 |                                                     | -    BSONObj orderBy ;                                             |
|                                                                 |                                                     | -    condition = BSON("age"<<20) ;                                 |
|                                                                 |                                                     | -    orderBy = BSON("name"<<1) ;                                   |
|                                                                 |                                                     | -    collection .query (cursor, condition , obj, orderBy , obj ) ; |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select * from students where age > 20 and age < 30              | db.foo.bar.find({age:{\$gt:20,$lt:30}})             | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    BSONObj condition ;                                           |
|                                                                 |                                                     | -    condition = BSON("age"<<BSON("$gt"<<20<<" $lt"<<30)) ;        |
|                                                                 |                                                     | -    collection .query (cursor, condition ) ;                      |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| create index testIndex on students(name)                        | db.foo.bar.createIndex("testIndex",{name:1},false)  | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    BSONObj obj ;                                                 | 
|                                                                 |                                                     | -    obj = BSON( "name"<<1 ) ;                                     |
|                                                                 |                                                     | -    collection.createIndex ( &obj, "testIndex", FALSE, FALSE );   |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select * from students limit 20 skip 10                         | db.foo.bar.find().limit(20).skip(10)                | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    BSONObj obj ;                                                 |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    collection .query (cursor,obj, obj, obj, obj, 10, 20 ) ;      |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select count(*) from students where age > 20                    | db.foo.bar.find({age:{$gt:20}}).count()             | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    SINT64 count = 0 ;                                            |
|                                                                 |                                                     | -    BSONObj condition ;                                           |
|                                                                 |                                                     | -    Condition = BSON( "age"<<BSON("$gt"<<20)) ;                   |
|                                                                 |                                                     | -    collection.getCount (count, condition );                      |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| update students set a=a+2 where b=-1                            | db.foo.bar.update({$set:{a:2}},{b:-1})              | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    BSONObj condition = BSON( "b"<<1 ) ;                          |
|                                                                 |                                                     | -    BSONObj rule = BSON( "$set"<<BSON("a"<<2) );                  |
|                                                                 |                                                     | -    BSONObj obj ;                                                 |
|                                                                 |                                                     | -    collection.update ( rule, condition, obj ) ;                  |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| delete from students where a=1                                  | db.foo.bar.remove({a:1})                            | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    BSONObj condition = BSON("a"<<1) ;                            |
|                                                                 |                                                     | -    collection.del ( condition ) ;                                |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+

此部分是相关 [C++ API](api/cpp/html/index.html) 文档。

## 历史更新情况：##

**Version 1.10**

（1） SdbCollection 类添加的接口：

    explain，获取查询的访问计划
    createLob，创建一个新的lob
    openLob，打开一个已存在的lob，该版本中，打开的lob只用于读操作
    removeLob，删除一个lob
    listLobs，列出当前collection中的所有lob

（2） 添加类 sdbLob 用于大对象操作，其接口如下：

    read，从lob中读取数据
    write，把数据写入lob中
    seek，设置读起始位置，该版本中，seek只用于读操作
    close，关闭一个新创建的或打开的lob
    getOid，获取lob的oid
    getSize，获取lob的大小
    getCreateTime，获取lob的创建时间

**Version 1.8**

（1） sdb 类新添加的接口：

    connect，可提供多个地址，接口随机选择一个有效的地址连接。
    createCollectionSpace，提供一个 BSONObject 的选项，使创建集合空间更加灵活
    backupOffline，离线备份支持更多的选项
    createDomain，创建域
    getDomain，获取域
    dropDomain，删除域
    listDomain，列出所有域

（2） sdbCollection 类新添加的接口：

    alterCollection，修改集合（表）属性

（3） 添加 Domain 类用于与域相关的操作

**Version 1.6**

（1） 添加类 Node 来取代原来的类 ReplicaNode。类 ReplicaNode 以及与它相关的方法将在 version 2.x 中被弃用。

更多详情可查看辅助API [BASE64C API](api/base64c/html/index.html) 和 [FROMJSON API](api/fromjson/html/index.html)。
## 概述##

SequoiaDB C# 驱动提供了数据库操作和集群操作的接口。数据库操作包括数据库的连接，用户的创建删除，数据的增删改查，索引的创建删除，快照的获取与重置，以及集合与集合空间的创建删除操作等操作。集群操作包括管理分区组和数据节点的各种操作，譬如启动，停止分区组，启动，停止数据节点，获取主从数据节点，集合分区等。

## C# 类实例##

C# 驱动的有两种类实例。一种用于数据库操作，另一种用于集群操作。

* 数据库操作实例

SequoiaDB 数据库中的数据存放分为三个级别：

1）数据库

2）集合空间

3）集合

因此，在数据库操作中，可用3个类来分别表示连接，集合空间，集合实例，另1个类表示游标实例，1个类表示大对象：

<table>
        <tr>
            <td>SequoiaDB</td>
            <td>数据库实例</td>
            <td>代表一个单独的数据库连接</td>
        </tr>
        <tr>
            <td>CollectionSpace</td>
            <td>集合空间实例</td>
            <td>代表一个单独的集合空间</td>
        </tr>
        <tr>
            <td>DBCollection</td>
            <td>集合实例</td>
            <td>代表一个单独的集合</td>
        </tr>
        <tr>
            <td>DBCursor</td>
            <td>游标实例</td>
            <td>代表一个查询产生的结果集</td>
        </tr>
        <tr>
            <td>DBLob</td>
            <td>大对象实例</td>
            <td>用于对大对象进行读写操作</td>
        </tr>
</table>

C# 驱动需要使用不同的实例进行操作。譬如读取数据的操作需要游标实例，而创建表空间则需要数据库实例。

* 集群操作实例

SequoiaDB 数据库中的集群操作分为三个级别：

    1）分区组 

    2）数据节点 

    3）域

**Note:**

分区组包含三种类型：协调分区组，编目分区组，数据分区组。

分区组实例和数据节点实例及域实例可以用以下三种类的实例表示。

<table>
    <tr>
        <td>ReplicaGroup</td>
        <td>分区组类</td>
        <td>分区组实例代表一个单独的分区组</td>
    </tr>
    <tr>
        <td>Node</td>
        <td>数据节点类</td>
        <td>数据节点实例代表一个单独的数据节点</td>
    </tr>
    <tr>
        <td>Domain</td>
        <td>域</td>
        <td>用于管理若干个分区组</td>
    </tr>
</table>

无疑与集群相关的操作需要使用分区组及数据节点实例。

ReplicaGroup 的实例用于管理分区组。其操作包括启动，停止分区组，获取分区组中节点的状态，名称信息，数目信息。

Node 的实例用于管理节点。其操作包括启动，停止指定的节点，获取指定节点实例，获取主从节点实例，获取数据节点地址信息。


## 线程安全性##

对于每一个连接，其产生的集合空间，集合公用一个套接字。因此在多线程系统中，必须确保每个线程不会同时针对同一套接字，在同一时间发送或接收数据。一般来说，不建议使用多个线程共同操作一个连接实例与其产生的其它实例。如果每个线程使用自己的连接实例以及其它产生的实例，则可以保证线程安全。


## 错误信息##

每一个接口都会抛出 SequoiaDB.BaseException 和 System.Exception 异常，分别对应于数据库引擎返回的异常信息和客户端本地的异常信息，其中 BaseException 的异常信息可以通过该异常类的 ErrorCode，ErrorType 和 Message 属性获得。
## 获取驱动开发包##

从 [SequoiaDB](http://www.sequoiadb.com) 下载对应操作系统版本的 SequoiaDB 驱动开发包；解压驱动开发包，从 driver/CSharp/ 目录中获取 sequoiadb.dll 链接库，然后，在 Visual Studio 中引用该链接库，或者在命令行编译时指定引用该链接库，比如“csc /target:exe /reference:sequoiadb.dll Find.cs Common.cs”，即可使用相关 API。在安装目录下的 smaples\\C# 目录可以找到 C# 驱动的完整示例。

## BSON 库 API##

SequoiaDB 数据库的 C# 驱动使用了第三方公司 MongoDB 提供的 C# BSON 库，详细介绍可以参照 [MongoDB 官方文档](http://docs.mongodb.org/ecosystem/tutorial/use-csharp-driver/#the-bson-library)

## Visual Studio 版本支持##

当前版本的 C# 驱动可在以下版本的 Visual Studio 中使用

-   Visual Studio 2008

-   Visual Studio 2010

## .NET Framework 版本支持##

当前版本的 C# 驱动在 .NET Framework3.5 中生成，可在以下版本的 .NET Framework 中使用

-   .NET Framework 3.5

-   .NET Framework 4.0
这里介绍如何使用 C# 驱动接口编写使用 SequoiaDB 数据库的程序。该文档介绍了 SequoiaDB 数据库 C# 驱动的简单示例，详细的使用规范可参照官方的 [C# API](api/cs/html/index.html) 文档。

## 命名空间##

在使用 C# 驱动的相关 API 之前，你必须在源代码中添加如下的 using 申明：
<pre class="prettyprint lang-javascript">
using SequoiaDB;
using SequoiaDB.Bson;</pre>


数据操作

* 连接数据库和身份验证

若数据库没有创建用户，则可以匿名连接到数据库：
<pre class="prettyprint lang-javascript">
string addr = "127.0.0.1:11810";
Sequoiadb sdb = new Sequoiadb(addr);
try
{
    sdb.Connect();
}
catch (BaseException e)
{
    Console.WriteLine("ErrorCode:{0}, ErrorType:{1}", e.ErrorCode, e.ErrorType);
    Console.WriteLine(e.Message);
}
catch (System.Exception e)
{
    Console.WriteLine(e.StackTrace);
    }</pre>

否则，连接的时候必须指定用户名和密码：
<pre class="prettyprint lang-javascript">
string addr = "127.0.0.1:11810";
Sequoiadb sdb = new Sequoiadb(addr);
try
{
    sdb.Connect("testusr", "testpwd");
}
catch (BaseException e)
{
    Console.WriteLine("ErrorCode:{0}, ErrorType:{1}", e.ErrorCode, e.ErrorType);
    Console.WriteLine(e.Message);
}
catch (System.Exception e)
{
    Console.WriteLine(e.StackTrace);
}</pre>

这里给出了异常信息的 try 和 catch 块，下面的所有操作都会抛出同样的异常信息，因此不再给出相关的 try 和 catch 块。

* 断开与数据库连接

<pre class="prettyprint lang-javascript">
// do not forget to disconnect from sdb
sdb.Disconnect();</pre>

* 得到或创建集合空间和集合

根据名字，得到对应的 CollectionSpace，如果不存在，则创建：
<pre class="prettyprint lang-javascript">
// create collectionspace, if collectionspace exists get it
string csName = "TestCS";
CollectionSpace cs = sdb.GetCollecitonSpace(csName);
if (cs == null)
cs = sdb.CreateCollectionSpace(csName);
// or sdb.CreateCollectionSpace(csName, pageSize), need to specify the pageSize</pre>

根据名字，得到对应的 Collection，如果不存在，则创建：
<pre class="prettyprint lang-javascript">
// create collection, if collection exists get it
string clName = "TestCL";
DBCollection dbc = cs.GetCollection(clName);
if (dbc == null)
dbc = cs.CreateCollection(clName);
//or cs.createCollection(collectionName, options), create collection with some options</pre>

* 对 Collection 进行插入操作

创建需要插入的数据 BsonDocument 并插入：
<pre class="prettyprint lang-javascript">
BsonDocument insertor = new BsonDocument();
string date = DateTime.Now.ToString();
insertor.Add("operation", "Insert");
insertor.Add("date", date);
ObjectId id = dbc.Insert(insertor);</pre>

当然，BsonDocument 中还可以嵌套 BsonDocument 对象；而且你还可以直接 new 一个完整的 BsonDocument，而不需要通过 Add 方法：
<pre class="prettyprint lang-javascript">
BsonDocumentinsertor = new BsonDocument
{
    {"FirstName","John"},
    {"LastName","Smith"},
    {"Age",50},
    {"id",i},
    {"Address",
        new BsonDocument
        {
            {"StreetAddress","212ndStreet"},
            {"City","NewYork"},
            {"State","NY"},
            {"PostalCode","10021"}
        }
    },
    {"PhoneNumber",
        new BsonDocument
        {
            {"Type","Home"},
            {"Number","212555-1234"}
        }
    }
};</pre>

插入多条数据：

<pre class="prettyprint lang-javascript">
//bulkinsert
List< BsonDocument > insertor=new List < BsonDocument > ();
for(inti=0;i<10;i++)
{
BsonDocumentobj=new BsonDocument();
obj.Add("operation","BulkInsert");
obj.Add("date",DateTime.Now.ToString());
insertor.Add(obj);
}
dbc.BulkInsert(insertor,0);</pre>

* 索引的相关操作

创建索引：
<pre class="prettyprint lang-javascript">
//createindexkey,indexonattribute'Id'byASC(1)/DESC(-1)
BsonDocument key = new BsonDocument();
key.Add("id", 1);
string name = "index name";
bool isUnique = true;
bool isEnforced = true;
dbc.CreateIndex(name, key, isUnique, isEnforced);</pre>

删除索引：
<pre class="prettyprint lang-javascript">
string name = "index name";
dbc.DropIndex(name);</pre>

* 查询操作

进行查询操作，需要使用游标对查询结果进行遍历，而且可以先得到当前 Collection 的索引，如果不为空，可作为制定访问计划（hint）用于查询：
<pre class="prettyprint lang-javascript">
DBCursor icursor = dbc.GetIndex(name);
BsonDocument index = icursor.Current();</pre>

构建相应的 BsonDocument 对象用于查询，包括：查询匹配规则（matcher，包含相应的查询条件），域选择（selector），排序规则（orderBy，增序或降序），制定访问计划（hint），跳过记录个数（0），返回记录个数（-1：返回所有数据）。查询后，得到对应的 Cursor，用于遍历查询得到的结果：
<pre class="prettyprint lang-javascript">
BsonDocument matcher = new BsonDocument();
BsonDocument conditon = new BsonDocument();
conditon.Add("$gte", 0);
conditon.Add("$lte", 9);
matcher.Add("id", conditon);
BsonDocument selector = new BsonDocument();
selector.Add("id", null);
selector.Add("Age", null);
BsonDocument orderBy = new BsonDocument();
orderBy.Add("id", -1);
BsonDocument hint = null;
if (index != null)
    hint = index;
else
    hint = new BsonDocument();
DBCursor cursor = dbc.Query(matcher, selector, orderBy, hint, 0, -1);</pre>

使用 DBCursor 游标进行遍历：
<pre class="prettyprint lang-javascript">
while (cursor.Next() != null)
Console.WriteLine(cursor.Current());</pre>

* 删除操作

构建相应的 BsonDocument 对象，用于设置删除的条件：
<pre class="prettyprint lang-javascript">
//createthedeletecondition
BsonDocument drop = new BsonDocument();
drop.Add("Last Name", "Smith");
coll.Delete(drop);</pre>

* 更新操作


构建相应的 BsonDocument 对象，用于设置更新条件，你还可以创建 DBQuery 对象封装所有的查询或更新规则：
<pre class="prettyprint lang-javascript">
DBQuery query = new DBQuery();
BsonDocument updater = new BsonDocument();
BsonDocument matcher = new BsonDocument();
BsonDocument modifier = new BsonDocument();
updater.Add("Age", 25);
modifier.Add("$set", updater);
matcher.Add("First Name", "John");
query.Matcher = matcher;
query.Modifier = modifier;
dbc.Update(query);</pre>

更新操作，如果没有满足 matcher 的条件，则插入此记录：
<pre class="prettyprint lang-javascript">
dbc.Upsert(query);</pre>
SequoiaDB 的查询用 json（bson）对象表示，下表以例子的形式显示了 SQL 语句，SequoiaDB shell 语句和 SequoiaDB C# 驱动程序语法之间的对照。

+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| SQL                                                             | SequoiaDB shell                                     | C# Driver                                                |
+=================================================================+=====================================================+==========================================================+
| insert into students(a,b) values(1,-1)                          | db.foo.bar.insert({a:1,b:-1})                       | bar.insert("{'a':1,'b':-1}")                             |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select a,b from students                                        | db.foo.bar.find(null,{a:"",b:""})                   | bar.query("", "{'a':'','b':''}", "", "")                 |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select * from students                                          | db.foo.bar.find()                                   | bar.query()                                              |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select * from students where age=20                             | db.foo.bar.find({age:20})                           | bar.query("{'age':20}", "", "", "")                      |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select * from students where age=20 order by name               | db.foo.bar.find({age:20}).sort({name:1})            | bar.query("{'age':20}", "", "{'name':1}", "")            |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select * from students where age > 20 and age < 30              | db.foo.bar.find({age:{\$gt:20,$lt:30}})             | bar.query("{'age':{'\$gt':20,'\$lt':30}}", "", "", "")     |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| create index testIndex on students(name)                        | db.foo.bar.createIndex("testIndex",{name:1},false)  | bar.createIndex("testIndex", "{'name':1}", false, false) |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select * from students limit 20 skip 10                         | db.foo.bar.find().limit(20).skip(10)                | bar.query("", "", "", "", 10, 20)                        |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select count(*) from students where age > 20                    | db.foo.bar.find({age:{$gt:20}}).count()             | bar.getCount("{'age':{'$gt':20}}")                       |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| update students set a=a+2 where b=-1                            | db.foo.bar.update({$set:{a:2}},{b:-1})              | bar.update("{'b':-1}", "{'$inc':{'a':2}}", "")           |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| delete from students where a=1                                  | db.foo.bar.remove({a:1})                            | bar.delete("{'a':1}")                                    |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
此部分是相关 [C# API](api/cs/html/index.html) 文档。



## 历史更新情况：##

**Version 1.12**

（1） 添加使用SSL连接数据库的接口## 概述##

SequoiaDB Java 驱动提供了数据库操作和集群操作的接口。主要包括以下8个级别的操作：数据库，集合空间，集合，游标，副本组，节点，域，大对象。

Java 驱动的有两种类实例。一种用于数据库操作，另一种用于集群操作。

* 数据库操作实例

SequoiaDB 数据库中的数据存放分为三个级别：

 1）数据库

 2）集合空间

 3）集合

因此，在数据库操作中，可用3个类来分别表示连接，集合空间，集合实例，另2个类分别表示游标实例和大对象实例：

<table>
    <tr>
        <td>SequoiaDB</td>
        <td>数据库实例</td>
        <td>代表一个单独的数据库连接</td>
    </tr>
    <tr>
        <td>CollectionSpace</td>
        <td>集合空间实例</td>
        <td>代表一个单独的集合空间</td>
    </tr>
    <tr>
        <td>DBCollection</td>
        <td>集合实例</td>
        <td>代表一个单独的集合</td>
    </tr>
    <tr>
        <td>DBCursor</td>
        <td>游标实例</td>
        <td>代表一个查询产生的结果集</td>
    </tr>
    <tr>
        <td>DBLob</td>
        <td>大对象实例</td>
        <td>代表一个大对象</td>
    </tr>
</table>

Java 驱动需要使用不同的实例进行操作。譬如读取数据的操作需要游标实例，而创建表空间则需要数据库实例。

**Note:**

SequoiaDB 只建立一条 Socket 连接，且内部没有对网络操作加锁。如果需要多线程连接数据库，各个线程必须各自新建一个 SequoiaDB 对象及其之上的 CollectionSpace/DBCollection/DBCursor 对象。

* 集群操作实例

SequoiaDB 数据库中的集群操作分为三个级别：

1）分区组

2）数据节点

3）域

**Note:**

分区组包两种类型：编目分区组、数据分区组。

分区组实例和数据节点实例可以用以下三种类的实例表示。

<table>
    <tr>
        <td>ReplicaGroup</td>
        <td>分区组类</td>
        <td>分区组实例代表一个单独的分区组</td>
    </tr>
    <tr>
        <td>Node</td>
        <td>数据节点类</td>
        <td>数据节点实例代表一个单独的数据节点</td>
    </tr>
    <tr>
        <td>Domain</td>
        <td>域类</td>
        <td>域实例代表一个管理若干个分区组的域</td>
    </tr>
</table>


与集群相关的操作需要使用分区组及数据节点实例。

-   ReplicaGroup 的实例用于管理分区组。其操作包括启动，停止分区组，获取分区组中节点的状态，名称信息，数目信息。

-   Node 的实例用于管理节点。其操作包括启动，停止指定的节点，获取指定节点实例，获取主从节点实例，获取数据节点地址信息。

-   sdbDomain 的实例用于管理域。其包括修改域，获取域信息等操作。

## 错误信息##

* 当执行出现异常时，大部分接口都会抛出 com.sequoiadb.exception.BaseException 和 java.lang.Exception 异常，分别对应于数据库引擎返回的异常信息和客户端本地的异常信息；

* BaseException 的异常信息可以通过该类的 getErrorType，getErrorCode 和 getMessage 方法获取。
## 获取驱动开发包##

从 [SequoiaDB](http://www.sequoiadb.com) 下载对应操作系统版本的 SequoiaDB 驱动开发包；解压驱动开发包，从 driver/java/ 目录中获取 sequoiadb.jar 文件。

## 配置 Eclipse 开发环境##

（1） 将 SequoiaDB 驱动开发包中的 sequoiadb.jar 文件拷贝到工程文件目录下（建议将其放置在其他所有依赖库目录，如 lib 目录）；

（2）在 Eclipse 界面中，创建/打开开发工程；

（3）在 Eclipse 主窗口左侧的“Package Explore”窗口中，选择开发工程，并点击鼠标右键；

（4） 在菜单中选择“properties”菜单项；

（5） 在弹出的“property for project …”窗口中，选择“Java Build Path”->“Libraries”，如下图所示：

![](eclipse.jpg)

（6） 点击“Add JARs..”按钮，选择添加 sequoiadb.jar 到工程中；

（7） 点击“OK”完成环境配置。

更多操作请参考 Java 开发基础
这里介绍如何使用Java驱动接口编写使用SequoiaDB数据库的程序。为了简单起见，下面的示例不全部是完整的代码，只起示例性作用。可到安装目录/client/samples/java下获取相应的完整的代码。 更多查看 [Java API](api/java/html/index.html)

## 数据操作##

* 连接数据库：Connecting 如下是一个连接数据库，并列出所有集合信息的一个例子：

<pre class="prettyprint lang-javascript">
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
public class Sample {
  public static void main(String[] args) {
    String connString = "192.168.1.2:11810";
	  try {
		  // 建立 SequoiaDB 数据库连接
		  Sequoiadb sdb = new Sequoiadb(connString, "", "");
		  // 获取所有 Collection 信息，并打印出来
		  DBCursor cursor = sdb.listCollections();
		  while(cursor.hasNext()) {
		    System.out.println(cursor.getNext());
	    }
	  } catch (BaseException e) {
		  System.out.println("Sequoiadb driver error, error description:" + e.getErrorType());
		}
  }
}</pre>

**Note:**

（1） 本例程连接到本地数据库的11810端口是协调节点的服务端口，使用的是空的用户名和密码。用户需要根据自己的实际情况配置参数。

（2） SequoiaDB 类为非线程安全类，每个线程必须单独建立自己的 SequoiaDB 对象，不能传递给多个线程同时操作。

* 插入数据

<pre class="prettyprint lang-javascript">
String connString = "192.168.1.2:11810";
try {
  Sequoiadb sdb = new Sequoiadb(connString, "", "");
  CollectionSpace db = sdb.createCollectionSpace("space");
  DBCollection cl = db.createCollection("collection");
  // 创建一个插入的 bson 对象
  BSONObject obj = new BasicBSONObject();
  obj.put("name", "tom");
  obj.put("age", 24);
  cl.insert(obj);
} catch (BaseException e) {
	System.out.println("Sequoiadb driver error, error description:" + e.getErrorType());
}</pre>

**Note:**

本例程连接到本地数据库的11810端口是协调节点的服务端口，使用的是空的用户名很密码。用户需要根据自己的实际情况配置参数。

* 查询数据

<pre class="prettyprint lang-javascript">
// 定义一个游标对象
DBCursor cursor;
BSONObject queryCondition = new BasicBSONObject();
queryCondition = (BSONObject) JSON.parse("({age:{$ne:20}})");
// 查询所有记录，并把查询结果放在游标对象中
cursor = cl.query(queryCondition, null, null, null);
// 从游标中显示所有记录
while (cursor.hasNext()) {
  BSONObject record = cursor.getNext();
  String name = (String) record.get("name");
  System.out.println("name=" +  name);
} </pre>

**Note:**

（1） 此示例中设置了简单的查询条件，实际上还可以设置筛选条件，排序情况，及仅使用默认索引等选项。
（2） 游标对象将数据表中的部分数据缓存在本地进程的内存中，如果本地数据读取完了，游标对象会通过网络从服务器再次获取部分数据缓存在本地。

## 集群操作##

* 创建分区组

<pre class="prettyprint lang-javascript">
String connString = "192.168.1.2:11810";
try {
  Sequoiadb sdb = new Sequoiadb(connString, "", "");
  ReplicaGroup rg = sdb.createRG("group1");
  rg.createNode("dbserver-1", 11820, "/opt/sequoiadb/database/data/11820", null);
  rg.start();
} catch (BaseException e) {
  System.out.println("Sequoiadb driver error, error description" + e.getErrorType());
}</pre>

**Note:**

（1） rg.createNode() 方法的第一个参数为新增节点所在的主机名，注意这里必须是主机名（暂时不支持使用 IP 地址），第三个参数为数据文件存放路径，SequoiaDB 将自动新建该目录，但需要确保 SequoiaDB 管理员用户（默认 sdbadmin）有写权限。

（2） rg.start() 方法用于启动一个分区组的所有节点，该函数一般需要等待10秒钟左右才可完成。该方法不保证分区组选举完成，为了保证分区组可以正常使用，start 完成后，还需要等待30秒时间才可以正常使用新建的分区组。

* 在分区组增加节点
<pre class="prettyprint lang-javascript">
String connString = "192.168.1.2:11810";
try {
  Sequoiadb sdb = new Sequoiadb(connString,"","");
  ReplicaGroup rg = sdb.getReplicaGroup("group1");
  Node node = rg.createNode("dbserver-1", 11830, "/var/sequoiadb/database/data/11830", null);
  node.start();
} catch (BaseException e) {
  System.out.println("Sequoiadb driver error, error description" + e.getErrorType());
  }</pre>

**Note:**

rg.createNode() 方法的第一个参数为新增节点所在的主机名，注意这里必须是主机名（暂时不支持使用IP地址），第三个参数为数据文件存放路径，SequoiaDB将自动新建该目录，但需要确保SequoiaDB管理员用户（默认sdbadmin）有写权限。
## Java BSON 数据类型##

目前，SequoiaDB 支持多种 BSON 数据类型。详情请查看数据模型 - 文档一节。

## Java 构造 BSON 数据类型##

* 整数/浮点数

<pre class="prettyprint lang-javascript">
Java BSON 构造整数/浮点数类型// {a:123, b:3.14}
BSONObject obj = new BasicBSONObject();
obj.put("a", 123);
obj.put("b", 3.14);</pre>

* 字符串

<pre class="prettyprint lang-javascript">
Java BSON 构造字符串类型// {a:"hi"}
BSONObject obj = new BasicBSONObject();
obj.put("a", "hi");</pre>

* 空类型

<pre class="prettyprint lang-javascript">
Java BSON 构造空类型// {a:null}
BSONObject obj = new BasicBSONObject();
obj.put("a", null);</pre>

* 对象

<pre class="prettyprint lang-javascript">
Java BSON 构造嵌套对象类型// {b:{a:1}}
BSONObject subObj = new BasicBSONObject();
subObj.put("a", 1);
BSONObject obj = new BasicBSONObject();
obj.put("b", subObj);</pre>

* 数组

<pre class="prettyprint lang-javascript">
Java BSON 使用 org.bson.types.BasicBSONList 来构造数组类型// {a:[0,1,2]}
BSONObject obj = new BasicBSONObject();
BSONObject arr = new BasicBSONList();
arr.put("0", 0);
arr.put("1", 1);
arr.put("2", 2);
obj.put("a", arr);</pre>

* 布尔

<pre class="prettyprint lang-javascript">
Java BSON 构造布尔类型// {a:true, b:false}
BSONObject obj = new BasicBSONObject();
obj.put("a", true);
obj.put("b", false);</pre>

* 对象 ID

Java BSON 使用 org.bson.types.ObjectId 来生成每条记录的“\_id”字段内容。Java BSON 12 字节的 ObjectId 与[文档](SdbDoc_Cn/data_model/document.html)一节介绍的对象 ID 略有不同，目前，Java ObjectId 的12字节内容由三部分组成：4字节精确到秒的时间戳，4字节系统（物理机）标示，4字节由随机数起始的序列号。默认情况下，数据库为每条记录生成一个字段名为“\_id”的唯一对象 ID。

<pre class="prettyprint lang-javascript">
BSONObject obj = new BasicBSONObject();
ObjectId id1 = new ObjectId();
ObjectId id2 = new ObjectId("53bb5667c5d061d6f579d0bb");
obj.put("_id", id1);</pre>

* 正则表达式

Java BSON 使用 java.util.regex.Pattern 来构造正则表达式数据类型。

<pre class="prettyprint lang-javascript">
BSONObject matcher = new BasicBSONObject();
Pattern obj = Pattern.compile("^2001",Pattern.CASE_INSENSITIVE);
matcher.put("serial_num", obj);
BSONObject modifier = new BasicBSONObject("$set", new BasicBSONObject("count",1000));
cl.update(matcher, modifier, null);</pre>

以上使用正则表达式构造了一个匹配条件，将序列号以“2001”开头的记录的“count”字段内容改为“1000”。

**Note:**

以上使用 Patten 构造的 bson matcher，当使用 matcher.toString()，内容为：
<pre class="prettyprint lang-diy">
{ "serial_num" : { "$options" : "i" , "$regex" : "^2001"}}</pre>

通过以下 bson 构造方式也可以得到相同的内容：

<pre class="prettyprint lang-javascript">
BSONObject matcher2 = new BasicBSONObject();
BSONObject obj2 = new BasicBSONObject();
obj2.put("$regex","^2001");
obj2.put("$options","i");
matcher2.put("serial_num", obj2);</pre>

但是，通过后者构造出的 matcher2 的数据类型是一个普通的对象嵌套类型，而不是正则表达式类型。

* 日期

Java BSON 使用 java.util.Date 来构造日期类型。

<pre class="prettyprint lang-javascript">
BSONObject obj = new BasicBSONObject();
Date now = new Date();
obj.put("date", now);</pre>

* 二进制

Java BSON 使用 org.bson.types.Binary 来构造二进制类型。

<pre class="prettyprint lang-javascript">
BSONObject obj = new BasicBSONObject();
String str = "hello world";
byte[] arr = str.getBytes();
Binary bindata = new Binary(arr);
obj.put("bindata", bindata);</pre>

* 时间戳

Java BSON 使用 org.bson.types.BSONTimestamp 来构造时间戳类型。

<pre class="prettyprint lang-javascript">
String mydate = "2014-07-01 12:30:30.124232";
String dateStr = mydate.substring(0, mydate.lastIndexOf('.'));
String incStr = mydate.substring(mydate.lastIndexOf('.') + 1);
        
SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
Date date = format.parse(dateStr);
int seconds = (int)(date.getTime()/1000);
int inc = Integer.parseInt(incStr);
BSONTimestamp ts = new BSONTimestamp(seconds, inc);
        
BSONObject obj = new BasicBSONObject();
obj.put("timestamp", ts);</pre>
Java 驱动的 Datasource 提供给用户一个快速获取有效连接实例的途径。

## 连接池用法##

使用类 SequoiadbDatasource 的 getConnection 方法从连接池中获取一个连接，使用 close 方法把取出的连接放回连接池。当连接池使用的连接数到达连接上限时，下一个请求连接的操作将会等待一段时间，若在规定的时间内无空闲的连接可用，将抛出异常。类 ConfigOptions 可以设置连接的各项参数。类 SequoiadbOption 中可以设置连接池的各种参数。

详情请查看相关 [Java API](api/java/html/index.html) 介绍。

## 例子##
<pre class="prettyprint lang-javascript">
SequoiadbDatasource ds = null;
Sequoiadb db = null;
ArrayList&lt;String&gt; urls = new ArrayList&lt;String&gt;();
ConfigOptions nwOpt = new ConfigOptions();        // 定义连接选项
SequoiadbOption dsOpt = new SequoiadbOption();	  // 定义连接池选项

urls.add("ubuntu-dev1:11810");
urls.add("ubuntu-dev2:11810");
urls.add("ubuntu-dev3:11810");

nwOpt.setConnectTimeout(500);                     // 设置若连接失败，超时时间（ms）
nwOpt.setMaxAutoConnectRetryTime(0);	          // 设置若连接失败，重试次数

// 以下设置的都是 SequoiadbOption 的默认值
dsOpt.setMaxConnectionNum(500);                   // 设置连接池最大连接数
dsOpt.setInitConnectionNum(10);                   // 初始化连接池时，创建连接的数量
dsOpt.setDeltaIncCount(10) ;                      // 当池中没有可用连接时，增加连接的数量
dsOpt.setMaxIdeNum(10);                           // 周期清理多余的空闲连接时，应保留连接的数量
dsOpt.setTimeout(5 * 1000);                 // 当已使用的连接数到达设置的最大连接数时（500），请求连接的等待时间。
dsOpt.setAbandonTime(10 * 60 * 1000);             // 连接存活时间，当连接空闲时间超过连接存活时间，将被连接池丢弃
dsOpt.setRecheckCyclePeriod(1 * 60 * 1000);       // 清除多余空闲连接的周期
dsOpt.setRecaptureConnPeriod(10 * 60 * 1000);     // 检测并取回异常地址的周期

ds = new SequoiadbDatasource(urls, "", "", nwOpt, dsOpt); // 创建连接池
db = ds.getConnection();                                  // 从连接池获取连接
// do something else                                      // 使用连接进行业务操作
ds.close(db);</pre>
SequoiaDB 的查询用 json（bson）对象表示，下表以例子的形式显示了 SQL 语句，SequoiaDB shell 语句和 SequoiaDB Java 驱动程序语法之间的对照。

+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| SQL                                                             | SequoiaDB shell                                     | Java Driver                                              |
+=================================================================+=====================================================+==========================================================+
| insert into students(a,b) values(1,-1)                          | db.foo.bar.insert({a:1,b:-1})                       | bar.insert("{'a':1,'b':-1}")                             |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select a,b from students                                        | db.foo.bar.find(null,{a:"",b:""})                   | bar.query("", "{'a':'','b':''}", "", "")                 |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select * from students                                          | db.foo.bar.find()                                   | bar.query()                                              |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select * from students where age=20                             | db.foo.bar.find({age:20})                           | bar.query("{'age':20}", "", "", "")                      |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select * from students where age=20 order by name               | db.foo.bar.find({age:20}).sort({name:1})            | bar.query("{'age':20}", "", "{'name':1}", "")            |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select * from students where age > 20 and age < 30              | db.foo.bar.find({age:{\$gt:20,$lt:30}})             | bar.query("{'age':{'\$gt':20,'\$lt':30}}", "", "", "")     |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| create index testIndex on students(name)                        | db.foo.bar.createIndex("testIndex",{name:1},false)  | bar.createIndex("testIndex", "{'name':1}", false, false) |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select * from students limit 20 skip 10                         | db.foo.bar.find().limit(20).skip(10)                | bar.query("", "", "", "", 10, 20)                        |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select count(*) from students where age > 20                    | db.foo.bar.find({age:{$gt:20}}).count()             | bar.getCount("{'age':{'$gt':20}}")                       |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| update students set a=a+2 where b=-1                            | db.foo.bar.update({$set:{a:2}},{b:-1})              | bar.update("{'b':-1}", "{'$inc':{'a':2}}", "")           |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| delete from students where a=1                                  | db.foo.bar.remove({a:1})                            | bar.delete("{'a':1}")                                    |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
此部分是相关 [Java API](api/java/html/index.html) 文档。

## 历史更新情况：##

**Version 1.10**

（1） DBCollection 类新添加的接口：

    createLob，创建一个大对象
    openLob，打开一个已存在的大对象
    removeLob，删除一个大对象
    listLobs，列出所有大对象
    explain，获取执行访问计划

（2） 新增大对象类 DBLob，用于操作大对象：

    write，向一个大对象写入数据
    read，从大对象中读取数据
    seek，指定读取数据的偏移
    close，关闭一个大对象
    getID，获取大对象的标识ID
    getSize，获取大对象的大小
    getCreateTime，获取大对象的创建时间

**Version 1.8**

（1） Sequoiadb 类新添加的接口：

    isValid，判断当前连接是否有效
    createCollectionSpace，提供一个 BSONObject 的选项，使创建集合空间更加灵活
    backupOffline，离线备份支持更多的选项
    evalJS，执行 js 代码
    createDomain，创建域
    getDomain，获取域
    dropDomain，删除域
    isDomainExist，域是否存在
    listDomain，列出所有域

（2） DBCollection 类新添加的接口：

    alterCollection，修改集合（表）属性
    setMainKeys，设置主键。此接口只与 save 接口配合使用，它设置的主键并不对其他接口起作用
    save，可使用默认的主键"_id"或者指定其他主键，同时插入或更新多条记录

（3） 添加 Domain 类用于与域相关的操作

（4） SequoiadbDatasource类新添加的接口：

    SequoiadbDatasource，可提供多个地址的构造器，便于机器负载均衡
    getIdleConnNum，获取当前可用的连接数量
    getUsedConnNum，获取当前已使用的连接数量
    getNormalAddrNum，获取当前正常的地址数量
    getAbnormalAddrNum，获取当前异常的地址数量

（5） SequoiadbOption 类新添加接口：

    setRecaptureConnPeriod，设置周期检测异常地址是否重新可用的时间
    getRecaptureConnPeriod，获取周期检测异常地址是否重新可用的时间

**Version 1.6**

（1） 添加类 Node 来取代原来的类 ReplicaGroup。类 ReplicaNode 以及与它们相关的方法将在 version 2.x 中被弃用。

详情请查看相关 [Java API](api/java/html/index.html)。
## 概述##

SequoiaDB PHP 驱动提供了数据库操作和集群操作的 PHP 接口。数据库操作包括数据库的连接，用户的创建删除，数据的增删改查，索引的创建删除，快照的获取与重置，以及集合与集合空间的创建删除操作等操作。集群操作包括管理分区组和数据节点的各种操作，譬如启动，停止分区组，启动，停止数据节点，获取主从数据节点，集合分区等。

PHP 驱动的有两种类实例。一种用于数据库操作，另一种用于集群操作。

* 数据库操作实例

SequoiaDB 数据库中的数据存放分为三个级别：

1）数据库

2）集合空间

3）集合

因此，在数据库操作中，可用3个类来分别表示连接，集合空间，集合实例，另1个类表示游标实例：

<table>
        <tr>
            <td>SequoiaDB</td>
            <td>数据库类</td>
        </tr>
        <tr>
            <td>SequoiaCS</td>
            <td>集合空间类</td>
        </tr>
        <tr>
            <td>SequoiaCollection</td>
            <td>集合类</td>
        </tr>
        <tr>
            <td>SequoiaCursor</td>
            <td>游标类</td>
        </tr>
</table>


PHP 驱动需要使用不同的实例进行操作。譬如读取数据的操作需要游标实例，而创建表空间则需要数据库实例。

* 集群操作实例

SequoiaDB 数据库中的集群操作分为两个级别：1）分区组 2）数据节点

**Note:**
分区组包三种类型：协调分区组，编目分区组，数据分区组。

分区组实例和数据节点实例可以用以下两种类的实例表示。

<table>
        <tr>
            <td>SequoiaGroup</td>
            <td>分区组类</td>
            <td>分区组实例代表一个单独的分区组</td>
        </tr>
        <tr>
            <td>SequoiaNode</td>
            <td>数据节点类</td>
            <td>数据节点实例代表一个单独的数据节点</td>
        </tr>
</table>

与集群相关的操作需要使用分区组及数据节点实例。

-   SequoiaGroup 的实例用于管理分区组。其操作包括启动，停止分区组，获取分区组中节点的状态，名称信息，数目信息。

-   SequoiaNode 的实例用于管理节点。其操作包括启动，停止指定的节点，获取指定节点实例，获取主从节点实例，获取数据节点地址信息。

## 错误信息##

* 一个函数被成功调用则返回 true（或整型1），否则返回值为 false（或整型0）
* 如果用户需要知道详细的错误信息，可以调用 getError() 获取错误信息，如果没有错误，则会输出“No error message”
##获取驱动开发包##

从 [SequoiaDB](http://www.sequoiadb.com) 下载对应操作系统版本的 SequoiaDB 驱动开发包；解压驱动开发包，从 driver/lib/phplib/libsdbphp-x.x.x.so（x.x.x 为版本号，请根据 PHP 版本选择，前两位需相同版本，第三位版本要小于或等于 PHP 的版本）文件。

##数据操作##

-   Linux
    **准备工作：**安装 Apache 和 PHP 环境，PHP 要求5.3.3及以上版本
    **配置步骤：**

    1. 打开 /etc/php5/apache2/php.ini 文件；
      
    2. 在该文件的 [PHP] 配置段中新增如下行：

<pre class="prettyprint lang-diy">
extension=&lt;PATH&gt;/libsdbphp-x.x.x.so </pre>

      其中 PATH 为 libsdbphp-x.x.x.so 文件放置路径。

    3. 保存关闭文件；

    4. 重新启动 apache2 服务；

<pre class="prettyprint lang-javascript">
$ service apache2 restart（SUSE/Redhat） 或  service httpd restart（CentOS）</pre>

    5. 编写包含如下内容 PHP 测试脚本，包存为 test.php 文件，并放在在 Web 服务目录下；

<pre class="prettyprint lang-javascript">
&lt;?php phpinfo(); ?&gt;</pre>

    6. 通过浏览器打开 http://localhost/test.php ，在打开的页面中查看是否包含 SequoiaDB 模块。

-   Windows

    暂未提供 Windows 驱动开发包
## 获取驱动开发包##

这里介绍如何使用 PHP 驱动接口编写使用 SequoiaDB 数据库的程序。为了简单起见，下面的示例不全部是完整的代码，只起示例性作用。可到安装目录 /client/samples/php 下获取相应的完整的代码。更多查看 [PHP API](api/php/html/index.html)

## 数据操作##

* 连接数据库

<pre class="prettyprint lang-javascript">
//创建 SequoiaDB 对象
$db = new Sequoiadb();
//连接数据库
$array = $db -> connect("localhost:11810");
//检验连接结果，返回的默认是 php 数组类型，数据是 array(0){"errno"=>0}
//如果 errno 为0，那么连接成功
if($array['errno'] !=0 )
{
  exit();
}</pre>

* 选择集合空间

<pre class="prettyprint lang-javascript">
//选择名称为"foo"的集合空间，如果不存在，则自动创建
//返回 SequoiaCS 对象
$cs = $db -> selectCs("foo");
//检验结果，如果成功返回对象，失败返回 NULL
if( empty($cs) )
{
  exit();
}</pre>

* 选择集合

<pre class="prettyprint lang-javascript">
//选择名称为"big"的集合，如果不存在，则自动创建
//返回 SequoiaCollection 对象
$cl = $cs -> selectCollection("big");
//检验结果，如果成功返回对象，失败返回 NULL
if( empty($cl) )
{
  exit();
}</pre>

* 插入

<pre class="prettyprint lang-javascript">
//插入 json
$arr = $cl -> insert("{test:1}");
//检测结果
if($array['errno'] !=0 )
{
  exit();
}
//插入数组
$arr = $cl -> insert(array("test">=2));
//检测结果
if($arr['errno'] !=0 )
{
  exit();
}</pre>

* 查询

<pre class="prettyprint lang-javascript">
//查询集合中的所有记录
//返回 SequoiaCursor 对象
$cursor = $cl -> find();
//遍历所有记录
while($record = $cursor -> getNext())
{
  var_dump($record);
}</pre>

* 更新

<pre class="prettyprint lang-javascript">
//修改集合中的多有记录，把字段 test 的值修改为0
$arr = $cl -> update("{$set:{test:0}}");
//检测结果
if($arr['errno'] !=0 )
{
  exit();
}</pre>

* 删除

<pre class="prettyprint lang-javascript">
//删除集合中的所有记录
$arr = $cl -> remove();
//检测结果
if($arr['errno'] !=0 )
{
  exit();
}</pre>

## 集群操作##

* 选择组

<pre class="prettyprint lang-javascript">
//选择名称为"group"的组，如果不存在，则自动创建
//返回 SequoiaGroup 对象
$group = $db -> selectGroup("group");
//检验结果，如果成功返回对象，失败返回 NULL
if( empty($group) )
{
  exit();
}</pre>

* 启动分区组

<pre class="prettyprint lang-javascript">
//启动分区组，首次会自动激活
//返回操作信息
$arr = $group -> start() ;
//检查结果
If ( $arr['errno'] != 0 )
{
  Exit() ;
}</pre>

* 选择节点

<pre class="prettyprint lang-javascript">
//获取名称为"node"的节点
//返回 SequoiaNode 对象
$node = $group -> getNode( 'node') ;
//检查对象是否空
If ( empty( $node ) )
{
  Exit() ;
}</pre>
SequoiaDB 的查询用 json（bson）对象表示，下表以例子的形式显示了 SQL 语句，SequoiaDB shell 语句和 SequoiaDB PHP 驱动程序语法之间的对照。

+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+
| SQL                                                             | SequoiaDB shell                                     | PHP Driver                                     |
+=================================================================+=====================================================+=================================================+
| insert into students(a,b) values(1,-1)                          | db.foo.bar.insert({a:1,b:-1})                       | $bar->insert( "{a:1,b:-1}" )                    |
+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+
| select a,b from students                                        | db.foo.bar.find(null,{a:"",b:""})                   | $bar->find(NULL,'{a:"",b:""}                    |
+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+
| select * from students                                          | db.foo.bar.find()                                   | $bar->find()                                    |
+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+
| select * from students where age=20                             | db.foo.bar.find({age:20})                           | $bar->find("{age:20}")                          |
+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+
| select * from students where age=20 order by name               | db.foo.bar.find({age:20}).sort({name:1})            | $bar->find("{'age':20}", "", "{'name':1}", "")  |
+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+
| select * from students where age > 20 and age < 30              | db.foo.bar.find({age:{\$gt:20,$lt:30}})             | $bar->find("{age:20}",NULL,"{name:1}")          |
+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+
| create index testIndex on students(name)                        | db.foo.bar.createIndex("testIndex",{name:1},false)  | $bar->find("{age:{$gt:20,$lt:30}}")             |
+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+
| select * from students limit 20 skip 10                         | db.foo.bar.find().limit(20).skip(10)                | $bar->createIndex("{name:1}","testIndex",false) |
+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+
| select count(*) from students where age > 20                    | db.foo.bar.find({age:{$gt:20}}).count()             | $bar->find(NULL,NULL,NULL,NULL,10,20)           |
+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+
| update students set a=a+2 where b=-1                            | db.foo.bar.update({$set:{a:2}},{b:-1})              | $bar->update("{$inc:{a:2}}","{b:-1}")           |
+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+
| delete from students where a=1                                  | db.foo.bar.remove({a:1})                            | $bar->remove("{a:1}")                           |
+-----------------------------------------------------------------+-----------------------------------------------------+-------------------------------------------------+此部分是相关 [PHP API](api/php/html/index.html) 文档。


## 概述##

Python 客户端驱动提供了数据库操作和集群操作的接口。数据库操作包括数据库的连接，用户的创建删除，数据的增删改查，索引的创建删除，快照的获取与重置，以及集合与集合空间的创建删除操作等操作。集群操作包括管理分区组和数据节点的各种操作，譬如启动、停止分区组，启动、停止数据节点，获取主从数据节点，集合分区等。更多参考 [Python API](api/python/html/index.html)

Python 客户端驱动的有两种类实例。一种用于数据库操作，另一种用于集群操作。

* 数据库操作实例

SequoiaDB 数据库中的数据存放分为三个级别：

1）数据库

2）集合空间

3）集合

因此，在数据库操作中，可用3个类来分别表示连接，集合空间，集合实例，另1个类表示游标实例：

<table>
    <tr>
        <td>client</td>
        <td>数据库类</td>
        <td>连接实例代表一个单独的数据库连接</td>
    </tr>
    <tr>
        <td>collectionspace</td>
        <td>集合空间类</td>
        <td>集合空间实例代表一个单独的集合空间</td>
    </tr>
    <tr>
        <td>collection</td>
        <td>集合类</td>
        <td>集合实例代表一个单独的集合</td>
    </tr>
    <tr>
        <td>cursor</td>
        <td>游标类</td>
        <td>游标实例代表一个查询产生的游标</td>
    </tr>
</table>

Python 客户端需要使用不同的实例进行操作。譬如读取数据的操作需要游标实例，而创建表空间则需要数据库实例。

* 集群操作实例

SequoiaDB数据库中的集群操作分为两个级别：1）分区组 2）数据节点

Note: 分区组包三种类型：协调分区组，编目分区组，数据分区组。

分区组实例和数据节点实例可以用以下两种类的实例表示。

<table>
    <tr>
        <td>replicagroup</td>
        <td>分区组类</td>
        <td>分区组实例代表一个单独的分区组</td>
    </tr>
    <tr>
        <td>replicaode</td>
        <td>数据节点类</td>
        <td>数据节点实例代表一个单独的数据节点</td>
    </tr>
</table>

与集群相关的操作需要使用分区组及数据节点实例。

-   replicagroup 的实例用于管理分区组。其操作包括启动，停止分区组，获取分区组中节点的状态，名称信息，数目信息。

-   replicanode 的实例用于管理节点。其操作包括启动，停止指定的节点，获取指定节点实例，获取主从节点实例，获取数据节点地址信息。

## 错误信息##

每个函数都有返回值，返回值的定义如下：

SDB_OK（数据值为0）：表示执行成功；

< 0 ：表示数据库错误，具体的错误描述在 err.prop 文件中可以找到，也可以用 pysequoiadb.getErr(error_no) 获取；

\> 0 ：表示系统错误，请查阅相关系统的错误码信息。
## 获取驱动开发包##

从 [SequoiaDB](http://www.sequoiadb.com) 下载对应操作系统版本的 SequoiaDB 驱动开发包。

## 配置开发环境##

* Linux

编译：

编译需要 scons 环境，中确保 scons 已经安装后，切换到 driver/python 目录，执行 scons。scons 执行完毕，会自动打包生成 **pysequoiadb.tar.gz** 压缩包。

安装：

（1） 解压下来的驱动开发包；得到 python 目录。

（2） 将 python 目录下的 bson 和 pysequoiadb 目录拷贝到开发工程目录中（建议放在第三方库目录下）。

* Windows

暂未推出 Windows 驱动开发包。
本节介绍使用 Python 运行 SequoiaDB。首先安装 SequoiaDB，安装信息请查看 SequoiaDB 服务器安装章节。

这里介绍如何使用 Python 客户端驱动接口编写使用 SequoiaDB 数据库的程序。为了简单起见，下面的示例不全部是完整的代码，只起示例性作用。可到 /sequoiadb/client/samples/python 下获取相应的完整的代码。更多查看 [Python API](api/python/html/index.html)

## 数据库操作##

* 数据库连接（Connecting）

connect.py 演示如何连接到数据库。文件应当 import “pysequoiadb”中的 client，const 等模块，以及 error 模块中的 SDBBaseError 类。

<pre class="prettyprint lang-javascript">
import pysequoiadb
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import SDBBaseError

# connect to local db, using default args value.
# host= 'localhost', port= 11810, user= '', password= ''
try:
  db = client()
except DBBaseError, e:
  pysequoiadb._print(e)
  del db
  exit()

# if no error occurs, connect to specified server successfully
print 'Connect success'
db.disconnect()
# Need to release client whether it connected db server successfully or not
del db</pre>

在 Linux 下，可以直接运行 python 解释执行 connect.py。

**Note:**

本例程连接到本地数据库的11810端口，使用的是空的用户名和密码。用户需要根据自己的实际情况配置参数。譬如，将上述代码中的 `db = client()` 修改为 `db = client('192.168.10.188', 11810)`。当数据库已经创建用户时，应该使用正确的用户及密码连接到数据库，否则连接失败。

* 创建集合空间和集合

以下创建了一个名字为“foo”的集合空间和一个名字为“bar”的集合，集合空间内的集合的数据页大小为16k。可根据实际情况选择不同大小的数据页。创建集合后，可对集合做增删改查等操作。

<pre class="prettyprint lang-javascript">
# 连接到数据库
try:
  db = client()
except SDBBaseError, e:
  pysequoiadb._print(e)

# success to connect to db
try:
cs_name = 'foo'
  cs = db.create_collection_space(cs_name, {PageSize:16384}) except SDBBaseError, e:
pysequoiadb._print(e)

# success to create collection space
cl_name = 'bar'
try:
  cl = cs.create_collection(cl_name)
except SDBBaseError, e:
pysequoiadb._print(e)</pre>

* 插入数据（insert）
<pre class="prettyprint lang-javascript">
# 首先，需要创建一个插入的 dict 对象。
record = {"name":"Tom", "age":24}
# 接着，把此 dict 对象插入集合中
oid = cl.insert ( record ) ;</pre>

record 为输入参数，为要插入的数据。dict 对象将会被转换成 bson 插入到集合中。oid 是插入该记录，返回的 bson 结构的 objectid。

* 查询（query）

<pre class="prettyprint lang-javascript">
# 查询所有记录，把结果放入游标中，并循环打印游标中的每条记录
try:
  cr = cl.query()
  while True:
    try:
      record = cr.next()
    except SDBEndOfCursor:
      break
    except SDBBaseError, e:
      pysequoiadb._print(e)</pre>

查询操作需要一个游标对象存放查询的结果到本地。要获得查询的结果需要使用游标操作。本例使用了游标操作的 next 接口，表示从查询结果中取到一条记录。此示例中没有设置查询条件，筛选条件，排序情况，及仅使用默认索引。

* 索引（index）

<pre class="prettyprint lang-javascript">
index_name = "index_name"
# 首先创建一个 dict 对象包含将要创建的索引的信息
idx = { 'name':1, 'age':-1 }
# 创建一个以"name"为升序，"age"为降序的索引
cl.create_index ( idx, index_name, FALSE, FALSE ) ;</pre>

集合对象 collection 中创建一个以“name”为升序，“age”为降序的索引。

* 更新（update）

<pre class="prettyprint lang-javascript">
# 先创建一个包含更新规则的 BSONObj 对象
rule = {"$set":{ "age":19}}
# 打印出更新规则
print rule
# 更新记录
cl.update( rule )</pre>

在集合对象 collection 中更新了记录。实例中没有指定数据匹配规则，所以此示例将更新集合中所有的集合。

## 集群操作##

分区组操作包括创建分区组（client::creat_replica_group），得到分区组实例（client:: get_replica_group_by_name 和 client:: get_replica_group_by_id），启动分区组所有数据节点（replicagroup::start），停止分区组所有数据节点（replicagroup::stop）等。

以下为分区组操作示例性的例子。真正的应用应包括错误检测等。
<pre class="prettyprint lang-javascript">
# 定义一个空的 map 对象表示创建数据节点没有更多的配置内容
config = {}
...

# 先建立一个编目分区组
db.create_cata_replica_group ( HOST_NAME, SERVICE_NAME, CATALOG_GROUP_PATH , None)

# 创建数据分区组
rg = db.create_replica_group ( REPLICA_GROUP_NAME)

# 创建第一个数据节点
rg.create_node ( HOST_NAME1, SERVICE_NAME1, DATABASE_PATH1, config )
...

# 启动分区组
rg.start ()</pre>

* 数据节点操作

数据节点操作包括创建数据节点（replicagroup::create_node），得到主数据节点（replicagroup::get_master），得到从数据节点（replicagroup::get_slave），启动数据节点（replicanode::start），停止数据节点（replicanode::stop）等。

以下为数据节点操作示例性的例子。真正的应用应包括错误检测等。

<pre class="prettyprint lang-javascript">
# 获取主数据节点
master = rg.get_master() ;

# 获取从数据节点
slave = rg.get_slave() ;</pre>
SequoiaDB 的查询用 dict（bson）对象表示，下表以例子的形式显示了 SQL 语句，SequoiaDB shell 语句和 SequoiaDB Python 驱动程序语法之间的对照。

+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| SQL                                                             | SequoiaDB shell                                     | Python Driver                                            |
+=================================================================+=====================================================+==========================================================+
| insert into students(a,b) values(1,-1)                          | db.foo.bar.insert({a:1,b:-1})                       |                                                          |
|                                                                 |                                                     | -    cl = collection()                                   |
|                                                                 |                                                     | -    obj = { "a":1, "b":-1 }                             |
|                                                                 |                                                     | -    cl.insert( obj )                                    |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
| select a,b from students                                        | db.foo.bar.find(null,{a:"",b:""})                   |                                                          |
|                                                                 |                                                     | -    cl = collection()                                   |
|                                                                 |                                                     | -    obj = {}                                            |
|                                                                 |                                                     | -    selected = { "a":"","b":"" }                        |
|                                                                 |                                                     | -    cr = cl.query(obj, selected )                       |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
|                                                                 |                                                     |                                                          |
| select * from students                                          | db.foo.bar.find()                                   | -    cl = collection()                                   |
|                                                                 |                                                     | -    cr = cl.query ()                                    |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
|                                                                 |                                                     |                                                          |
| select * from students where age=20                             | db.foo.bar.find({age:20})                           | -    cl = collection()                                   |
|                                                                 |                                                     | -    condition ={"age":20}                               |
|                                                                 |                                                     | -    cr = cl.query ( condition )                         |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
|                                                                 |                                                     |                                                          |
| select * from students where age=20 order by name               | db.foo.bar.find({age:20}).sort({name:1})            | -    cl = collection()                                   |
|                                                                 |                                                     | -    condition ={"age":20}                               |
|                                                                 |                                                     | -    orderBy = {"name":1}                                |
|                                                                 |                                                     | -    cr = cl .query (condition , None, orderBy , None )  |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
|                                                                 |                                                     |                                                          |
| select * from students where age > 20 and age < 30              | db.foo.bar.find({age:{\$gt:20,$lt:30}})             | -    cl = collection()                                   |
|                                                                 |                                                     | -    condition = {"age":{"$gt":20","$lt":30}}            |
|                                                                 |                                                     | -    cr = cl .query (condition )                         |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
|                                                                 |                                                     |                                                          |
| create index testIndex on students(name)                        | db.foo.bar.createIndex("testIndex",{name:1},false)  | -    cl = collection()                                   |
|                                                                 |                                                     | -    obj = { "name":1 }                                  | 
|                                                                 |                                                     | -    cl.create_index ( &obj, "testIndex", FALSE, FALSE ) |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
|                                                                 |                                                     |                                                          |
| select * from students limit 20 skip 10                         | db.foo.bar.find().limit(20).skip(10)                | -    cl = collection()                                   |
|                                                                 |                                                     | -    cr = cl .query (None, None, None, None, 10, 20 )    |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
|                                                                 |                                                     |                                                          |
| select count(*) from students where age > 20                    | db.foo.bar.find({age:{$gt:20}}).count()             | -    cl = collection()                                   |
|                                                                 |                                                     | -    count = 0L                                          |
|                                                                 |                                                     | -    condition = { "age":{"$gt":20}}                     |
|                                                                 |                                                     | -    count = cl.get_count (condition )                   |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
|                                                                 |                                                     |                                                          |
| update students set a=a+2 where b=-1                            | db.foo.bar.update({$set:{a:2}},{b:-1})              | -    cl = collection()                                   |
|                                                                 |                                                     | -    condition = { "b":1 }                               |
|                                                                 |                                                     | -    rule = { "$set":{"a":2} }                           |
|                                                                 |                                                     | -    cl.update ( rule, condition, None )                 |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
|                                                                 |                                                     |                                                          |
| delete from students where a=1                                  | db.foo.bar.remove({a:1})                            | -    cl = collection()                                   |
|                                                                 |                                                     | -    condition = {"a":1}                                 |
|                                                                 |                                                     | -    cl.delete ( condition )                             |
+-----------------------------------------------------------------+-----------------------------------------------------+----------------------------------------------------------+
此部分是相关 [Python API](api/python/html/index.html) 文档。

## 历史更新情况：##

**Version 1.10**

（1） 新增接口类 lob：

-   close，关闭创建的lob对象，用以刷新数据
-   read，可从lob对象中读取数据
-   write，可把数据写入lob
-   seek，可跳转到到指定数据位置
-   get_oid，可获取lob对象的oid
-   get_size，可获取lob对象的大小(bytes)
-   get_create_time，可获取lob对象的创建时间

（2） collection 新增接口：

-   create_lob，可在当前的collection中创建一个lob对象
-   remove_lob，可在当前的collection中删除指定lob对象
-   get_lob，可获取当前collection中指定oid的lob对象
-   list_lobs，可列出当前collection中所有的lob

详情请查看相关 [Python API](api/python/html/index.html)。
协调节点（coord）和数据节点（data）对外提供 REST 接口访问，同时支持 HTTP 和 HTTPS 协议。

##通用请求头##

                   说明                                   例子
  ---------------- -------------------------------------- -------------------------------------------------
  Content-Type     请求内容的类型                         application/x-www-form-urlencoded;charset=UTF-8
  Content-Length   请求内容的长度                         54
  Host             主机名（协调节点或数据节点的服务地址）   192.168.1.214:11814

<pre class="prettyprint lang-diy">
POST / HTTP/1.0
Content-Type: application/x-www-form-urlencoded;charset=UTF-8
Content-Length: 54
Host: 192.168.1.214:11814</pre>

##通用响应头##

                   说明             例子
  ---------------- ---------------- -----------
  Content-Type     响应内容的类型   text/html
  Content-Length   响应内容的长度   54

<pre class="prettyprint lang-diy">
HTTP/1.1 200 Ok
Content-Length: 35
Content-Type: text/html</pre>
##创建集合空间##

+----------+------------------------------------------+--------------------------------------------------------------------+
|          | 说明                                     | 例子                                                               |
+==========+==========================================+====================================================================+
| 请求头   | 同通用请求头                             |                                                                    |
+----------+------------------------------------------+--------------------------------------------------------------------+
| 请求内容 | cmd: create collectionspace<br>          | cmd=create collectionspace&name=cs                                 |
|          | name: 集合空间名字                       |                                                                    |
+----------+------------------------------------------+--------------------------------------------------------------------+
| 说明     |                                          |                                                                    |
+----------+------------------------------------------+--------------------------------------------------------------------+
| 响应头   | 同通用响应头                             |                                                                    |
+----------+------------------------------------------+--------------------------------------------------------------------+
| 响应内容 | {<br>                                    | { "errno": -33, "description": "Collection space already exists" } |
|          | errno: 返回值，0表示成功，其他为失败<br> |                                                                    |
|          | description: 失败时的错误描述<br>        |                                                                    |
|          | }                                        |                                                                    |
+----------+------------------------------------------+--------------------------------------------------------------------+
| 说明     |                                          |                                                                    |
+----------+------------------------------------------+--------------------------------------------------------------------+

##删除集合空间##

+----------+------------------------------------------+----------------------------------+
|          | 说明                                     | 例子                             |
+==========+==========================================+==================================+
| 请求头   | 同通用请求头                             |                                  |
+----------+------------------------------------------+----------------------------------+
| 请求内容 | cmd: drop collectionspace<br>            | cmd=drop collectionspace&name=cs |
|          | name: 集合空间名字（集合空间.集合）      |                                  |
+----------+------------------------------------------+----------------------------------+
| 说明     |                                          |                                  |
+----------+------------------------------------------+----------------------------------+
| 响应头   | 同通用响应头                             |                                  |
+----------+------------------------------------------+----------------------------------+
| 响应内容 | {<br>                                    | { "errno": 0 }                   |
|          | errno: 返回值，0表示成功，其他为失败<br> |                                  |
|          | description: 失败时的错误描述 <br>       |                                  |
|          | }                                        |                                  |
+----------+------------------------------------------+----------------------------------+
| 说明     |                                          |                                  |
+----------+------------------------------------------+----------------------------------+

##创建集合##

+----------+------------------------------------------+-----------------------------------+
|          | 说明                                     | 例子                              |
+==========+==========================================+===================================+
| 请求头   | 同通用请求头                             |                                   |
+----------+------------------------------------------+-----------------------------------+
| 请求内容 | cmd: create collection<br>               | cmd=create collection&name=cs.cl; |
|          | name: 集合的全称（集合空间.集合）<br>    |                                   |
|          | options: 选项（可选参数，可不填）        |                                   |
+----------+------------------------------------------+-----------------------------------+
| 说明     |                                          |                                   |
+----------+------------------------------------------+-----------------------------------+
| 响应头   | 同通用响应头                             |                                   |
+----------+------------------------------------------+-----------------------------------+
| 响应内容 | {<br>                                    | { "errno": 0 }                    |
|          | errno: 返回值，0表示成功，其他为失败<br> |                                   |
|          | description: 失败时的错误描述<br>        |                                   |
|          | }                                        |                                   |
+----------+------------------------------------------+-----------------------------------+
| 说明     |                                          |                                   |
+----------+------------------------------------------+-----------------------------------+


##删除集合##

+----------+------------------------------------------+---------------------------------+
|          | 说明                                     | 例子                            |
+==========+==========================================+=================================+
| 请求头   | 同通用请求头                             |                                 |
+----------+------------------------------------------+---------------------------------+
| 请求内容 | cmd: drop collection<br>                 | cmd=drop collection&name=cs.cl; |
|          | name: 集合的全称（集合空间.集合）        |                                 |
+----------+------------------------------------------+---------------------------------+
| 说明     |                                          |                                 |
+----------+------------------------------------------+---------------------------------+
| 响应头   | 同通用响应头                             |                                 |
+----------+------------------------------------------+---------------------------------+
| 响应内容 | {<br>                                    | { "errno": 0 }                  |
|          | errno: 返回值，0表示成功，其他为失败<br> |                                 |
|          | description: 失败时的错误描述<br>        |                                 |
|          | }                                        |                                 |
+----------+------------------------------------------+---------------------------------+
| 说明     |                                          |                                 |
+----------+------------------------------------------+---------------------------------+


##插入数据##

+----------+------------------------------------------+-----------------------------------------------------------+
|          | 说明                                     | 例子                                                      |
+==========+==========================================+===========================================================+
| 请求头   | 同通用请求头                             |                                                           |
+----------+------------------------------------------+-----------------------------------------------------------+
| 请求内容 | cmd: insert<br>                          | cmd=insert&name=cs.cl&insertor={"age":12,"name":"hello"} |
|          | name: 集合的全称（集合空间.集合）<br>    |                                                           |
|          | insertor: 待插入数据                     |                                                           |
+----------+------------------------------------------+-----------------------------------------------------------+
| 说明     |                                          |                                                           |
+----------+------------------------------------------+-----------------------------------------------------------+
| 响应头   | 同通用响应头                             |                                                           |
+----------+------------------------------------------+-----------------------------------------------------------+
| 响应内容 | {<br>                                    | { "errno": 0 }                                            |
|          | errno: 返回值，0表示成功，其他为失败<br> |                                                           |
|          | description: 失败时的错误描述<br>        |                                                           |
|          | }                                        |                                                           |
+----------+------------------------------------------+-----------------------------------------------------------+
| 说明     |                                          |                                                           |
+----------+------------------------------------------+-----------------------------------------------------------+

##查询数据##

+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
|          | 说明                                            | 例子                                                                                  |
+==========+=================================================+=======================================================================================+
| 请求头   | 同通用请求头                                    |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
| 请求内容 | cmd: query<br>                                  | cmd=query&name=cs.cl&filter={"name":"hello"}                                          |
|          | name: 集合的全称（集合空间.集合）<br>           |                                                                                       |
|          | sort: 待排序字段名（可选参数，可不填）<br>      |                                                                                       |
|          | selector: 查询结果列（可选参数，可不填）<br>    |                                                                                       |
|          | skip: 跳过多少行（可选参数，可不填）<br>        |                                                                                       |
|          | returnnum: 最大返回条数（可选参数，可不填）<br> |                                                                                       |
|          | filter: 查询条件（可选参数，可不填）            |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
| 说明     |                                                 |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
| 响应头   | 同通用响应头                                    |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
| 响应内容 | {<br>                                           | { "errno": 0 }{ "_id":{ "$oid":"54def72f0d8737161d9d6934" },"age":12,"name":"hello" } |
|          | errno: 返回值，0表示成功，其他为失败<br>        |                                                                                       |
|          | description: 失败时的错误描述<br>               |                                                                                       |
|          | }<br>                                           |                                                                                       |
|          | {<br>                                           |                                                                                       |
|          | 返回表里的记录<br>                              |                                                                                       |
|          | }<br>                                           |                                                                                       |
|          | ...                                             |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
| 说明     |                                                 |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+


##查询更新数据##

+----------+--------------------------------------------------+-------------------------------------------------------------------------------------------------+
|          | 说明                                             | 例子                                                                                            |
+==========+==================================================+=================================================================================================+
| 请求头   | 同通用请求头                                     |                                                                                                 |
+----------+--------------------------------------------------+-------------------------------------------------------------------------------------------------+
| 请求内容 | cmd: queryandupdate<br>                          | cmd=queryandupdate&name=cs.cl&updator={$set:{"age":100}}&filter={"name":"hello"}&returnnew=true |
|          | name: 集合的全称（集合空间.集合）<br>            |                                                                                                 |
|          | updator: 更新操作<br>                            |                                                                                                 |
|          | sort: 待排序字段名（可选参数，可不填）<br>       |                                                                                                 |
|          | selector: 查询结果列（可选参数，可不填）<br>     |                                                                                                 |
|          | skip: 跳过多少行（可选参数，可不填）<br>         |                                                                                                 |
|          | returnnum: 最大返回条数（可选参数，可不填）<br>  |                                                                                                 |
|          | filter: 查询条件（可选参数，可不填）<br>         |                                                                                                 |
|          | returnnew: 是否返回更新后记录（可选参数，可不填）|                                                                                                 |
+----------+--------------------------------------------------+-------------------------------------------------------------------------------------------------+
| 说明     |                                                  |                                                                                                 |
+----------+--------------------------------------------------+-------------------------------------------------------------------------------------------------+
| 响应头   | 同通用响应头                                     |                                                                                                 |
+----------+--------------------------------------------------+-------------------------------------------------------------------------------------------------+
| 响应内容 | {<br>                                            | { "errno": 0 }{ "_id":{ "$oid":"54def72f0d8737161d9d6934" },"age":12,"name":"hello" }           |
|          | errno: 返回值，0表示成功，其他为失败<br>         |                                                                                                 |
|          | description: 失败时的错误描述<br>                |                                                                                                 |
|          | }<br>                                            |                                                                                                 |
|          | {<br>                                            |                                                                                                 |
|          | 返回表里的记录<br>                               |                                                                                                 |
|          | }<br>                                            |                                                                                                 |
|          | ...                                              |                                                                                                 |
+----------+--------------------------------------------------+-------------------------------------------------------------------------------------------------+
| 说明     |                                                  |                                                                                                 |
+----------+--------------------------------------------------+-------------------------------------------------------------------------------------------------+


##查询删除数据##

+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
|          | 说明                                            | 例子                                                                                  |
+==========+=================================================+=======================================================================================+
| 请求头   | 同通用请求头                                    |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
| 请求内容 | cmd: queryandremove<br>                         | cmd=queryandremove&name=cs.cl&filter={"name":"hello"}                                 |
|          | name: 集合的全称（集合空间.集合）<br>           |                                                                                       |
|          | sort: 待排序字段名（可选参数，可不填）<br>      |                                                                                       |
|          | selector: 查询结果列（可选参数，可不填）<br>    |                                                                                       |
|          | skip: 跳过多少行（可选参数，可不填）<br>        |                                                                                       |
|          | returnnum: 最大返回条数（可选参数，可不填）<br> |                                                                                       |
|          | filter: 查询条件（可选参数，可不填）            |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
| 说明     |                                                 |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
| 响应头   | 同通用响应头                                    |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
| 响应内容 | {<br>                                           | { "errno": 0 }{ "_id":{ "$oid":"54def72f0d8737161d9d6934" },"age":12,"name":"hello" } |
|          | errno: 返回值，0表示成功，其他为失败<br>        |                                                                                       |
|          | description: 失败时的错误描述<br>               |                                                                                       |
|          | }<br>                                           |                                                                                       |
|          | {<br>                                           |                                                                                       |
|          | 返回表里的记录<br>                              |                                                                                       |
|          | }<br>                                           |                                                                                       |
|          | ...                                             |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+
| 说明     |                                                 |                                                                                       |
+----------+-------------------------------------------------+---------------------------------------------------------------------------------------+


##删除记录##

+----------+------------------------------------------+------------------------------------------------+
|          | 说明                                     | 例子                                           |
+==========+==========================================+================================================+
| 请求头   | 同通用请求头                             |                                                |
+----------+------------------------------------------+------------------------------------------------+
| 请求内容 | cmd: delete<br>                          | cmd=delete&name=cs.cl&deletor={"name":"hello"} |
|          | name: 集合的全称（集合空间.集合）<br>    |                                                |
|          | deletor: 删除条件                        |                                                |
+----------+------------------------------------------+------------------------------------------------+
| 说明     |                                          |                                                |
+----------+------------------------------------------+------------------------------------------------+
| 响应头   | 同通用响应头                             |                                                |
+----------+------------------------------------------+------------------------------------------------+
| 响应内容 | {<br>                                    | { "errno": 0 }                                 |
|          | errno: 返回值，0表示成功，其他为失败<br> |                                                |
|          | description: 失败时的错误描述<br>        |                                                |
|          | }                                        |                                                |
+----------+------------------------------------------+------------------------------------------------+
| 说明     |                                          |                                                |
+----------+------------------------------------------+------------------------------------------------+



##更新记录##

+----------+------------------------------------------+--------------------------------------------------------------------------+
|          | 说明                                     | 例子                                                                     |
+==========+==========================================+==========================================================================+
| 请求头   | 同通用请求头                             |                                                                          |
+----------+------------------------------------------+--------------------------------------------------------------------------+
| 请求内容 | cmd: update<br>                          | cmd=update&name=cs.cl&updator={$set:{"age":100}}&filter={"name":"hello"} |
|          | name: 集合的全称（集合空间.集合）<br>    |                                                                          |
|          | updator: 更新操作<br>                    |                                                                          |
|          | filter: 更新条件                         |                                                                          |
+----------+------------------------------------------+--------------------------------------------------------------------------+
| 说明     |                                          |                                                                          |
+----------+------------------------------------------+--------------------------------------------------------------------------+
| 响应头   | 同通用响应头                             |                                                                          |
+----------+------------------------------------------+--------------------------------------------------------------------------+
| 响应内容 | {<br>                                    | { "errno": 0 }                                                           |
|          | errno: 返回值，0表示成功，其他为失败<br> |                                                                          |
|          | description: 失败时的错误描述<br>        |                                                                          |
|          | }                                        |                                                                          |
+----------+------------------------------------------+--------------------------------------------------------------------------+
| 说明     |                                          |                                                                          |
+----------+------------------------------------------+--------------------------------------------------------------------------+


##更新或插入记录##

+----------+------------------------------------------+-----------------------------------------------------------------------------------------------------+
|          | 说明                                     | 例子                                                                                                |
+==========+==========================================+=====================================================================================================+
| 请求头   | 同通用请求头                             |                                                                                                     |
+----------+------------------------------------------+-----------------------------------------------------------------------------------------------------+
| 请求内容 | cmd: upsert<br>                          | cmd=upsert&name=cs.cl&updator={$set:{"age":100}}&filter={"name":"hello"}&setoninsert={"sex":"male"} |
|          | name: 集合的全称（集合空间.集合）<br>    |                                                                                                     |
|          | updator: 更新操作<br>                    |                                                                                                     |
|          | filter: 更新条件（可选参数，可不填）<br> |                                                                                                     |
|          | setoninsert: 插入数据（可选参数，可不填）|                                                                                                     |
+----------+------------------------------------------+-----------------------------------------------------------------------------------------------------+
| 说明     |                                          |                                                                                                     |
+----------+------------------------------------------+-----------------------------------------------------------------------------------------------------+
| 响应头   | 同通用响应头                             |                                                                                                     |
+----------+------------------------------------------+-----------------------------------------------------------------------------------------------------+
| 响应内容 | {<br>                                    | { "errno": 0 }                                                                                      |
|          | errno: 返回值，0表示成功，其他为失败<br> |                                                                                                     |
|          | description: 失败时的错误描述<br>        |                                                                                                     |
|          | }                                        |                                                                                                     |
+----------+------------------------------------------+-----------------------------------------------------------------------------------------------------+
| 说明     |                                          |                                                                                                     |
+----------+------------------------------------------+-----------------------------------------------------------------------------------------------------+


##获取记录数##

+----------+------------------------------------------+-----------------------------+
|          | 说明                                     | 例子                        |
+==========+==========================================+=============================+
| 请求头   | 同通用请求头                             |                             |
+----------+------------------------------------------+-----------------------------+
| 请求内容 | cmd: get count<br>                       | cmd=get count&name=cs.cl    |
|          | name: 集合的全称（集合空间.集合）<br>    |                             |
|          | filter: 过滤条件（可选）                 |                             |
+----------+------------------------------------------+-----------------------------+
| 说明     |                                          |                             |
+----------+------------------------------------------+-----------------------------+
| 响应头   | 同通用响应头                             |                             |
+----------+------------------------------------------+-----------------------------+
| 响应内容 | {<br>                                    | { "errno": 0 }{ "Total":1 } |
|          | errno: 返回值，0表示成功，其他为失败<br> |                             |
|          | description: 失败时的错误描述<br>        |                             |
|          | }<br>                                    |                             |
|          | {<br>                                    |                             |
|          | Total: 总计数<br>                        |                             |
|          | }                                        |                             |
+----------+------------------------------------------+-----------------------------+
| 说明     |                                          |                             |
+----------+------------------------------------------+-----------------------------+


##修改表属性##

+----------+------------------------------------------+-----------------------------------------------------------------------------------+
|          | 说明                                     | 例子                                                                              |
+==========+==========================================+===================================================================================+
| 请求头   | 同通用请求头                             |                                                                                   |
+----------+------------------------------------------+-----------------------------------------------------------------------------------+
| 请求内容 | cmd: alter collection<br>                | cmd=alter collection&name=cs.cl&options={ShardingKey:{age:1},ShardingType:"hash"} |
|          | name: 集合的全称（集合空间.集合）<br>    |                                                                                   |
|          | options: 属性                            |                                                                                   |
+----------+------------------------------------------+-----------------------------------------------------------------------------------+
| 说明     |                                          |                                                                                   |
+----------+------------------------------------------+-----------------------------------------------------------------------------------+
| 响应头   | 同通用响应头                             |                                                                                   |
+----------+------------------------------------------+-----------------------------------------------------------------------------------+
| 响应内容 | {<br>                                    | { "errno": 0 }                                                                    |
|          | errno: 返回值，0表示成功，其他为失败<br> |                                                                                   |
|          | description: 失败时的错误描述<br>        |                                                                                   |
|          | }                                        |                                                                                   |
+----------+------------------------------------------+-----------------------------------------------------------------------------------+
| 说明     |                                          |                                                                                   |
+----------+------------------------------------------+-----------------------------------------------------------------------------------+


##表分区##

+----------+------------------------------------------+------------------------------------------------------------+
|          | 说明                                     | 例子                                                       |  
+==========+==========================================+============================================================+
| 请求头   | 同通用请求头                             |                                                            |
+----------+------------------------------------------+------------------------------------------------------------+
| 请求内容 | cmd: split<br>                           | cmd=split&name=cs.cl&source=db1&target=db2&splitpercent=50 |
|          | name: 集合的全称（集合空间.集合）<br>    |                                                            |
|          | source: 源数据组<br>                     |                                                            |
|          | target: 目标数据组<br>                   |                                                            |
|          | splitpercent: 百分比<br>                 |                                                            |
|          | splitquery: 开始条件<br>                 |                                                            |
|          | splitEndQuery: 结束条件                  |                                                            |
+----------+------------------------------------------+------------------------------------------------------------+
| 说明     |                                          |                                                            |
+----------+------------------------------------------+------------------------------------------------------------+
| 响应头   | 同通用响应头                             |                                                            |
+----------+------------------------------------------+------------------------------------------------------------+
| 响应内容 | {<br>                                    | { "errno": 0 }                                             |
|          | errno: 返回值，0表示成功，其他为失败<br> |                                                            |
|          | description: 失败时的错误描述<br>        |                                                            |
|          | }                                        |                                                            |
+----------+------------------------------------------+------------------------------------------------------------+
| 说明     |                                          |                                                            |
+----------+------------------------------------------+------------------------------------------------------------+


##列出数据组##

+----------+-------------------------------------------+-----------------+
|          | 说明                                      | 例子            |   
+==========+===========================================+=================+
| 请求头   | 同通用请求头                              |                 |
+----------+-------------------------------------------+-----------------+
| 请求内容 | cmd: list groups                          | cmd=list groups |
+----------+-------------------------------------------+-----------------+
| 说明     |                                           |                 |
+----------+-------------------------------------------+-----------------+
| 响应头   | 同通用响应头                              |                 |
+----------+-------------------------------------------+-----------------+
| 响应内容 | {<br>                                     | { "errno": 0 }  |
|          | errno: 返回值，0表示成功，其他为失败<br>  |                 |
|          | description: 失败时的错误描述<br>         |                 |
|          | }<br>                                     |                 |
|          | {<br>                                     |                 |
|          | 返回数据组的内容<br>                      |                 |
|          | }                                         |                 |
+----------+-------------------------------------------+-----------------+
| 说明     |                                           |                 |
+----------+-------------------------------------------+-----------------+

BSON 是 JSON 的二进制表现形式，通过记录每个对象，元素，以及嵌套元素和数组的类型以及长度，能够高速有效地进行某个元素的查找。因此，在 C 和 C++ 中使用 BSON 官方提供的 BSON 接口进行数据存储。更多参考 [C BSON API](api/bson/html/index.html)。

与普通的 JSON 不同，BSON 提供更多的数据类型，以满足 C/C++ 语言多种多样的需求。SequoiaDB 提供了包括8字节浮点数（DOUBLE），字符串，嵌套对象，嵌套数组，对象 ID（数据库中每个集合中每条记录都有一个唯一 ID），布尔值，日期，NULL，正则表达式，4字节整数（INT），时间戳，以及8字节整数等数据类型。这些类型的定义可以在 bson.h 中的 bson_type 找到。注意：使用 C BSON API 函数在建立 BSON 出错时，将返回错误码，应当适当检测函数返回值。详情请查看 C BSON API。

在用户程序使用 BSON 对象时，主要分为建立对象和读取对象两个操作。

## 建立对象##

总的来说，一个 BSON 对象的创建主要分为三大步操作：

1）创建对象（bson_create ; bson_init）

2）使用对象

3）清除对象（bson_dispose ; bson_destory）

* 创建一个简单的 BSON 对象{age:20}。

<pre class="prettyprint lang-javascript">
INT32 rc = SDB_OK;
bson obj;
bson_init(&obj);
bson_appent_int(obj,"age",20);
if ( bson_finish(obj) != SDB_OK )
printf("Error.") ;
bson_destory(obj);</pre>

* 创建一个复杂的 BSON 对象

<pre class="prettyprint lang-javascript">
/* 创建一个包含{name:"tom",colors:["red","blue","green"], address: {city:"Toronto, province: "Ontario"}}的对象 */
bson_iterator bi ;
bson *newobj = bson_create () ;
bson_append_string ( newobj, "name", "tom" ) ;
bson_append_start_object ( newobj, "address" ) ;
bson_append_string ( newobj, "city", "Toronto" ) ;
bson_append_string ( newobj, "provice", "Ontario" ) ;
bson_append_start_array(newobj,"colors");
bson_appent_string(newobj,"0","red");
bson_appent_string(newobj,"1","blue");
bson_appent_string(newobj,"2","green");
bson_append_finish_object ( newobj ) ;
if( bson_finish ( newobj ) != BSON_OK )
   printf("Error.") ;</pre>

## 读取对象##

读取 BSON 对象使用一个 bson_iterator，对一个完整的例子，可以使用 bson_print_raw 方法来读取。但是首先得初始化 bson_iterator 对象，然后使用 bson_iterator_next 遍历每一个元素。

例如：

<pre class="prettyprint lang-javascript">
bson_iterator i[1] ;
bson_type type ;
const char * key;

bson_iterator_init(i, newobj) ;

type = bson_iterator_next (i);
key = bson_iterator_key (i);

printf( "Type: %d, Key: %s\n", type, key) ;</pre>

对于每个 bson_iterator，使用 bson_iterator_type 函数可以得到其类型，使用 bson_iterator_string 等函数可以得到其相对应类型的数值。

<pre class="prettyprint lang-javascript">
printf( "Value: %s, bson_iterator_string(i)) ;</pre>


* 遍历每个连续的 BSON 对象元素，可以使用 bson_find 函数直接跳转得到元素的名称。如果该元素不存在于 bson 之内，则 bson_find 函数返回 BSON_EOO。

例如想得到 name 元素名可以这样使用：

<pre class="prettyprint lang-javascript">
bson_iterator i[1] ,sub[i] ;
bson_type type ;

bson_find ( i, newobj, "name" )</pre>

* 读取数组元素或嵌套对象，因为“address”是一个嵌套对象，需要特殊遍历。首先得到 address 值，再初始化一个新的 BSON 迭代器：

<pre class="prettyprint lang-javascript">
type = bson_find(i,newobj,"address");
bson_iterator_subiterator(i,sub);</pre>

方法 bson_iterator_subiterator 初始化迭代器 sub，并且指向子对象的开始位置，从这里开始可以遍历 sub 中的所有元素，直到子对象的结束位置。
## C++ BSON 主要类##

C++ BSON 用到4个类：

-   bson::BSONObj：创建 BSONObj 对象。

-   bson::BSONElement：BSONObj对象由 BSONElement 对象组成，即 BSONElement 对象为 BSONObj 对象的字段或者元素，它是键值对。

-   bson::BSONObjBuilder：BSONObjBuilder 用来实例化 BSONObj 对象。

-   bson::BSONObjlterator：BSONObjlterator 用来遍历 BSONObj 对象中的元素。命名空间 bson 中定义了这些类的类型为：

	-   typedef bson::BSONElement be;

	-   typedef bson::BSONObj bo;

	-   typedef bson::BSONObjBuilder bob;

另外，可以使用 bo::iterator 代替 BSONObjlterator。

## 建立对象##

以下简单介绍如何创建用 CPP BSON 实例。详细内容请查阅 [C++ BSON API](api/bsoncpp/html/index.html)

* 使用 BSONObject，BSONObjBuilder 建立对象

<pre class="prettyprint lang-javascript">
#include "client.hpp"
...
using namespace bson ;
BSONObj obj ;
BSONObjBuilder b ;

b.append("name","sam") ;
b.append("age","24") ;
obj = b.obj() ;
或者
obj = BSONObjBuilder().genOID().append("name","sam").append("age",24).obj() ;</pre>

另外，可以使用数据流的方法建立 BSONObj 对象。

<pre class="prettyprint lang-javascript">
BSONObj obj ;
BSONObjBuilder b ;
b << "name" << "sam" << "age" << "24" ;
obj = b.obj() ;</pre>

* 使用宏 BSON 建立对象

C++ BSON 中定义还定义了一个 BSON 的宏，可以用它来快速地建立 BSONObj 对象。

<pre class="prettyprint lang-javascript">
BSONObj obj ;
// int
obj = BSON( "a" << 1 ) ;
// float
obj = BSON( "b" << 3.14159265359 ) ;
// string
obj = BSON( "foo" << "bar" ) ;
// OID
obj = BSON( GENOID ) ;
// bool
obj = BSON( "flag" << true"ret" << false ) ;
// object
obj = BSON( "d" << BSON("e" << "hi!") ) ;
// array
obj = BSON( "phone" << BSON_ARRAY( "13800138123" << "13800138124" ) ) ;
// others, less then, greater then, etc
obj = BSON( "g" << LT << 99 ) ;</pre>


* 使用 fromjson 接口建立对象

此外，可以使用 fromjson.hpp 中的 fromjson() 将 json 字符串转换成 BSONObj 对象。

<pre class="prettyprint lang-javascript">
string s("{name:"sam"}") ;
fromjson ( s, obj ) ;
或者
const char *r ="{
                   firstName:"Sam",
                   lastName:"Smith",age:25,id:"count",
                   address:{streetAddress: "25 3ndStreet",
                   city:"NewYork",state:"NY",postalCode:"10021"},
                   phoneNumber:[{type: "home",number:"212555-1234"}]
               	}" ;
fromjson ( r, obj ) ;</pre>
##说明##

SecureSdb 是 Sdb 的子类，SecureSdb 的对象使用 SSL 连接。##语法##
***traceFmt(&lt;formatType&gt;,&lt;input&gt;,&lt;output&gt;)***

将 db.traceOff() 导出来的二进制文件格式化为另外文件输出。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| formatType | int | 格式类型 | 是 |
| input | string | 输入文件 | 是 |
| output | string | 输出文件 | 是 |

##示例##

**格式化输出文件**：

<pre class="prettyprint lang-javascript">
> traceFmt(0, "/opt/sequoiadb/trace.dump", "/opt/sequoiadb/trace.flw")</pre>
##语法##
***backupOffline([options])***

备份数据库。

##参数描述##

| 参数名 |参数类型| 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options |Json 对象| 设定备份名，指定复制组，备份方式等参数 | 否 |

###Options 格式##

| 属性名 | 描述 | 格式 |
| ------ | ------ | ------ |
| GroupID | 指定备份的复制组 ID，缺省为所有复制组 | GroupID:1000 或 GroupID:[1000, 1001] |
| GroupName | 指定备份的复制组名，缺省为所有复制组 | GroupName:"data1" 或 GroupName:["data1", "data2"] |
| Name | 备份名称，缺省为“YYYY-MM-DD-HH:mm:SS”时间格式的备份名 |  Name:"backup-2014-1-1" |
| Path | 备份路径，缺省为配置参数指定的备份路径。该路径支持通配符（%g/%G: group name, %h/%H: host name, %s/%S:service name）| Path:"/opt/sequoiadb/backup/%g" |
| IsSubDir | 上述 Path 参数所配置的路径是否为配置参数指定的备份路径的子目录，缺省为 false | IsSubDir:false |
| Prefix | 备份前缀名，支持通配符（%g,%G,%h,%H,%s,%S），缺省为空 | Prefix:"%g_bk_" |
| EnableDataDir | 是否开启日期子目录功能，如果开启则会自动根据当前日期创建“YYYY-MM-DD”的子目录，缺省为 false | EnableDataDir:false |
| Description | 备份描述 | Description:"First backup" |
| EnsureInc | 是否开启增量备份，缺省为 false | EnsureInc:false |
| OverWrite | 存在同名备份是否覆盖，缺省为 false | OverWrite:false |

##示例##

**对整个数据库进行全量备份**
<pre class="prettyprint lang-javascript">
> db.backupOffline({Name:"FullBackup1"})
> db.listBackup()
{
  "Name": "FullBackup1",
  "NodeName": "susetzb:30000",
  "GroupName": "SYSCatalogGroup",
  "EnsureInc": false,
  "BeginLSNOffset": 0,
  "EndLSNOffset": 5299104,
  "StartTime": "2015-10-20-16:52:42",
  "HasError": false
}
{
  "Name": "FullBackup1",
  "NodeName": "susetzb:40000",
  "GroupName": "db2",
  "EnsureInc": false,
  "BeginLSNOffset": 0,
  "EndLSNOffset": 230209508,
  "StartTime": "2015-10-20-16:52:42",
  "HasError": false
}
{
  "Name": "FullBackup1",
  "NodeName": "susetzb:20000",
  "GroupName": "db1",
  "EnsureInc": false,
  "BeginLSNOffset": 0,
  "EndLSNOffset": 272453160,
  "StartTime": "2015-10-20-16:52:42",
  "HasError": false
}
Return 3 row(s).
Takes 0.34440s.
</pre>
##语法##
***db.cancelTask(&lt;id&gt;,[ isAsync ])***

取消任务

##参数描述##

| 参数名 | 参数类型 | 描述   | 是否必填 |
|--------|----------|--------|----------|
| id     | 整数     |任务ID  | 是       |
| isAsync| 布尔     |是否异步| 否       |

##示例##

**停止切分任务**
<pre class="prettyprint lang-javascript">
> var taskid1 = db.test.test.splitAsync("db1", "db2", 50);
> db.cancelTask( taskid1, true )</pre>
##语法##
***db.close()***

关闭数据库连接。

##示例##

-   关闭数据库连接

<pre class="prettyprint lang-javascript">
> var db = new Sdb("192.168.20.178", 11810)
Takes 0.11733s.
> db.close()
Takes 0.569s.</pre>
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
##语法##
***db.createCoordRG()***

创建唯一的协调分区组。

##示例##

-   创建一个协调分区组

<pre class="prettyprint lang-javascript">
> db.createCoordRG()</pre>
##语法##
***db.createCS(&lt;name&gt;,[options])***

在数据库对象中创建集合空间。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 集合空间名。同一个数据库对象中，集合空间名必须唯一。 | 是 |
| options | Json | 对象 | 集合空间可选属性。 | 否 |

###options 格式##

| 属性名 | 描述 | 格式 |
| ------ | ------ | ------ |
| PageSize | 数据页大小。默认为65536B。 | PageSize:&lt;int32&gt; |
| Domain | 所属域 | Domain:&lt;string&gt; |
| LobPageSize | Lob数据页大小。默认262144B | LobPageSize:&lt;int32&gt; |
| IndexEngineType（社区版） | 索引存储引擎类型。默认mmap | IndexEngineType:&lt;string&gt; |

**Note:**

* name 字段的值不能是空串，含点（.）或者美元符号（$）。且长度不超过127B。
* 同一个数据库对象集合空间名必须唯一。
* 在创建集合空间时用户可以指定数据页大小，指定后不可更改。如果不指定默认为65536B。
* PageSize 只能选填0，4096，8192，16384，32768，65536之一，0即为默认值65536。
* 所属域必须已经存在，且不能为 SYSDOMAIN。
* 为兼容较早版本接口，db.createCS(&lt;name&gt;,[ PageSize ]) 同样可以工作。
* LobPageSize只能选填0，4096，8192，16384，32768，65536，131072，262144，524288之一，0即为默认值262144。
* IndexEngineType可选填mmap，rocksdb。

##示例##

* 创建名为 foo 的集合空间，不指定数据页大小，即数据页大小为默认值65536B

<pre class="prettyprint lang-javascript">
> db.createCS("foo")</pre>

* 创建名为 foo 的集合空间，指定数据页大小为4096B，所属域为“mydomain”
<pre class="prettyprint lang-javascript">
> db.createCS("foo",{PageSize:4096,Domain:"mydomain"})</pre>

* 创建名为 foo 的集合空间，指定其索引存储引擎使用RocksDB
<pre class="prettyprint lang-javascript">
> db.createCS("foo",{IndexEngineType:"rocksdb"})</pre>

##语法##
***db.createDomain(&lt;name&gt;,&lt;groups&gt;,[options])***

创建一个域。域中可以包含若干个复制组（Replica Group）。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 域名，全局唯一。 | 是 |
| groups | Json 数组 | 域包含的复制组。 | 是 |
| options | Json 对象 | 在创建域时可以通过 options 设置其他属性。 | 否 |

##格式##

目前通过 options 可设置域的属性有：

| 属性名 | 描述 | 格式 |
| ------ | ------ | ------ |
| AutoSplit | 自动切分散列分区集合。 | AutoSplit:true\|false |

**Note:**

* AutoSplit 只作用于散列分区集合。
* 不能在空域（不包含复制组）创建集合空间。

##示例##

* 创建一个域，包含两个复制组。

<pre class="prettyprint lang-javascript">
> db.createDomain('mydomain',['datagroup1','datagroup2'])</pre>

* 创建一个域，包含两个复制组，并且指定自动切分。

<pre class="prettyprint lang-javascript">
> db.createDomain('mydomain',['datagroup1','datagroup2'],{AutoSplit:true})</pre>
##语法##
***db.createProcedure(&lt;code&gt;)***

在数据库对象中创建存储过程。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| code | 自定义函数 | 标准函数定义，不是字符串类型，在输入参数时不能使用引号。 | 是 |

##格式##

createProcedure() 方法的定义格式包含 code 参数，参数值为标准函数定义。

<pre class="prettyprint lang-javascript">
> createProcedure(&lt;code&gt;)</pre>

**说明：**

* 推荐直接使用存储过程中已初始化全局的 db，且全局 db 采用当前执行该存储过程的会话的鉴权信息，如：`db.createProcedure( function getAll(){return db.foo.bar.find();} )` 。
* 自己初始化 db 的形式为 `var db = new Sdb()`，db 采用当前执行该存储过程的会话的鉴权信息。如果需要加入其它用户名和密码，为 `var db = new Sdb('usrname','passwd')` 。这里需要注意的时，存储过程只能运行在已连接上的 db，不提供远程连接其他 db 的方法。在不需要鉴权的情况下，即使如 `var db = new Sdb('hostname', 'servicename')` 语句正常执行。得到的 db 仍然是本地 db。
* db 角色必须为协调节点。standalone 模式不提供存储过程功能。

##函数定义##

* 函数定义

（1） 函数必须包含函数名。不能使用如：function(x,y){return x+y;}

（2） 在函数定义时可以调用其他函数甚至是不存在的函数。但需要保证运行时所有函数已存在。

（3） 函数名全局唯一。不提供重载。

（4） 每个函数均在全系统可用。随意删除一个存储过程可能导致他人运行失败。

* 函数参数

<pre class="prettyprint lang-javascript">
> native type of JS</pre>

* 函数输出

函数中所有标准输出，标准错误会被屏蔽。同时不建议在函数定义或执行时加入输出语句。大量的输出可能会导致存储过程运行失败

* 函数返回值

函数返回值可以是除 db 以外任意类型数据。如：function getCL(){return db.foo.bar;}

##示例##

* 创建 sum 函数
<pre class="prettyprint lang-javascript">
> db.createProcedure(function sum(x,y){return x+y;})</pre>

创建之后可以使用 db.listProcedures() 查看函数信息。
##语法##
***db.createRG(&lt;name&gt;)***

新建一个分区组。创建后系统自动为分区组分配一个 GroupId。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 分区组名，同一个数据库对象中，分区组名唯一。 | 是 |

##格式##

createRG() 方法的定义格式只有 name 字段，name 的值是字符串型，必填。
<pre class="prettyprint diy">
(<"分区组名">)</pre>

**Note:**

* 分区组名不能是空串，含点（.）或者美元符号（$），并且长度不能超过127B。

## 示例

* 新建名为“group”的分区组

<pre class="prettyprint lang-javascript">
> db.createRG("group")</pre>
##语法##
***db.createUsr(&lt;name&gt;,&lt;password&gt;)***

为数据库创建用户名和密码。防止非法用户对数据库进行非法操作。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 用户名 | 是 |
| password | string | 密码 | 是 |

## 格式##

createUsr() 方法的定义格式 name 和 password 两个参数，两个参数都是字符串类型，且可以为空串。

<pre class="prettyprint lang-diy">
("<用户名>","<密码>")</pre>

**Note:**

* 当为数据库设置了用户名和密码时，那么只能使用正确的用户名和密码才能登录数据库进行相关操作。此时登录数据库的命令如下格式：


## 示例##

* 为数据库创建用户名为 root，密码为 admin 的命令如下：

<pre class="prettyprint lang-javascript">
> db = new Sdb("&lt;hostname&gt;","&lt;port&gt;","&lt;name&gt;","&lt;password&gt;")
> db.createUsr("root","admin")</pre>
## 语法##
***db.dropCS(&lt;name&gt;)***

在数据库对象中删除指定的集合空间。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 集合空间名。同一个数据库对象中集合空间名唯一。 | 是 |

## 格式##

删除集合空间的定义格式只有 name 字段，name 的值为 string 类型，指定的集合空间名必须要在数据库对象中存在，否则操作异常。

<pre class="prettyprint lang-diy">
("<集合空间名>")</pre>

**Note:**

* name字段的值不能使空串，含点（.），或者美元符号（$）。且长度不超过127B。
* 集合空间在数据库对象中存在。

## 示例##

* 删除名为 foo 的集合空间，假定 foo 已存在

<pre class="prettyprint lang-javascript">
> db.dropCS("foo")</pre>
## 语法##
***db.dropDomain(&lt;name&gt;)***

删除指定域。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 域名。 | 是 |

**Note:**

* dropDomain() 方法的定义格式必须指定 name 参数，并且 name 的值在系统中存在，否则操作异常。
* 删除域前必须保证域中不存在任何数据。
* 不能删除系统域。

## 示例##

* 删除一个之前创建的域。

<pre class="prettyprint lang-javascript">
> db.dropDomain('mydomain')</pre>

* 删除一个包含集合空间的域，返回错误：

<pre class="prettyprint lang-javascript">
> db.dropDomain('hello')
(nofile):0 uncaught exception: -256
Takes 0.1865s.
> getErr(-256)
Domain is not empty</pre>
## 语法##
***db.dropUsr(&lt;name&gt;,&lt;password&gt;)***

删除数据库已有的用户名和密码。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 用户名。 | 是 |
| password | string | 密码 | 是 |

## 格式##

dropUsr() 方法的定义格式 name 和 password 两个参数，两个参数都是字符串类型。

<pre class="prettyprint lang-diy">
("<用户名>","<密码>")</pre>

## 示例##

* 删除用户名为 root，密码为 admin 的数据库权限。

<pre class="prettyprint lang-javascript">
> db.dropUsr("root","admin")</pre>
## 语法##
***db.eval(&lt;code&gt;)***

根据需要填入 JavaScript 语句。同时可以在语句中调用已经创建好的存储过程。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| code | 字符串 | JavaScript 语句或者创建好的存储过程函数。 | 是 |

**说明：**

（1） 执行成功则按照语句返回结果。可以将返回值直接赋值给另一个变量。如：`var a = db.eval(' db.foo.bar'); a.find(); `

（2） 执行失败会返回错误码及错误信息。如：`{ "errmsg": "(nofile):1 ReferenceError: sum is not defined","retCode": -152 } `

（3） 在函数执行结束前操作不会返回。中途退出则终止整个执行，但已经执行的代码不会被回滚。

（4） 自定义返回值的长度有一定限制，参考 SequoiaDB 插入记录的最大长度。

（5） 支持定义临时函数，如：`db.eval('function sum(x,y){return x+y;} sum(1,2)')`

（6） 全局 db 使用方式与 createProcedure 相同。

**Note：**

虽然语句中的所有输出都会被屏蔽，但还是建议不要加入任何打印语句。

## 示例##

* 在eval() 方法中调用存储过程函数 sum

<pre class="prettyprint lang-javascript">
//初始时 sum() 方法不存在，返回异常信息
> var a = db.eval('sum(1,2)');
{ "errmsg": "(nofile):1 ReferenceError: getCL is not defined", "retCode": -152 }
(nofile):0 uncaught exception: -152
//初始化 sum()
> db.createProcedure(function sum(x,y){return x+y;})
//调用 sum()
> db.eval('sum(1,2)')
3</pre>

* 在 eval() 方法中填写 JavaScript 语句

<pre class="prettyprint lang-javascript">
> var rc = db.eval("db.foo.bar")
> rc.find()
{
  "_id": {
    "$oid": "5248d3867159ae144a000000"
  },
  "a": 1
}
{
  "_id": {
    "$oid": "5248d3897159ae144a000001"
  },
  "a": 2
}...</pre>
##语法##
***db.exec(&lt;select sql&gt;)***

执行 SQL 的 select 语句

##示例##

* 从集合 my.my 中查找所有 age = 20 的记录

<pre class="prettyprint lang-javascript">
> db.exec("select * from my.my where age = 20")</pre>##语法##
***db.execUpdate()***

###db.execUpdate(&lt;other sql&gt;)###

执行 SQL 的其它语句

##示例##

* 向集合 my.my 中插入新的记录

<pre class="prettyprint lang-javascript">
> db.execUpdate("insert into my.my(name,age) values('zhangshang', 30) ")</pre>##语法##
***db.flushConfigure(&lt;rule&gt;)***

刷盘配置至配置文件

##参数描述##

参数名    参数类型    描述         是否必填
--------- ----------- ------------ ----------
rule      Json 对象   刷盘规则     是

##rule 格式##

属性名  |  描述                       |                             格式
---------| ----------------------------------------------------- |  --------
Global   | true 表示刷盘全系统配置，false 表示只刷盘本节点配置  | Global:true

##示例##

-   刷盘数据库配置

<pre class="prettyprint lang-javascript">
> db.flushConfigure({Global:true});</pre>
## 语法##
***db.forceSession(&lt;sessionID&gt;)***

终止指定会话的当前操作。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| sessionID | int | 会话编号。 | 是 |

**Note:**

只有用户会话可以被终止。

## 示例##

* 终止编号为100的会话。

<pre class="prettyprint lang-javascript">
> db.forceSession(100)</pre>
##语法##
***db.forceStepUp([options])***

在一个不具备选举条件的复制组中，强制将一个备节点升级为主节点。**请谨慎使用该命令！**

##参数描述##

参数名    参数类型    描述         是否必填
--------- ----------- ------------ ----------
options   Json 对象   参数集合。   否

##options 选项##

参数名    参数类型   描述                           默认值
--------- ---------- ------------------------------ --------
Seconds   int        强制升级为主节点的持续时间。   120

**Note:**

-   目前仅在 catalog 组中开放此功能。
-   目标复制组中不能存在主节点，且其他节点的 LSN 不能比目标节点的 LSN 大。
-   当持续时间到期，所有节点会重新按照选举规则进行选举。
-   如果创建了用户，在主 catalog 节点的情况下无法直接连接。可以先关闭 catalog 的鉴权功能（auth 参数）再进行操作。

##示例##

-   连接 catalog 节点（host1:30000），并使其强制升主， 持续300s。

<pre class="prettyprint lang-javascript">
> var db = new Sdb("host1", 30000) ;
> db.forceStepUp({Seconds:300});</pre>
##语法##
***db.getCatalogRG()***

获取编目分区组的引用。

##示例##

-   获取编目分区组引用

<pre class="prettyprint lang-javascript">
> var rg = db.getCatalogRG()</pre>
##语法##
***db.getCoordRG()***

获取协调分区组的引用。

##示例##

-   获取协调分区组的引用

<pre class="prettyprint lang-javascript">
> var rg = db.getCoordRG()</pre>
## 语法##
***db.getCS(&lt;name&gt;)***

返回指定集合空间对象的引用。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 集合空间名。同一个数据库对象中集合空间名唯一。 | 是 |

## 格式##

getCS() 方法的定义格式只有 name 字段，name 的值是字符串型 。

<pre class="prettyprint diy">
("<集合空间名>")</pre>

**Note:**

* name 字段的值不能使空串，含点（.），或者美元符号（$）。且长度不超过127B。
* 集合空间在数据库对象中存在

## 示例##

* 返回集合空间 foo 的引用，假定 foo 已存在。

<pre class="prettyprint lang-javascript">
> db.getCS("foo")</pre>
## 语法##
***db.getDomain(&lt;name&gt;)***

获取指定域。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 域名 | 是 |

## 格式

getDomain() 方法的定义格式必须指定 name 参数，并且 name 的值在系统中存在，否则操作异常。

<pre class="prettyprint diy">
{"name":"<域名>"}</pre>

**Note:**

不能获取系统域。

## 示例##

* 获取一个之前创建的域。

<pre class="prettyprint lang-javascript">
> var domain = db.getDomain('mydomain')</pre>
## 语法##
***db.getRG(&lt;name&gt;|&lt;id&gt; )***

返回分区组的引用。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 分区组名。同一个数据库对象中，分区组名唯一| name 和 id 任选 |
| id | int | 分区组 id，创建分区组时系统自动分配 | id 和 name 任选 |

## 格式##

getRG() 方法定于格式包含 name 或 id 字段，name 为字符串型，id 为 int 型。指定的分区组名或 id 值要在数据库对象中存在，否则出现操作异常。

<pre class="prettyprint lang-diy">
("&lt;分区组名&gt;"|&lt;id&gt;)</pre>

**Note:**

* name 字段的值不能使空串，含点（.），或者美元符号（$）。且长度不超过127B。

##示例##

* 指定 name 值，返回分区组 rg1 的引用

<pre class="prettyprint lang-javascript">
> db.getRG("rg1")</pre>

* 指定 id 值，返回分区组 rg1 的引用

<pre class="prettyprint lang-javascript">
> db.getRG(1000)</pre>
## 语法##
***db.invalidateCache([options])***

清除节点（数据节点/协调节点）的缓存。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 清除缓存的选项 | 否 |

### options 格式###

目前通过 options 可设置域的属性有：

<table>
    <tr>
        <th>属性名</th>
        <th>描述</th>
        <th>格式</th>
    </tr>
    <tr>
        <td>Groups</td>
        <td>需要清除缓存的目标。</td>
        <td><p>Groups:null -- 当前协调节点；
        <p>Groups:['data1','data2'] -- 当前协调节点和指定的两个数据组；
        <p>Groups:'data1' -- 当前协调节点和指定的一个数据组。 </td>
    </tr>
</table>

**Note:**

当不指定 Groups 时，作用域为当前协调节点和所有数据节点。

## 示例##

* 清除当前协调节点和数据组‘data1’的缓存信息。

<pre class="prettyprint lang-javascript">
> db.invalidateCache({Groups:'data1'})</pre>
## 语法##
***db.list(&lt;listType&gt;,[con],[sel],[sort])***

枚举列表。列表是一种轻量级得到当前系统状态的命令。查看更多有关列表信息

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| listType | 枚举 | [列表类型](SdbDoc_Cn/database_management/monitoring/overview.html)。 | 是 |
| con | Json 对象 | 选择条件，只返回符合 con 字段值的记录，为 null 时，返回所有。 | 否 |
| sel | Json 对象 | 选择返回字段名。为 null 时，返回所有的字段名。 | 否 |
| sort | Json 对象 | 对返回的记录按选定的字段排序。1为升序；-1为降序。 | 否 |

## 格式##

list() 方法定义格式有 listType，con，sel，sort四个参数，listType 为枚举类型，其他全部为 Json 对象，格式如下：

<pre class="prettyprint lang-diy">
{"listType":"<列表类型>",["con":"{"字段名1":{"操作符1":"值1"},"字段名2":{"操作符2":"值2"}...}"],
["sel":"{"字段名1":"","字段名2":"",...}"],["sort":"{"字段名1":-1|1,"字段名2":1|-1,...}"]}</pre>

**Note:**

*  listType 字段的值请参考 [列表类型](SdbDoc_Cn/database_management/monitoring/overview.html)
*  sel 参数是一个 json 结构，字段名的值一般指定为空串。如果指定为如下结构：{"字段名1":"值1","字段名2":"值2",...}，对于记录中存在所选字段名的话，设定的值其实无效；对于记录中不存在所选字段名的话，返回{"字段名1":"值1","字段名2":"值2",...}
*  字段的值是数组的话，我们用"."操作符引用数组内的元素。并加上双引号("")

## 示例##

* 指定 listType 的值为 SDB_LIST_CONTEXTS：

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_CONTEXTS)</pre>

返回：

<pre class="prettyprint lang-diy">
{ 
  "SessionID": 4, 
  "Contexts": [ 0 ] 
}  ...</pre>

* 指定 listType 的值为 SDB_LIST_STORAGEUNITS：

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_STORAGEUNITS)</pre>

返回：

<pre class="prettyprint lang-javascript">
{ 
  "Name": "foo", 
  "ID": 4094, 
  "Logical ID": 1, 
  "PageSize": 4096, 
  "Sequence": 1, 
  "NumCollections": 1, 
  "CollectionHWM": 3, 
  "Size": 172032000 
}</pre>

* 返回符合条件 Logical ID 大于1的记录，并且每条记录只返回 Name 和 ID 这两个字段，记录按 Name 字段的值升序排序

<pre class="prettyprint lang-javascript">
> db.list(SDB_LIST_STORAGEUNITS,{"Logical ID":{$gt:1}},{Name:"space",ID:2},{Name：1})</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "ID": 4091,
  "Name": "foo"
}
{
  "ID": 4093,
  "Name": "name"
}...</pre>
## 语法##
***db.listBackup([options], [cond], [sel])***

查看数据库备份

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 设定备份名，指定复制组，路径等参数 | 否 |
| cond | Json 对象 | 备份过滤条件 | 否 |
| sel | Json 对象 | 查看备份选择输出的字段 | 否 |

### Options 格式###

| 参数名 | 描述 | 格式 |
| ------ | ------ | ------ |
| GroupID | 指定备份的复制组 ID，缺省为所有复制组 | GroupID:1000 或 GroupID:[1000, 1001] |
| GroupName | 指定备份的复制组名，缺省为所有复制组 | GroupName:"data1" 或 GroupName:["data1", "data2"] |
| Name | 备份名称，缺省查看所有备份 | Name:"backup-2014-1-1" |
| Path | 备份路径，缺省为配置参数指定的备份路径。该路径支持通配符（%g/%G: group name, %h/%H: host name, %s/%S:service name） | Path:"/opt/sequoiadb/backup/%g" |
| IsSubDir | 上述 Path 参数所配置的路径是否为配置参数指定的备份路径的子目录，缺省为 false IsSubDir:false | Prefix 备份前缀名，支持通配符（%g,%G,%h,%H,%s,%S），缺省为空 Prefix:"%g_bk_" |

## 示例##

* 查看数据库所有备份信息

<pre class="prettyprint lang-javascript">
> db.listBackup()</pre>
## 语法##
***db.listCollections()***

枚举集合，执行此方法会将指定集合空间下的集合信息显示出来。

## 示例##

<pre class="prettyprint lang-javascript">
> db.listCollections()</pre>

返回：
<pre class="prettyprint lang-diy">
{
  "Name": "foo.bar"
}</pre>
## 语法##
***db.listCollectionSpaces()***

枚举数据库中所有的集合空间。

## 示例##

<pre class="prettyprint lang-javascript">
> db.listCollectionSpaces()</pre>

返回：
<pre class="prettyprint lang-diy">
{
  "Name":"foo"
}</pre>
## 语法##
***db.listDomains()***

枚举域，执行此方法会显示系统中所有由用户创建的域。

## 示例##

<pre class="prettyprint lang-javascript">
> db.listDomains()</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "539ea19669d195f36432111a"
  },
  "Name": "hello",
  "Groups": 
  [
    {
      "GroupName": "data1",
      "GroupID": 1000
    },
    {
      "GroupName": "data2",
      "GroupID": 1001
    }
  ]
}</pre>
## 语法##
***db.listProcedures([cond])***

枚举所有的存储过程函数。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| cond | Json 对象 | 条件为空时，枚举所有的函数，不为空时，枚举符合条件函数。 | 是 |

listProcedures() 方法的定义，只有一个 Json 对象类型的参数名 cond，输入值时返回符合指定值的函数，否则的话返回所有的函数。

## 示例##

* 返回所有的函数信息

<pre class="prettyprint lang-javascript">
> db.listProcedures()

{ "_id" : { "$oid" : "52480389f5ce8d5817c4c353" }, 
  "name" : "sum", 
  "func" : "function sum(x, y) {return x + y;}", 
  "funcType" : 0 
}
{ "_id" : { "$oid" : "52480d3ef5ce8d5817c4c354" }, 
  "name" : "getAll", 
  "func" : "function getAll() {return db.foo.bar.find();}", 
  "funcType" : 0 
}
...</pre>

* 指定返回函数名为 sum 的记录

<pre class="prettyprint lang-javascript">
> db.listProcedures({name:"sum"})

{ "_id" : { "$oid" : "52480389f5ce8d5817c4c353" }, 
  "name" : "sum", 
  "func" : "function sum(x, y) {return x + y;}", 
  "funcType" : 0 
}</pre>
## 语法##
***db.listReplicaGroups()***

枚举分区组。

## 示例##

* 返回所有分区组信息，命令如下：

<pre class="prettyprint lang-javascript">
> db.listReplicaGroups()</pre>

返回：

<pre class="prettyprint lang-diy">
{
"Group": 
[
  {
    "dbpath": "/opt/sequoiadb/data/11800",
    "HostName": "vmsvr2-suse-x64",
    "Service": [
      {
        "Type": 0,
        "Name": "11800"
      },
      {
        "Type": 1,
        "Name": "11801"
      },
      {
        "Type": 2,
        "Name": "11802"
      },
      {
        "Type": 3,
        "Name": "11803"
      }
    ],
    "NodeID": 1000
  },
  {
    "dbpath": "/opt/sequoiadb/data/11850",
    "HostName": "vmsvr2-suse-x64",
    "Service": [
      {
        "Type": 0,
        "Name": "11850"
      },
      {
        "Type": 1,
        "Name": "11851"
      },
      {
        "Type": 2,
        "Name": "11852"
      },
      {
        "Type": 3,
        "Name": "11853"
      }
    ],
    "NodeID": 1001
  }
],
"GroupID": 1001,
"GroupName": "group",
"PrimaryNode": 1001,
"Role": 0,
"Status": 1,
"Version": 5,
"_id": {
  "$oid": "517b2fc33d7e6f820fc0eb57"
  }
}</pre>

这个分区组有有两个节点：11800和11850，其中11850为主节点。分区组详细信息请见分区组列表
##语法##
***db.listTasks([cond],[sel],[orderBy],[hint])***

查看数据库所有后台任务

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| cond | Json 对象 | 任务过滤条件 | 否 |
| sel | Json 对象 | 任务选择字段 | 否 |
| hint | Json 对象 | 保留项 | 否 |

## 示例##

* 列出系统所有后台任务

<pre class="prettyprint lang-javascript">
> db.listTasks()</pre>
##语法##
***db.removeBackup([options])***

删除数据库备份

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 设定备份名，指定复制组，备份路径等参数 | 否 |

### options 格式###

| 属性名 | 描述 | 格式 |
| ------ | ------ | ------ |
| GroupID | 指定备份的复制组 ID，缺省为所有复制组 | GroupID:1000 或 GroupID:[1000, 1001] |
| GroupName | 指定备份的复制组名，缺省为所有复制组 | GroupName:"data1" 或 GroupName:["data1", "data2"] |
| Name | 备份名称，缺省删除所有备份 | Name:"backup-2014-1-1" |
| Path | 备份路径，缺省为配置参数指定的备份路径。该路径支持通配符（%g/%G: group name, %h/%H: host name, %s/%S:service name） | Path:"/opt/sequoiadb/backup/%g" |
| IsSubDir | 上述 Path 参数所配置的路径是否为配置参数指定的备份路径的子目录，缺省为 false | IsSubDir:false |
| Prefix | 备份前缀名，支持通配符（%g,%G,%h,%H,%s,%S），缺省为空 | Prefix:"%g_bk_" |

## 示例##

* 删除数据库中备份名为“backup-2014-1-1”的备份信息

<pre class="prettyprint lang-javascript">
> db.removeBackup({Name:"backup-2014-1-1"})</pre>
##语法##
***db.removeCatalogRG()***

删除编目分区组。

##示例##

-   删除编目分区组

<pre class="prettyprint lang-javascript">
> db.removeCatalogRG()</pre>

**Note:**

删除编目分区组，要求编目分区组上已经没有数据节点及协调节点的信息。删除编目分区组将会把该组中所有的编目节点都删除。
##语法##
***db.removeCoordRG()***

删除数据库中协调分区组。

##示例##

-   删除协调分区组

<pre class="prettyprint lang-javascript">
> db.removeCoordRG()</pre>

**Note:**

删除协调分区组，原则上会把该分区组的所有协调节点都删除，但如果在删除这些节点过程中，先把db对象所连接上的协调节点删除，则有可能会遗留部分协调节点未删除，需要使用Oma类的removeCoord方法删除遗留的协调节点。
##语法##
***db.removeProcedure(&lt;function name&gt;)***

删除指定的函数名，函数名必须存在，否则出现异常信息。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| function name | 字符串 | 函数名 | 是 |

removeProcedure() 方法的定义，只有一个字符串类型的参数名 function name，它的值必须已存在，否则异常。

## 示例##

* 删除 sum 函数

<pre class="prettyprint lang-javascript">
> db.removeProcedure("sum")</pre>

必须保证待删除函数的函数名和定义时的一致。诸如 db.removeProcedure('sum') 的调用将返回失败。
## 语法##
***db.removeRG(&lt;name&gt;)***

删除数据库中指定的分区组。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 分区组名，同一个数据库对象中，分区组名唯一。 | 是 |

## 格式##

removeRG() 方法的定义格式只有 name 字段，name 的值是字符串型，必填。

<pre class="prettyprint lang-diy">
(<"分区组名">)</pre>

**Note:**

* 分区组名必须存在。

## 示例##

* 删除名为“group”的分区组

<pre class="prettyprint lang-javascript">
> db.removeRG("group")</pre>
## 语法##
***db.resetSnapshot([cond])***

重置快照。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| con | Json 对象 | 选择条件，只重置符合 cond 条件的快照记录，为 null 时，重置所有。 | 否 |

## 格式##

resetSnapshot() 方法定义格式有 cond 参数，它是一个 Json 对象。

<pre class="prettyprint lang-diy">
{["cond":"{"字段名1":{"匹配符1":"值1"},"字段名2":{"匹配符2":"值2"}...}"]}</pre>

## 示例##

* 重置 SessionID 大于1的快照。

<pre class="prettyprint lang-javascript">
> db.resetSnapshot({SessionID:{$gt:1}})</pre>
##语法##
***db.setSessionAttr (&lt;options&gt;)***

设置会话属性

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 会话属性选项 | 是 |

### options 格式###

| 属性名 | 描述 | 格式 |
| ------ | ------ | ------ |
| PreferedInstance | 会话读操作优先选取的数据库实例标识；取值"m"/"M"/"s"/"S"/"a"/"A"/1-7，分别表示 master/slave/anyone/node1-node7 | PreferedInstance:"M" |

## 示例##

* 设置会话优先从“主”数据库实例获取数据

<pre class="prettyprint lang-javascript">
> db.setSessionAttr({PreferedInstance:"M"})</pre>
## 语法##
***db.snapshot(&lt;snapType&gt;,[cond],[sel],[sort])***

枚举快照，快照是一种得到当前系统状态的命令。查看更多有关快照信息

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| snapType | 枚举 | 快照类型。 | 是 |
| cond | Json 对象 | 选择条件，只返回 cond 字段指定的节点或分区组的快照信息，为 null 时，返回整个集群的快照信息。 | 否 |
| sel | Json 对象 | 选择返回字段名。为 null 时，返回所有的字段名。 | 否 |
| sort | Json 对象 | 对返回的记录按选定的字段排序。1为升序；-1为降序。 | 否 |

## 格式##

snapshot() 方法定义格式有 snapType，cond两个字段，snapType 为枚举类型，cond 为 Json 对象，格式如下：

<pre class="prettyprint lang-diy">
{"snapType":"<快照类型>",["cond":"{"字段名1":{"操作符1":"值1"},"字段名2":{"操作符2":"值2"}...}"]}</pre>

**Note:**

* snapType 字段的值请参考快照类型
* sel 参数是一个 json 结构，字段名的值一般指定为空串。如果指定为如下结构：{"字段名1":"值1","字段名2":"值2",...}，对于记录中存在所选字段名的话，设定的值其实无效；对于记录中不存在所选字段名的话，返回{"字段名1":"值1","字段名2":"值2",...}
* 字段的值是数组的话，我们用“.”操作符引用数组内的元素。并加上双引号（""）

## 示例##

* 指定 snapType 的值为 SDB_SNAP_CONTEXTS：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS)</pre>

返回整个集群的上下文快照：

<pre class="prettyprint lang-diy">
{
  "SessionID": "vmsvr1-cent-x64-1:11820:22",
  "Contexts": [
    {
      "ContextID": 8,
      "Type": "DUMP",
      "Description": "BufferSize:0",
      "DataRead": 0,
      "IndexRead": 0,
      "QueryTimeSpent": 0,
      "StartTimestamp": "2013-12-28-16.07.59.146399"
    }
  ]
}
{
  "SessionID": "vmsvr1-cent-x64-1:11830:22",
  "Contexts": [
    {
      "ContextID": 6,
      "Type": "DUMP",
      "Description": "BufferSize:0",
      "DataRead": 0,
      "IndexRead": 0,
      "QueryTimeSpent": 0,
      "StartTimestamp": "2013-12-28-16.07.59.147576"
    }
  ]
}
{
  "SessionID": "vmsvr1-cent-x64-1:11840:23",
  "Contexts": [
    {
      "ContextID": 7,
      "Type": "DUMP",
      "Description": "BufferSize:0",
      "DataRead": 0,
      "IndexRead": 0,
      "QueryTimeSpent": 0,
      "StartTimestamp": "2013-12-28-16.07.59.148603"
    }
  ]
}</pre>

* 通过组名或组 ID 查询某个分区组的快照信息，如：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS,{GroupName:'data1'})</pre>

返回组名为“data1”的分区组快照信息

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS,{GroupID:1000})</pre>

返回组 ID 为“1000”的分区组快照信息
<pre class="prettyprint lang-diy">
{
  "SessionID": "vmsvr1-cent-x64-1:11820:22",
  "Contexts": [
    {
      "ContextID": 11,
      "Type": "DUMP",
      "Description": "BufferSize:0",
      "DataRead": 0,
      "IndexRead": 0,
      "QueryTimeSpent": 0,
      "StartTimestamp": "2013-12-28-16.13.57.864245"
    }
  ]
}
{
  "SessionID": "vmsvr1-cent-x64-1:11840:23",
  "Contexts": [
    {
      "ContextID": 10,
      "Type": "DUMP",
      "Description": "BufferSize:0",
      "DataRead": 0,
      "IndexRead": 0,
      "QueryTimeSpent": 0,
      "StartTimestamp": "2013-12-28-16.13.57.865103"
    }
  ]
}</pre>

* 通过“组名+主机名+服务名”或“组 ID+节点 ID”查询某个节点的快照信息，如：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS,{GroupName:'data1',HostName:"vmsvr1-cent-x64-1",svcname:"11820"});
> db.snapshot(SDB_SNAP_CONTEXTS,{GroupID:1000,NodeID:1001});</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "SessionID": "vmsvr1-cent-x64-1:11820:22",
  "Contexts": [
    {
      "ContextID": 11,
      "Type": "DUMP",
      "Description": "BufferSize:0",
      "DataRead": 0,
      "IndexRead": 0,
      "QueryTimeSpent": 0,
      "StartTimestamp": "2013-12-28-16.13.57.864245"
    }
  ]
}</pre>

* 通过“主机名+服务名”查询某个节点的快照信息，如：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_CONTEXTS,{HostName:"vmsvr1-cent-x64-1",svcname:"11820"})</pre>
## 语法##
***db.startRG(&lt;name&gt;)***

启动指定的分区组。分区组启动后才能在分区组上创建节点。这个方法等价于 rg.start()。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 分区组的名称 | 是 |

## 格式##

db.startRG() 的方法定义包含 name 一个参数，参数类型为字符串，为要启动的分区组名。

<pre class="prettyprint lang-diy">
("&lt;分区组名&gt;")</pre>

**Note:**

若指定的分区组不存在，将抛异常；若不指定任何分区组，该操作为空操作。

## 示例##

* 启动分区组的命令如下：

<pre class="prettyprint lang-javascript">
> db.startRG("group1")
> db.startRG("group2","group3","group4")</pre>
##语法##
***db.traceOff(&lt;dumpFile&gt;)***

关闭数据库引擎跟踪功能，并将跟踪情况导出二进制文件，如：/opt/sequoiadb/trace.dump

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| dump file | string | dump 的文件名称| 是 |

## 示例##

* 关闭数据库引擎跟踪 /opt/sequoiadb/trace.dump

<pre class="prettyprint lang-javascript">
> db.traceOff("/opt/sequoiadb/trace.dump");</pre>
##语法##
***db.traceOn(&lt;bufferSize&gt;,[strComp],[strBreakPoint])***

开启数据库引擎跟踪功能。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| bufferSize | int | 开启追踪的文件大小。 | 是 |
| strComp | string | 指定模块。 | 否 |
| strBreakPoint | string | 于函数处打断点进行跟踪。 | 否 |

## 示例##

* 开启数据库引擎程序跟踪的功能，默认为所有模块：

<pre class="prettyprint lang-javascript">
> db.traceOn(10000000);</pre>

* 开户数据库引擎程序跟踪功能，指定跟踪的模块名称和指定断点进行跟踪：

<pre class="prettyprint lang-javascript">
> db.traceOn(10000000, "cls, dms, mth", "_dmsTempCB::init") ;</pre>
##语法##
***db.traceResume()***

重新开启断点跟踪程序。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| - | - | - | - |

## 示例##

* 重新开启断点跟踪程序：

<pre class="prettyprint lang-javascript">
> db.traceResume()</pre>
##语法##
***db.traceStatus()***

查看当前程序跟踪的状态。

## 参数描述

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| - | - | - | - |

## 示例##

* 查看当前程序跟踪的状态：

<pre class="prettyprint lang-javascript">
> db.traceStatus()
{
  "TraceStarted": true,
  "Wrapped": false,
  "Size": 524288,
  "Mask": 
  [
    "auth",
    "bps",
    "cat",
    "cls",
    "dps",
    "mig",
    "msg",
    "net",
    "oss",
    "pd",
    "rtn",
    "sql",
    "tools",
    "bar",
    "client",
    "coord",
    "dms",
    "ixm",
    "mon",
    "mth",
    "opt",
    "pmd",
    "rest",
    "spt",
    "util",
    "aggr",
    "spd",
    "qgm"
  ],
  "BreakPoint": []
}</pre>
##语法##
***db.transBegin()***

开启[事务](SdbDoc_Cn/basic_operation/transaction.html) 。SequoiaDB 数据库事务是指作为单个逻辑工作单元执行的一系列操作。事务处理可以确保除非事务性单元内的所有操作都成功完成，否则不会永久更新面向数据的资源。

## 示例##

* 开启事务：

<pre class="prettyprint lang-javascript">
> db.transBegin()
Takes 0.4125s. </pre>

* 插入记录:

<pre class="prettyprint lang-javascript">
> cl.insert({date:99,id:8,a:0})
Takes 0.3434s. </pre>

* 回滚事务，插入的记录将被回滚，集合中无记录：

<pre class="prettyprint lang-javascript">
> db.transRollback()
Takes 0.6474s.
> cl.count()
Return 0 row(s). </pre>

* 开启事务：

<pre class="prettyprint lang-javascript">
> db.transBegin()
Takes 0.4084s. </pre>

* 插入记录:

<pre class="prettyprint lang-javascript">
> cl.insert({date:99,id:8,a:0})
Takes 0.2644s. </pre>

* 提交事务，插入的记录将被写入数据库：

<pre class="prettyprint lang-javascript">
> db.transCommit()
Takes 0.780s.
> cl.count()
Return 1 row(s). </pre>##语法##
***db.transCommit()***

事务提交。在开启事务之后，如果单个逻辑工作单元执行的操作无异常，执行事务提交命令，那么数据库的数据将被更新。

## 示例##

* 事务提交命令：

<pre class="prettyprint lang-javascript">
> db.transCommit() </pre>
##语法##
***db.transRollback()***

事务回滚。在开启事务之后，如果单个逻辑工作单元执行的操作出现异常，执行事务回滚命令，那么数据库回到原来状态。

## 示例##

* 事务回滚命令：

<pre class="prettyprint lang-javascript">
> db.transRollback() </pre>
##语法##
***db.waitTasks(&lt;id1&gt;,[id2],...)***

同步等待指定任务结束或取消

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| id1, id2… | 整数 | 任务 ID | 是 |

## 示例##

* 同步等待数据切分任务完成

<pre class="prettyprint lang-javascript">
> var taskid1 = db.test.test.splitAsync("db1", "db2", 50);
> var taskid2 = db.my.my.splitAsync("db3", "db4", 50) ;
> db.waitTasks( taskid1, taskid2 )</pre>
## 语法##
***db.collectionspace.createCL(&lt;name&gt;,[ options ])***

在指定集合空间下创建集合（Collection）。 集合是数据库中存放文档记录的逻辑对象。任何一条文档记录必须属于一个且仅一个集合。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ |------ |
| name | string | 集合名，在同一个集合空间中，集合名必须唯一。 | 是 |
| options | Json 对象 | 在创建集合时，可以通过 options 参数设置集合的其他属性，如指定集合的分区键，是否以压缩的形式插入数据等。 | 否 |

## 格式##

createCL() 方法的定义格式包含 name 和 options 两个参数。name 的值为字符串类型，必须有值。options 是设置集合其他属性参数，目前通过 options 可设置集合的属性有：

+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| 属性名              | 描述                                                                                              | 格式                                                                                   |
+=====================+===================================================================================================+========================================================================================+
| ShardingKey         | 分区键。                                                                                          | ShardingKey:{&lt;字段1&gt; : &lt;1&#124;-1&gt;,[&lt;字段2&gt;:&lt;1&#124;-1&gt;, ...]} |
+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| ShardingType        | 分区方式，默认为 hash  分区。                                                                     | ShardingType:"hash"&#124;"range"                                                       |
+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| Partition           | 分区数，hash 分区时填写，代表了 hash 分区的个数。其值必须是2的幂。范围在[2^3, 2^20]。默认为1024。 | Partition: &lt;分区数&gt;                                                              |
+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| ReplSize            | 副本数，默认情况下，副本写入个数为1。                                                             | ReplSize: &lt;int num&gt;                                                              |
+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| Compressed          | 是否数据压缩。默认为false。                                                                       | Compressed:true&#124;false                                                             |
+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| CompressionType     | 压缩算法类型。默认为 snappy 算法。                                                                | CompressionType:"snappy"&#124;"lzw"                                                    |
+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| IsMainCL            | 主分区集合。标识是否为主分区集合，默认为否。                                                      | IsMainCL:true&#124;false                                                               |
+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| AutoSplit           | 是否自动切分，默认为true。                                                                        | AutoSplit:true&#124;false                                                              |
+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| Group               | 指定创建在某个复制组。                                                                            | Group: &lt;group name&gt;                                                              |
+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| AutoIndexId         | 集合是否自动使用_id字段创建名字为"$id"的唯一索引，默认为true。                                    | AutoIndexId:true&#124;false                                                            |
+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| EnsureShardingIndex | 集合是否自动使用ShardingKey包含的字段创建名字为"$shard"的索引，默认为true。                       | EnsureShardingIndex:true&#124;false                                                    |
+---------------------+---------------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------------+


创建集合的格式为：
<pre class="prettyprint lang-diy">
{"name":"<集合名>",[options]} </pre>

**Note:**

* ShardingKey是一个 JSON 对象，JSON 对象中每一个字段对应分区键的字段，其值为1或者-1，代表正向或逆向排序。需要分区，必须指定 ShardingKey。
* ReplSize是一个 JSON 对象，它的值是 int 类型，设置写入数据节点的个数，默认为1，当 ReplSize 等于0时，副本写入个数会根据当前数据组中节点数变化而变化；当 ReplSize 等于-1时，副本写入个数根据活跃副本数变化而变化；手动指定副本写入个数时，不能超出当前组内节点个数。
* Compressed也是一个 JSON 对象，它的值为 boolean 类型，为“true”时，表示集合中的数据压缩存储，“false”时表示正常存储数据。开启压缩时，还可通过CompressionType指定压缩类型，当前可支持的压缩类型有snappy及lzw。不显式指定CompressionType时默认使用snappy压缩。snappy速度较快，而lzw压缩效果较好。
* 当 options 内设置了多个参数时，用逗号（,）隔开。
* name 的值不能是空串，含点（.）或者美元符号（$），并且长度不能超过127B，否则操作失败。
* AutoSplit 必须配合散列分区和域使用，且不能与 Group 同时使用。
* AutoSplit 不能与 Group 同时使用。
* 如果在集合中没有指定 AutoSplit，则使用所属域中的 AutoSplit 参数。
* Group 必须存在于集合空间所属的域中（所有复制组均属于 SYSDOMAIN，即如果集合空间没有指定域，则系统内任意复制组均可）。

## 示例##

* 在集合空间 foo 下创建集合 bar，不指定分区键

<pre class="prettyprint lang-javascript">
> db.foo.createCL("bar")</pre>

* 在集合空间 foo 下创建集合 bar，指定字段 age 为分区键，升序排序

<pre class="prettyprint lang-javascript">
> db.foo.createCL("bar",{ShardingKey:{"age":1},ShardingType:"hash",Partition:1024,ReplSize:1,Compressed:true})</pre>
## 语法##
***db.collectionspace.dropCL(&lt;name&gt;)***

删除指定集合空间下指定的集合。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ |------ |
| name | string | 集合名，在同一个集合空间中，集合名必须唯一。 | 是 |

## 格式##

dropCL() 方法的定义格式必须指定 name 参数，并且 name 的值在集合空间中存在，否则操作异常。

<pre class="prettyprint lang-diy">
{"name":"<集合名>"}</pre>

**Note:**

* name 的值不能是空串，含点（.）或者美元符号（$），并且长度不能超过127B，否则操作失败
* 集合名必须在集合空间中存在，否则操作异常

## 示例##

* 删除集合空间 foo 下的集合 bar。假定集合存在

<pre class="prettyprint lang-javascript">
> db.foo.dropCL("bar")</pre>
## 语法##
***db.collectionspace.getCL(&lt;name&gt;)***

返回指定集合空间下集合的引用。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ |------ |
| name | string | 集合名，在同一个集合空间中，集合名必须唯一。 | 是 |

## 格式##

getCL() 方法的定义格式必须指定 name 参数，并且 name 的值在集合空间中存在，否则操作异常。

<pre class="prettyprint lang-diy">
{"name":"<集合名>"}</pre>

**Note:**

* name 的值不能是空串、含点（.）或者美元符号（$），并且长度不能超过127B，否则操作失败。
* 集合名必须在集合空间中存在，否则操作异常。

## 示例##

* 返回集合空间 foo 下集合 bar 的引用。假定集合存在。

<pre class="prettyprint lang-javascript">
> db.foo.getCL("bar")</pre>
## 语法##
***db.collectionspace.collection.aggregate( &lt;subOp&gt;... )***

aggregate() 方法与 find() 方法功能比较接近，也是从 SequoiaDB 的集合中检索文档记录，并返回游标。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| subOp | json 对象 | subOp 表示子操作，在 aggregate() 方法中可以填写 1~N 个子操作。| 是 |

## 格式##

aggregate() 方法只有一个参数 subOp，它表示 1~N 个子操作，每个子操作是一个 JSON 对象，子操作之间用逗号隔开。聚集框架支持以下子操作参数：

+----------+---------------------------------------------------------------------------+------------------------------------------------+
| 参数名   | 描述                                                                      | 示例                                           |
+==========+===========================================================================+================================================+
| $project | 选择需要输出的字段名，“1”表示输出，“0”表示不输出，还可以实现字段的重命名。| {$project:{field1:1,field:0,aliase:"$field3"}} |
+----------+---------------------------------------------------------------------------+------------------------------------------------+
| $match   | 实现从集合中选择匹配条件的记录，相当与 SQL 语句的 where。                 | {$match:{field:{$lte:value}}}                  |
+----------+---------------------------------------------------------------------------+------------------------------------------------+
| $limit   | 限制返回的记录条数。                                                      | {$limit:10}                                    |
+----------+---------------------------------------------------------------------------+------------------------------------------------+
| $skip    | 控制结果集的开始点，即跳过结果集中指定条数的记录。                        | {$skip:5}                                      |
+----------+---------------------------------------------------------------------------+------------------------------------------------+
| $group   | 实现对记录的分组，类似与 SQL 的 group by 语句，“_id”指定分组字段。        | {$group:{_id:"$field"}}                        |
+----------+---------------------------------------------------------------------------+------------------------------------------------+
| $sort    | 实现对结果集的排序，“1”代表升序，“-1”代表降序。                           | {$sort:{field1:1,field2:-1,...}}               |
+----------+---------------------------------------------------------------------------+------------------------------------------------+

**说明：**

aggregate() 方法可以有任意多个子操作，但是注意各子操作的参数名的语法规则。

## 示例##

假设集合 collection 包含如下格式的记录：

<pre class="prettyprint lang-diy">
{
  no:1000,
  score:80,
  interest:["basketball","football"],
  major:"计算机科学与技术",
  dep:"计算机学院",
  info:
  {
    name:"Tom",
    age:25,
    gender:"男"
  }
}</pre>

* 按条件选择记录，并指定返回字段名

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$match:{$and:[{no:{$gt:1002}},{no:{$lt:1015}},{dep:"计算机学院"}]}},{$project:{no:1,"info.name":1,major:1}})</pre>

此聚集操作操作首先使用 \$match 选择匹配条件的记录，然后使用 \$project 只返回指定的字段名。返回结果集如下：

<pre class="prettyprint lang-diy">
{
  "no": 1003,
  "info.name": "Sam",
  "major": "计算机软件与理论"
}
{
  "no": 1004,
  "info.name": "Coll",
  "major": "计算机工程"
}
{
  "no": 1005,
  "info.name": "Jim",
  "major": "计算机工程"
}</pre>

* 按条件选择记录，并对记录进行分组

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$match:{dep:"计算机学院"}},{$group:{_id:"$major",Major:{$first:"$major"},avg_age:{$avg:"$info.age"}}})</pre>

此聚集操作首先使用 \$match 选择匹配条件的记录，然后使用 \$group 对记录按字段 major 进行分组，并使用 \$avg 返回每个分组中嵌套对象 age 字段的平均值。

<pre class="prettyprint lang-diy">
{
  "Major": "计算机工程",
  "avg_age": 25
}
{
  "Major": "计算机科学与技术",
  "avg_age": 22.5
}
{
  "Major": "计算机软件与理论",
  "avg_age": 26
}</pre>

* 按条件选择记录，并对记录进行分组、排序、限制返回记录的起始位置和返回记录数

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$match:{interest:{$exists:1}}},{$group:{_id:"$major",avg_age:{$avg:"$info.age"},major:{$first:"$major"}}},{$sort:{avg_age:-1,major:-1}},{$skip:2},{$limit:3})</pre>

此聚集操作首先按 \$match 选择匹配条件的记录；然后使用 \$group 按 major 进行分组，并使用 \$avg 返回每个分组中嵌套对象 age 字段的平均值，输出字段名为 avg_age； 最后使用 \$sort 按 avg_age 字段值（降序），major 字段值（降序）对结果集进行排序，使用 \$skip 确定返回记录的起始位置，使用 \$limit 限制返回记录的条数。

<pre class="prettyprint lang-diy">
{
  "avg_age": 25,
  "major": "计算机科学与技术"
}
{
  "avg_age": 22,
  "major": "计算机软件与理论"
}
{
  "avg_age": 22,
  "major": "物理学"
}</pre>
## 语法##
***db.collectionspace.collection.alter(&lt;options&gt;)***

修改集合的属性。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 修改的属性。 | 是 |

### options 中可选的属性格式###

| 参数名 | 描述 | 格式 |
| ------ | ------ | ------ |
| ReplSize | 一次写请求完成副本数。 | ReplSize: &lt;int32&gt; |
| ShardingKey | 分区键 | ShardingKey:{&lt;字段1&gt;:&lt;1\|-1&gt;,[&lt;字段2&gt;:&lt;1\|-1&gt;, ...]} |
| ShardingType | 分区方式，默认为 range 分区。 | ShardingType:"hash"\|"range" |
| Partition | 分区数，hash 分区时填写，代表了 hash 分区的个数。其值必须是2的幂。范围在[2^3 , 2^20]。 | Partition:&lt;分区数&gt; |

**Note:**

* ShardingKey，ShardingType，Partition 的使用方式见 db.collectionspace.createCL()。
* 分区集合不能修改与分区相关的属性。
* 修改为分区集合后需要手动进行 split。

## 示例##

* 创建一个普通集合；

<pre class="prettyprint lang-javascript">
> db.foo.createCL('bar')</pre>

* 修改为分区集合

<pre class="prettyprint lang-javascript">
> db.foo.bar.alter({ShardingKey:{a:1},ShardingType:"hash"})</pre>
## 语法##
***db.collectionspace.collection.attachCL(&lt;subCLFullName&gt;, &lt;options&gt;)***

在主分区集合下挂载子分区集合。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| subCLFullName | string | 子分区集合名（包含集合空间名） | 是 |
| options | Json 对象 |  分区范围，包含两个字段“LowBound”（区间左值）以及“UpBound”（区间右值），例如：{LowBound:{a:0},UpBound:{a:100}}表示取字段“a”的范围区间：[0, 100) | 是 |

## 示例##

* 在主分区集合的指定区间下挂载子分区集合

<pre class="prettyprint lang-javascript">
> db.foo.year.attachCL("foo2.January",{LowBound:{date:"20130101"},UpBound:{date:"20130131"}})</pre>
##语法##
***db.collectionspace.collection.count([cond])***

统计指定集合空间下指定集合的记录总数。

##参数描述##

参数名   参数类型    描述                                                                          是否必填
-------- ----------- ----------------------------------------------------------------------------- ----------
cond     Json 对象   选择条件。为空时，统计集合下所有的记录总数；不为空时，统计符合条件的记录总数。   否

##格式##

count() 方法的定义格式包含 cond 字段，它是一个 JSON 对象。

<pre class="prettyprint lang-diy">
{[{"字段名1":{"匹配符1":"值1"},"字段名2":{"匹配符2":"值2"},...}]}</pre>

##示例##

-   统计集合 bar 所有的记录数，即不指定参数 cond

<pre class="prettyprint lang-javascript">
> db.foo.bar.count()</pre>

-   统计符合条件 name 字段的值为“Tom”且 age 字段的值大于25的记录数

<pre class="prettyprint lang-javascript">
> db.foo.bar.count({name:"Tom",age:{$gt:25}})</pre>
##语法##
***db.collectionspace.collection.createIdIndex()***

在 SequoiaDB 中创建集合时可以根据需要将 AutoIndexId 置为 false。这样集合将不会创建默认的“$id”索引，同时数据的更新、删除操作将被禁止。本方法可以恢复“&#36;id”索引，同时开放更新和删除功能。

##参数描述##

参数名          参数类型       解决方法                                                                         是否必填
--------------- -------------- -------------------------------------------------------------------------------- --------
SortBufferSize  int            创建索引时使用的排序缓存的大小，单位为MB。取值为0时表示不使用排序缓存。默认为64。否

##常见的错误##

错误码   可能的原因               解决方法
-------- ------------------------ --------------------
-247     $id 索引已经存在         -
-291     存在一个相同定义的索引   删除定义冲突的索引

##示例##

* 使用默认参数构建 $id 索引:

<pre class="prettyprint lang-javascript">
> db.foo.bar.createIdIndex();</pre>

* 使用离线方式构建 $id 索引:

<pre class="prettyprint lang-javascript">
> db.foo.bar.createIdIndex({SortBufferSize:128})</pre>
## 语法##
***db.collectionspace.collection.createIndex(&lt;name&gt;,&lt;indexDef&gt;,[isUnique],[enforced],[sortBufferSize])***

为集合创建索引，提高查询速度。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 索引名，同一个集合中的索引名必须唯一。 | 是 |
| indexDef | Json 对象 |  索引键，包含一个或多个指定索引字段与方向的对象。其中方向为1代表从小到大排序，-1则为从大到小排序。 | 是 |
| isUnique | Boolean | 索引是否唯一，默认 false。设置为 true 时代表该索引为唯一索引。 | 否 |
| enforced | Boolean | 索引是否强制唯一，可选参数，在 isUnique 为 true 时生效，默认 false。设置为 true 时代表该索引在 isUnique 为 true 的前提下，不可存在一个以上全空的索引键。 | 否 |
| sortBufferSize | int | 创建索引时使用的排序缓存的大小，单位为MB。取值为0时表示不使用排序缓存。默认为64。| 否 |

## 格式##

createIndex() 方法定义包含 name，indexDef，isUnique 三个参数， 其中 name 的值必须为字符串；indexDef 则为一个 JSON 对象，indexDef 的对象必须包含至少一个字段，其中字段名为用户需要索引的字段名，其值为1或者-1。其中1代表升序，-1代表降序；isUnique 为布尔类型，默认 false。

<pre class="prettyprint lang-diy">
{"name":"&lt;索引名&gt;","indexDef":{"&lt;索引字段1&gt;":&lt;1|-1&gt; [,"&lt;索引字段2&gt;":&lt;1|-1&gt;...] },["isUnique":&lt;true|false&gt;],["enforced":&lt;true|false&gt;],["sortBufferSize":&lt;排序缓存大小&gt;]}</pre>

**Note:**

* 在唯一索引所指定的索引键字段上，集合中不可存在一条以上的记录完全重复
* 索引名不能是空串，含点（.）或者美元符号（$）。且长度不超过1023B
* 索引的key最大大小为1000字符。如 {a:"fjsdlfjlsdfj....."}，如果a为索引字段，当插入记录含有a字段，并且该字段对应的value的大小超过1000字符时，无法插入该记录。
* 在集合记录数据量较大时（大于1000万条记录）适当增大排序缓存大小可以提高创建索引的速度

## 示例##

* 在集合 bar 下为字段名 age 创建名为 ageIndex 的唯一索引，记录按 age 字段值的升序排序。

<pre class="prettyprint lang-javascript">
db.foo.bar.createIndex("ageIndex",{age:1},true)</pre>
##语法##
***db.collectionspace.collection.deleteLob(&lt;oid&gt;)***

删除集合中的大对象。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| oid | string | 大对象的唯一描述符。 | 是 |

## 示例##

* 删除一个描述符为 5435e7b69487faa663000897 的大对象

<pre class="prettyprint lang-javascript">
> db.foo.bar.deleteLob('5435e7b69487faa663000897')</pre>
## 语法##
***db.collectionspace.collection.detachCL(&lt;subCLFullName&gt;)***

从主分区集合中分离出子分区集合。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| partitionName | string | 子分区名（原子分区集合名） | 是 |

## 示例##

* 从主分区集合中分离指定子分区

<pre class="prettyprint lang-javascript">
> db.foo.year.detachCL("foo2.January")</pre>
##语法##
***db.collectionspace.collection.dropIdIndex()***

删除集合中的 $id 索引，同时禁止更新、删除操作。

##常见的错误##

错误码   可能的原因      解决方法
-------- --------------- ----------
-47      $id索引不存在   -
##语法##
***db.collectionspace.collection.dropIndex(&lt;name&gt;)***

删除集合中指定的索引。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 索引名，同一个集合中的索引名必须唯一。 | 是 |

## 格式##

dropIndex() 方法的定义格式必须包含 name 字段。其中 name 的值必须为字符串。
<pre class="prettyprint lang-diy">
{"name":"<索引名>"}</pre>

**Note:**

* 做删除索引操作时，索引名必须在集合中存在。
* 索引名不能是空串，含点（.）或者美元符号（$），且长度不超过127B。

## 示例

* 删除集合 bar 下名为 ageIndex 的索引，假设 ageIndex 已存在。

<pre class="prettyprint lang-javascript">
> db.foo.bar.dropIndex("ageIndex")</pre>
##语法##
***db.collectionspace.collection.find([cond],[sel])***

选择集合记录，对选择的记录返回一个游标（cursor）。在 SequoiaDB中 游标是一个指针，指向一个查询结果集，客户端可以遍历检索结果。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| cond | Json 对象 | 选择条件。为空时，查询所有记录，不为空时，查询符合条件记录。 | 否 |
| sel | Json 对象 | 控制返回字段名。为空时，返回记录的所有字段，如果指定的字段名不在记录中，返回。 | 否 |

## 格式##

find() 的定义格式包含 cond 和 sel 两个参数，都是 JSON 对象类型。cond 控制符合条件的记录，sel 控制返回记录的字段名。

<pre class="prettyprint lang-diy">
{[{"字段名1":{"匹配符1":"值1","字段名2":{"匹配符2":"值2"},...}],[{"字段名1":"","字段名2":"",..}]}</pre>

**Note:**

* sel 是一个 Json 对象，字段的值一般设定为空。而如果指定值：{"字段名1":"值1","字段名2":"值2",...}，如果记录中存在所选字段，设定的值（值1，值2...）不生效；如果记录中不存在所选字段，则按指定的值输出。

## 示例##

* 查询所有记录，不指定 cond 和 sel 字段。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find()</pre>


* 查询匹配条件的记录，即设置 cond 参数的内容。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:25},name:"Tom"})</pre>

此操作返回集合 bar 中符合条件 age 字段值大于25且 name 字段值为“Tom”的记录。

* 指定返回的字段名，即设置 sel 参数的内容。如有记录

<pre class="prettyprint lang-diy">
{age:25,type:"system"}和{age:20,name:"Tom",type:"normal"}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.find(null,{age:"",name:""})</pre>

此操作返回记录的 age 字段和 name 字段，执行后返回：

<pre class="prettyprint lang-diy">
{age:25,name:""}，{age:20,name:"Tom"}</pre>

虽然第一条记录没有 name 字段，还是会返回 

<pre class="prettyprint lang-diy">
name:""</pre>
##语法##
***db.collectionspace.collection.findOne([cond],[sel])***

此方法的使用与 [find()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/find.html) 相同，具体的使用可以参照 find() 方法。但该操作方法只返回符合查询条件的一条记录。
##语法##
***db.collectionspace.collection.getIndex(&lt;name&gt;)***

返回指定索引的引用。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | strsing | 索引名，同一个集合中的索引名必须唯一。 | 是 |

## 格式##

getIndex() 方法的定义格式必须包含 name 字段。其中 name 的值必须为字符串。
<pre class="prettyprint lang-diy">
{"name":"<索引名>"}</pre>

**Note:**

* 在做返回索引引用操作时，索引名必须在集合中存在。
* 索引名不能是空串，含点（.）或者美元符号（$），且长度不超过127B。

## 示例##

* 返回集合 bar 下名为 ageIndex 索引的引用，假设 ageIndex 已存在。

<pre class="prettyprint lang-javascript">
> db.foo.bar.getIndex("ageIndex")</pre>
## 语法##
***db.collectionspace.collection.getLob(&lt;oid&gt;,&lt;file path&gt;,[forced])***

读取集合中的大对象。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| oid | string | 大对象的唯一描述符。 | 是 |
| file path | string | 待写入的本地文件全路径。 | 是 |
| forced | bool | 本地文件如果已经存在是否强制覆盖。 | 否 |

**Note:**

* 本地文件不需要事先手工创建。
* forced 默认为 false。

## 示例##

* 将标示符为 5435e7b69487faa663000897 的 lob 写入本地 /opt/newlob 文件

<pre class="prettyprint lang-javascript">
> db.foo.bar.getLob('5435e7b69487faa663000897','/opt/newlob')</pre>
##语法##
***db.collectionspace.collection.insert(&lt;doc|docs&gt;,[flag])***

向指定集合中插入记录。如果集合空间或集合不存在，首先需要手动创建一个集合空间，如 db.createCS("foo")，再在该集合空间下手动创建集合，如 db.foo.createCL("bar")。然后在集合中插入记录。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| doc|docs | Json 对象 | 文档记录。doc 为一条记录，docs 为多条记录。 | 是 |
| flag | Int | 可取 SDB_INSERT_RETURN_ID 或者 SDB_INSERT_CONTONDUP。前者在插入单条记录时有效，表示插入记录后返回记录中“\_id”字段内容；后者在插入多条记录时有效，表示在插入的记录中，若存在“\_id”字段内容重复的记录时，将跳过这些存在重复“\_id”的记录继续插入后面记录。默认情况下，当存在重复“\_id”字段内容的记录时，将停止插入后面的记录。 | 否 |

## 格式##

insert() 方法的定义格式包含 doc|docs 和 flag 两个字段。

doc：
<pre class="prettyprint lang-diy">
{"&lt;字段名 1&gt;":"&lt;值&gt;","&lt;字段名 2>":"&lt;值&gt;",…}</pre>

docs：
<pre class="prettyprint lang-diy">
{ 
    [
        {"&lt;字段名 1&gt;":"&lt;值&gt;","&lt;字段名 2>":"&lt;值&gt;",…},
        {"&lt;字段名 1&gt;":"&lt;值&gt;","&lt;字段名 2>":"&lt;值&gt;",…},
        ...
    ] 
}</pre>

**Note:**

如果插入的记录不指定 \_id 字段时，SequoiaDB 会自动为记录添加一个 \_id 字段来标识记录的唯一性。

## 示例##

* 不指定 _id 字段，插入一条记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({name:"Tom",age:20})</pre>

此操作是向集合 bar 中插入一条新的记录，name 字段的值为“Tom”，age 字段的值为20，_id 字段被唯一创建：
<pre class="prettyprint lang-diy">
{ "_id": { "$oid": "515152ba49af395200000000" }, "name": "Tom", "age": 20 }</pre>

* 插入一条带有 _id 字段的记录。
<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({_id:10,age:20})</pre>

此操作是向集合 bar 中插入一条新的记录，_id 字段的值为10，age 字段的值为20：
<pre class="prettyprint lang-diy">
{ "_id": 10, "age": 20 }</pre>

* 插入多条记录。
<pre class="prettyprint lang-javascript">
db.foo.bar.insert([{_id:20,name:"Mike",age:15},{name:"John",age:25,phone:123}])</pre>

此操作将会在集合 bar 中插入两条记录：

1）其中一条记录 _id 字段的值为20，name 字段的值为“Mike”，age 字段的值为15。

2）一条记录系统自动为 _id 字段生成唯一值，name 字段的值为“John”，age 字段的值为25，phone 字段的值为123。
<pre class="prettyprint lang-diy">
{
    "_id": 20,
    "name": "Mike",
    "age": 15
}
{
    "_id": { "$oid": "5151557a49af395200000001" },
    "name": "John",
    "age": 25,
    "phone": 123
}</pre>


* 插入拥有重复“_id”键的多条记录。
<pre class="prettyprint lang-javascript">
> db.foo.bar.insert([{_id:1,a:1 },{_id:1,b:2 },{_id:3,c:3}], SDB_INSERT_CONTONDUP)</pre>

此操作将会在集合 bar 中插入两条记录：
<pre class="prettyprint lang-diy">
{
    "_id": 1,
    "a": 1,
}
{
    "_id": 3,
    "c": 3
}</pre>
##语法##
***db.collectionspace.collection.listIndexes()***

枚举索引，执行此方法会将指定集合下的索引信息全部显示出来。

## 示例##

* 返回集合 bar 下的所有索引信息

<pre class="prettyprint lang-javascript">
> db.foo.bar.listIndexes()</pre>
##语法##
***db.collectionspace.collection.listLobs()***

枚举集合中的大对象。

## 参数描述##
| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| - | - | - | - |

**Note:**

此方法暂不支持排序等查询操作。

## 示例##

* 枚举 foo.bar 中的所有大对象

<pre class="prettyprint lang-javascript">
> db.foo.bar.listLobs()</pre>
##语法##
***db.collectionspace.collection.putLob(&lt;file path&gt;)***

在集合中插入大对象。

## 参数描述##
| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| file path | string | 待上传的文件全路径。 | 是 |

**Note:**

* 上传大对象成功后会返回其 OID。
* 需要拥有文件的读权限。

## 示例##

* 创建集合空间与集合

<pre class="prettyprint lang-javascript">
> db.createCS('foo')
> db.foo.createCL('bar')</pre>

* 上传大对象文件

<pre class="prettyprint lang-javascript">
> db.foo.bar.putLob('/opt/mylob');</pre>
## 语法##
***db.collectionspace.collection.remove([cond],[hint])***

删除集合中的记录。

## 参数描述##
| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| cond | Json 对象 | 选择条件。为空时，删除所有记录，不为空时，删除符合条件的记录。 | 否 |
| hint | Json 对象 | 指定访问计划。 | 否 |

## 格式##

cond 参数是一个Json 的对象。当它的内容为空（例如{}）时，删除集合下所有的记录。hint 参数是一个包含一个单一元素的 Json 对象，该元素的字段名会被忽略，而其字段值则指定为需要访问索引的名称，当字段值为 null 时，则遍历集合中所有的记录。

<pre class="prettyprint lang-diy">
{[{"字段名1":{"匹配符1":"值1","字段名2":{"匹配符2":"值2"},...}],[{"":"索引名"|null}]}</pre>

## 示例##

* 删除集合所有记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.remove()</pre>

* 按访问计划删除匹配 cond 条件的记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.remove({age:{$gte:20}},{"":"myIndex"})</pre>

此操作按照索引名为“myIndex”的索引遍历集合中的记录，在遍历得到的记录中删除符合条件 age 字段值大于等于20的记录。
##语法##
***db.collectionspace.collection.split(&lt;source group&gt;,&lt;target group&gt;,&lt;percent(0~100)|condition&gt;, [endcondition])***

在至少存在两个分区组的环境下，将数据记录按指定的条件切分到不同的分区组中。该操作为同步操作，直到数据切分完成才返回。

## 参数描述##
| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| source group | string | 源分区组。 | 是 |
| target group | string | 目标分区组。 | 是 |
| percent(0~100] | double | 百分比切分条件。 | percent 和 condition 二选一 |
| condition | Json 对象 | 范围切分条件。 | condition 和 percent 二选一 |
| endcondition | Json 对象 | 结束范围条件。| 可选，且只在按条件切分时有效，按百分比切分时无效 |

## 格式##

数据切分分为范围切分和百分比切分，其中“source group”和“target group”是公共参数，都是字符串类型；参数 condition 和 endcondition 为范围切分时填入，是一个 Json 结构的对象；参数 percent 为百分比切分时填入，是一个双精度浮点型数值。

* 范围切分

范围切分时，Range 分区使用精确条件，而 Hash 分区使用 Partition（分区数）条件。切分时起始条件为必填字段，而结束条件为选填条件，结束条件默认为切分源当前包含的最大数据范围。

<pre class="prettyprint lang-diy">
("<源分区组>","<目标分区组>"，&lt;condition&gt;|&lt;Partition&gt;) </pre>

**Note:**

范围分区时，如果指定分区键字段为降序时，如：{groupingKey:{<字段1>:&lt;-1&gt;}，condition（或 Partition）中的起始条件中的范围应该大于终止条件中的范围。Hash 分区使用的 Partition（分区数）必须为整形，不能为其他的类型。

* 百分比切分

<pre class="prettyprint lang-javascript">
> db.foo.bar.split("<源分区组>","<目标分区组>"，&lt;percent&gt;)</pre>

## 示例##

* Hash 分区范围切分

<pre class="prettyprint lang-javascript">
> db.foo.bar.split("group1", "group2",{Partition:10},{Partition:20}) </pre>

* Range 分区范围切分

<pre class="prettyprint lang-javascript">
> db.foo.bar.split("group1", "group2",{a:10}, {a:10000})</pre>

* 百分比切分

<pre class="prettyprint lang-javascript">
> db.foo.bar.split("group1", "group2", 50) </pre>

**Note:**

百分比切分时，需要注意：1、需要保证源分区组中含有数据，即集合不为空；2、百分比不能为0。
##语法##
***db.collectionspace.collection. splitAsync(&lt;source group&gt;,&lt;target group&gt;,&lt;percent(0~100)|condition&gt;, [endcondition])***

该操作与 [split()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/split.html) 功能相同，但该操作为异步分区操作，分区任务建立后立即返回任务 ID。
##语法##
***db.collectionspace.collection.truncate()***

truncate 会删除集合内所有数据（包括普通文档和 LOB 数据），但不会影响其元数据。与 remove 需要按照条件筛选目标不同，truncate 会直接释放数据页，在清空集合（尤其是大数据量下）数据时效率比 remove 更加高效。

##示例##

-   我们在集合 foo.bar 中插入了普通数据和 LOB 数据。通过快照查看其数据页使用情况：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_COLLECTIONS) ;
{
  "Name": "foo.bar",
  "Details": [
    {
      "GroupName": "datagroup",
      "Group": [
        {
          "ID": 0,
          "LogicalID": 0,
          "Sequence": 1,
          "Indexes": 1,
          "Status": "Normal",
          "TotalRecords": 10000,
          "TotalDataPages": 33,
          "TotalIndexPages": 7,
          "TotalLobPages": 36,
          "TotalDataFreeSpace": 41500,
          "TotalIndexFreeSpace": 103090
        }
      ]
    }
  ]
}</pre>

可以看到其中数据页为33，索引页为7，LOB 页为36。下面执行 truncate 操作：

<pre class="prettyprint lang-javascript">
> db.foo.bar.truncate();</pre>

再次通过快照查看数据页使用情况：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_COLLECTIONS) ;
{
  "Name": "foo.bar",
  "Details": [
    {
      "GroupName": "datagroup",
      "Group": [
        {
          "ID": 0,
          "LogicalID": 0,
          "Sequence": 1,
          "Indexes": 1,
          "Status": "Normal",
          "TotalRecords": 0,
          "TotalDataPages": 0,
          "TotalIndexPages": 2,
          "TotalLobPages": 0,
          "TotalDataFreeSpace": 0,
          "TotalIndexFreeSpace": 65515
        }
      ]
    }
  ]
}</pre>

可以看到除索引页为2（存储了索引的元数据信息）外，其余数据页已经全部被释放了。
##语法##
***db.collectionspace.collection.update(&lt;rule&gt;,[cond],[hint])***

更新集合记录。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| rule | Json 对象 | 更新规则。记录按 rule 的内容更新。 | 是 |
| cond | Json 对象 | 选择条件。为空时，更新所有记录，不为空时，更新符合条件的记录。 | 否 |
| hint | Json 对象 | 指定访问计划。 | 否 |

## 格式##

update() 方法的定义必须包含 rule 字段，rule 是一个 Json 对象。cond 和 hint 字段可选。hint 参数是一个包含一个单一字段的 Json 对象，字段名会被忽略，而其字段值则指定为需要访问索引的名称，当字段值为 null 时，则遍历集合中所有的记录，它的格式为{"":null}或者{"":"&lt;indexname&gt;"}。

<pre class="prettyprint lang-diy">
{&lt;{""更新符1"":{字段名1:"值"},"更新符2":{"字段名2":"值2"},...}&gt;,[{"字段名1":{"匹配符1":"值1"},"字段名2":{"匹配符2":"值2"},...}],[{"":"索引名"|null}]}</pre>

**Note:**

update 本版本暂不支持对表分区键（ShardingKey）字段更新，如果包含对分区键的更新操作，将自动剔除掉对分区键的更新，但其他字段更新生效，且不会发生错误。

## 示例##

* 按指定的更新规则更新集合中所有记录，即设置 rule 参数，不设定 cond 和 hint 参数的内容

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$inc:{age:1}})</pre>

此操作更新集合 bar 下的 age 字段，将 age 字段的值增加1。

* 选择符合匹配条件的记录，对这些记录按更新规则更新，即设定 rule 和 cond 参数

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{age:""}},{age:{$exists:1},name:{$exists:0}})</pre>

此操作更新集合 bar 中存在 age 字段而不存在 name 字段的记录，将这些记录的 age 字段删除。

* 按访问计划更新记录，假设集合中存在指定的索引名

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$inc:{age:1}},{age:{$gt:20}},{"":"testIndex"})></pre>

此操作使用索引名为 testIndex 的索引访问集合 bar 中 age 字段值大于20的记录，将这些记录的 age 字段名加1。
##语法##
***db.collectionspace.collection.upsert(&lt;rule&gt;,[cond],[hint],[setOnInsert])***

更新集合记录。upsert 方法跟 update 方法都是对记录进行更新，不同的是当使用 cond 参数在集合中匹配不到记录时，update 不做任何操作，而 upsert 方法会做一次插入操作。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| rule | Json 对象 | 更新规则。记录按 rule 的内容更新。 | 是 |
| cond | Json 对象 | 选择条件。为空时，更新所有记录，不为空时，更新符合条件的记录。 | 否 |
| hint | Json 对象 | 指定访问计划。 | 否 |
| setOnInsert | Json 对象 | 在做插入操作时向插入的记录中追加字段。 | 否 |

## 格式##

upsert() 方法的定义必须包含 rule 字段，rule 是一个 Json 对象。cond 和 hint 字段可选。hint 参数是一个包含一个单一字段的 Json 对象，字段名会被忽略，而其字段值则指定为需要访问索引的名称，当字段值为 null 时，则遍历集合中所有的记录，它的格式为{"":null}或者{"":"&lt;indexname&gt;"}。

<pre class="prettyprint lang-diy">
{&lt;{""更新符1"":{"字段名1":"值"},"更新符2":{"字段名2":"值2"},...}&gt;,[{"字段名1":{"匹配符1":"值1"},"字段名2":{"匹配符2":"值2"},...}],[{"":"索引名"|null}]}</pre>

当 cond 参数在集合中匹配不到记录时，upsert 会生成一条记录插入到集合中。记录生成规则为：首先从 cond 参数中取出 &#36;et 和 &#36;all 操作符对应的键值对，然后使用 rule 规则对其做更新操作，最后加入 setOnInsert 中的键值对。

**Note:**

upsert 本版本暂不支持对表分区键（ShardingKey）字段更新，如果包含对分区键的更新操作，将自动剔除掉对分区键的更新，但其他字段更新生效，且不会发生错误。

## 示例##

假设集合 bar 中有两条记录：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "516a76a1c9565daf06030000"
  },
  "age": 10,
  "name": "Tom"
}
{
  "_id": {
    "$oid": "516a76a1c9565daf06050000"
  },
  "a": 10,
  "age": 21
}</pre>

* 按指定的更新规则更新集合中所有记录，即设置 rule 参数，不设定 cond 和 hint 参数的内容

<pre class="prettyprint lang-javascript">
> db.foo.bar.upsert({$inc:{age:1},$set:{name:"Mike"}})</pre>

此操作等效于使用 update 方法，更新集合 bar 中的所有记录，将记录的 age 字段值加1，name 字段值更改为“Mike”，对不存在 name 字段的记录，$set 操作符会将 name 字段和其设定的值插入到记录中，使用 find 方法返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "516a76a1c9565daf06030000"
  },
  "age": 11,
  "name": "Mike"
}
{
  "_id": {
    "$oid": "516a76a1c9565daf06050000"
  },
  "a": 10,
  "age": 22,
  "name":"Mike"
}</pre>

* 选择符合匹配条件的记录，对这些记录按更新规则更新，即设定 rule 和 cond 参数

<pre class="prettyprint lang-javascript">
> db.foo.bar.upsert({$inc:{age:3}},{type:{$exists:1}})</pre>

此操作更新集合 bar 中存在 type 字段的记录，将这些记录的 age 字段值加3。在上面给出的两条记录中，都没有 type 字段，此时，upsert 操作会插入一条新的记录，新记录只有 \_id 字段和 age 字段名，\_id 字段值自动生成，而 age 字段值为3。

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "516a76a1c9565daf06030000"
  },
  "age": 11,
  "name": "Mike"
  }
  {
    "_id": {
      "$oid": "516a76a1c9565daf06050000"
    },
    "a": 10,
    "age": 22,
    "name":"Mike"
  }
{
  "_id": {
    "$oid": "516cfc334630a7f338c169b0"
  },
  "age": 3
}</pre>

* 按访问计划更新记录，假设集合中存在指定的索引名 testIndex

<pre class="prettyprint lang-javascript">
> db.foo.bar.upsert({$inc:{age:1}},{age:{$gt:20}},{"":"testIndex"})</pre>

此操作等效于使用 update 方法，使用索引名为 testIndex 的索引访问集合 bar 中 age 字段值大于20的记录，将这些记录的 age 字段名加1。此时返回：
<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "516a76a1c9565daf06050000"
  },
  "a": 10,
  "age": 23,
  "name":"Mike"
} </pre>

* 使用setOnInsert更新记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.upsert({$inc:{age:1}},{age:{$gt:30}},{}, {"name":"Mike"})</pre>

由于集合 bar 中 age 字段值大于30的记录为空，upsert在做插入操作时向插入的记录中追加字段{"name":"Mike"}。此时返回：
<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "516a76a1c9565daf06050000"
  },
  "age":1,
  "name":"Mike"
} </pre>
##语法##
***cursor.close()***

关闭当前游标，当前游标不再可用。

## 示例##

* 插入10条记录

<pre class="prettyprint lang-javascript">
> for(i = 0; i < 10; i++) { db.foo.bar.insert({a:i}) }</pre>

查询集合 foo.bar 的所有记录

<pre class="prettyprint lang-javascript">
> var cur = db.foo.bar.find()</pre>

使用游标取出一条记录

<pre class="prettyprint lang-javascript">
> cur.next()

{
  "_id": {
  "$oid": "53b3c2d7bb65d2f74c000000"
  },
  "a": 0
}</pre>

关闭游标

<pre class="prettyprint lang-javascript">
> cur.close()</pre>

再次获取下一条记录

<pre class="prettyprint lang-javascript">
> cur.next()</pre>

无结果返回
##语法##
***cursor.current()***

返回当前游标指向的记录。更多查看 cursor.next() 方法。

## 示例##

* 选择集合 bar 下 age 大于10的记录，返回当前游标指向的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).current()</pre>
##语法##
***cursor.next()***

返回当前游标指向的下一条记录。更多查看 cursor.current() 方法。

## 示例##

* 选择集合 bar 下 age 大于10的记录，返回当前游标指向的下一条记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).next()</pre>
##语法##
***query.explain([option])***

返回查询的访问计划。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| option | json 对象 | 访问计划执行参数，目前有 Run 字段项，表示是否执行访问计划，true 表示执行访问计划，获取数据和时间信息，false 表示只获取访问计划的信息，并不执行 | 否，默认为 false |

## 访问计划描述##

| 字段名 | 类型 | 描述 |
| ------ | ------ | ------ |
| Name | string | 集合名 |
| ScanType | string | 扫描方式—— 表扫描：“tbscan”； 索引扫描：“ixscan”  |
| IndexName | string | 使用索引的名称 |
| UseExtSort | bool | 是否使用非索引排序 |
| NodeName | string | 节点名 |
| ReturnNum | int64 | 返回记录数量 |
| ElapsedTime | float64 | 查询耗时（秒） |
| IndexRead | int64 | 索引记录扫描条数 |
| DataRead | int64 | 数据记录扫描条数 |
| UserCPU | float64 | 用户态 CPU 使用时间（秒） |
| SysCPU | float64 | 内核态 CPU 使用时间（秒） |
| SubCollections | Json Array | 垂直分区表中各子表访问计划 |

## 格式##

**Note:**

如果集合经过 split 分布在多个复制组，访问计划会按照一组一记录的方式返回。

## 示例##

* foo.bar 是一个水平分区集合，分布在三个复制组上。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find().sort({b:1}).explain({Run:true})</pre>

返回：
<pre class="prettyprint lang-diy">
{
  "Name": "foo.bar",
  "ScanType": "tbscan",
  "IndexName": "",
  "UseExtSort": true,
  "NodeName": "vmsvr2-cent-x64:40020",
  "ReturnNum": 38,
  "ElapsedTime": 0.000477,
  "IndexRead": 0,
  "DataRead": 38,
  "UserCPU": 0,
  "SysCPU": 0
}
{
  "Name": "foo.bar",
  "ScanType": "tbscan",
  "IndexName": "",
  "UseExtSort": true,
  "NodeName": "vmsvr2-cent-x64:40000",
  "ReturnNum": 34,
  "ElapsedTime": 0.000415,
  "IndexRead": 0,
  "DataRead": 34,
  "UserCPU": 0,
  "SysCPU": 0
}
{
  "Name": "foo.bar",
  "ScanType": "tbscan",
  "IndexName": "",
  "UseExtSort": true,
  "NodeName": "vmsvr2-cent-x64:40010",
  "ReturnNum": 28,
  "ElapsedTime": 0.000517,
  "IndexRead": 0,
  "DataRead": 28,
  "UserCPU": 0,
  "SysCPU": 0
}</pre>
##语法##
***query.count()***

返回符合匹配条件的记录条数。

## 示例##

选择集合 bar 下 age 大于10的记录，返回符合匹配条件{age:{$gt:10}}的记录条数。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).count()</pre>

**Note:**

query.count()返回的结果忽略query.skip()及query.limit()的影响。
##语法##
***query[i]***

以下标的方式访问查询结果集。

## 示例##

以下标的方式访问查询的结果集。

<pre class="prettyprint lang-javascript">
> var query = db.foo.bar.find()
> println( query[0] )</pre>
##语法##
***query.hint(&lt;hint&gt;)***

按指定的索引遍历结果集。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| hint | Json 对象 | 指定访问计划，加快查询速度。 | 否 |

## 格式##

query.hint() 的方法定义包含 hint 参数，如果不设定 hint 参数的内容相当于不使用索引遍历结果集。hint 参数是一个包含一个单一字段的 Json 对象，字段名会被忽略，而其字段值则指定为需要访问索引的名称，当字段值为 null 时，则遍历集合中所有的记录。

格式如下：
<pre class="prettyprint lang-diy">
{"":null} 或者 {"":"&lt;indexname&gt;"}</pre>

##示例##

* 使用索引 ageIndex 遍历集合 bar 下存在 age 字段的记录，并返回。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$exists:1}}).hint({"":"ageIndex"})</pre>
##语法##
***query.limit(&lt;num&gt;)***

控制结果集返回记录的条数。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| num |	int | 自定义返回结果集的记录条数。 | 否 |

## 格式##

query.limit() 方法的定义格式包含 num 参数，它是 int 类型。如果不设定 num 的内容，相当于返回所有的结果集记录。如果想返回结果集的前5条记录，可是设置 num 的值为5。

## 示例##

* 选择集合 bar 下 age 字段值大于10的记录，控制返回前面10条记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).limit(10)</pre>

**Note:**

如果结果集的记录数小于10，按实际的记录数返回，如果结果集的记录数大于10，则返回前10条。
##语法##
***query.remove()***

删除查询结果集。

##示例##

-   查询集合 bar 下 age 字段值大于10的记录，并将符合条件的记录删除。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).remove()</pre>

**Note:**

-    不能与 query.count()、query.update()同时使用。

-    与 query.sort()同时使用时，在单个节点上排序必须使用索引。

-    在集群中与 query.limit()或query.skip()同时使用时，要保证查询条件会在单个节点或单个子表上执行。
##语法##
***query.size()***

返回当前游标到最终游标的记录条数。

## 示例##

* 选择集合 bar 下 age 大于10的记录，返回当前游标到最终游标的记录条数。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).size()</pre>

**Note:**

query.size()返回的结果考虑query.skip()及query.limit()的影响。
##语法##
***query.skip(&lt;num&gt;)***

指定结果集从哪条记录开始返回。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| num | int | 自定义结果集从哪条记录返回。 | 否 |

## 格式##

query.skip() 方法的定义格式包含 num 参数，它是 int 类型。如果不设定 num 的内容或者设定 num 的值为0，相当于返回所有的结果集；如果想从结果集的第3条记录开始返回，可是设置 num 的值等于2。

## 示例##

选择集合 bar 下 age 字段值大于10的记录，从第5条记录开始返回，即跳过前面的四条记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).skip(4)</pre>

**Note:**

如果结果集的记录数小于5，那么无记录返回；如果结果集的记录数大于5，从第5条开始返回。
##语法##
***query.sort(&lt;sort&gt;)***

对结果集按指定字段排序。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
|sort |	Json 对象 | 对结果集按指定字段排序。字段名的值为1或者-1,1代表升序；-1代表降序。 | 否 |

## 格式##

query.sort() 方法的定义格式包含 sort 参数，它是一个 Json 对象。如果不设定 sort 的内容，相当于对返回的结果集不排序。

设定 sort 参数的话，格式如下：

<pre class="prettyprint lang-diy">
{<字段名1>:<-1|1>,<字段名2>:<-1|1>,...}</pre>

**Note:**

当 find() 方法使用 sel 选项，若该选项没有包含 sort() 指定的排序字段，此时 sort() 方法设置的排序无意义，从而被自动忽略。

## 示例##

* 返回集合 bar 中 age 字段值大于20的记录，设置只返回记录的 name 和 age 字段，并按 age 字段值的升序排序。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:20}},{age:"",name:""}).sort({age:1})</pre>

**Note:**

通过 find() 方法，我们能任意选择我们想要返回的字段名，在上例中我们选择了返回记录的 age 和 name 字段，此时用 sort() 方法时，只能对记录的 age 或 name 字段排序。而如果我们选择返回记录的所有字段，即不设置 find 方法的 sel 参数内容时，那么 sort() 能对任意字段排序。

* 指定一个无效的排序字段。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:20}},{age:"",name:""}).sort({"sex":1})</pre>

**Note:**

因为“sex”字段并不存在 find() 方法的 sel 选项 {age:"",name:""} 中，所以 sort() 指定的排序字段 {"sex":1} 将被忽略。
##语法##
***query.toArray()***

以数组的形式返回结果集。


## 示例##

* 以数组的形式返回集合 bar 中 age 字段值大于5的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).toArray()</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "516a76a1c9565daf06030000"
  },
  "age": 10,
  "name": "Tom"
},{
  "_id": {
    "$oid": "516a76a1c9565daf06050000"
  },
  "age": 20,
  "a": 10
},{
  "_id": {
    "$oid": "516a76a1c9565daf06040000"
  },
  "age": 15
}</pre>
##语法##
***query.update(&lt;update&gt;,[returnNew])***

更新查询结果集。

##参数描述##

参数名      参数类型    描述                                   是否必填
----------- ----------- -------------------------------------- ----------
update      Json 对象    更新规则。记录按 update 的内容更新。   是
returnNew   bool        是否返回更新后的记录。                 否

##格式##

query.update()方法的定义格式包含 update 参数和 returnNew 参数。其中 update 参数是 Json 对象，与 [Collection.update()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/update.html)的 rule 参数相同。returnNew 参数可选，为 bool 类型，默认为 false。当为 true 时，返回修改后的记录值。

##示例##

查询集合 bar 下 age 字段值大于10的记录，并将符合条件的记录的 age 字段加1。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:10}}).update({$inc:{age:1}})</pre>

**Note:**

-    不能与 query.count()、query.remove()同时使用。

-    与 query.sort()同时使用时，在单个节点上排序必须使用索引。

-    在集群中与 query.limit()或 query.skip()同时使用时，要保证查询条件会在单个节点或单个子表上执行。
##语法##
***rg.attachNode(&lt;host&gt;,&lt;service&gt;, [options])***

将一个已经创建完成但不属于任何组的节点加入到当前组。搭配 rg.detachNode() 使用。

##参数描述##

参数名    参数类型    描述                        是否必填
--------- ----------- --------------------------- ----------
host      string      节点的主机名或者主机 IP。   是
service   string      节点服务名或者端口。        是
options   Json 对象   attachNode 选项。           否

##options 选项##

参数名     参数类型   描述                           默认值
---------- ---------- ------------------------------ --------
KeepData   bool       是否保留目标节点原有的数据。   false

**Note:**

如果目标节点原本不属于当前组，切勿打开 KeepData。

##常见的错误##

+----------+------------------------+------------------------------+
| 错误码   | 可能的原因             | 解决方法                     |
+==========+========================+==============================+
| -15      | 网络错误               | 1.检查 sdbcm 状态是否正常    |
|          |                        | 2.检查填写的 host 是否正确   |
+----------+------------------------+------------------------------+
| -146     | 节点不存在             | 检查目标节点是否被创建过     |
+----------+------------------------+------------------------------+
| -157     | 节点已存在于其他复制组 | 检查节点是否已属于其他复制组 |
+----------+------------------------+------------------------------+

##示例##

将一个节点从 datagroup1 中分离，加入到 datagroup2 中。

使用 db.listReplicaGroups() 查看当前节点信息；

<pre class="prettyprint lang-diy">
{
"Group": [
  {
    "HostName": "host1",
    "dbpath": "/data1",
    "Service": [
      {
        "Type": 0,
        "Name": "40060"
      },
      {
        "Type": 1,
        "Name": "40061"
      },
      {
        "Type": 2,
        "Name": "40062"
      }
    ],
    "NodeID": 1004
  },
  {
    "HostName": "host1",
    "dbpath": "/data2",
    "Service": [
      {
        "Type": 0,
        "Name": "40070"
      },
      {
        "Type": 1,
        "Name": "40071"
      },
      {
        "Type": 2,
        "Name": "40072"
      }
    ],
    "NodeID": 1005
  }
],
"GroupID": 1002,
"GroupName": "datagroup1",
"PrimaryNode": 1005,
"Role": 0,
"Status": 1,
"Version": 3,
"_id": {
  "$oid": "555d7b71d1cbaf20ed74e7df"
}
}

{
"Group": [
  {
    "HostName": "host1",
    "dbpath": "/data3",
    "Service": [
      {
        "Type": 0,
        "Name": "40040"
      },
      {
        "Type": 1,
        "Name": "40041"
      },
      {
        "Type": 2,
        "Name": "40042"
      }
    ],
    "NodeID": 1003
  }
],
"GroupID": 1001,
"GroupName": "datagroup2",
"PrimaryNode": 1003,
"Role": 0,
"Status": 1,
"Version": 4,
"_id": {
  "$oid": "555d7b5fd1cbaf20ed74e7de"
  }
}</pre>

分离“host:40060”节点；

<pre class="prettyprint lang-javascript">
> db.getRG('datagroup1').detachNode('host1', '40060');</pre>

将节点加入到 datagroup2 中；

<pre class="prettyprint lang-javascript">
> db.getRG('datagroup2').attachNode('host1', '40060')</pre>

使用 db.listReplicaGroups() 查看当前节点信息：

<pre class="prettyprint lang-diy">
{
  "Group": [
    {
      "HostName": "host1",
      "dbpath": "/data2",
      "Service": [
        {
          "Type": 0,
          "Name": "40070"
        },
        {
          "Type": 1,
          "Name": "40071"
        },
        {
          "Type": 2,
          "Name": "40072"
        }
      ],
      "NodeID": 1005
    }
  ],
  "GroupID": 1002,
  "GroupName": "datagroup1",
  "PrimaryNode": 1005,
  "Role": 0,
  "Status": 1,
  "Version": 3,
  "_id": {
    "$oid": "555d7b71d1cbaf20ed74e7df"
  }
}

{
  "Group": [
    {
      "HostName": "host1",
      "dbpath": "/data3",
      "Service": [
        {
          "Type": 0,
          "Name": "40040"
        },
        {
          "Type": 1,
          "Name": "40041"
        },
        {
          "Type": 2,
          "Name": "40042"
        }
      ],
      "NodeID": 1003
    },
    {
      "HostName": "host1",
      "dbpath": "/data1",
      "Service": [
        {
          "Type": 0,
          "Name": "40060"
        },
        {
          "Type": 1,
          "Name": "40061"
        },
        {
          "Type": 2,
          "Name": "40062"
        }
      ],
      "NodeID": 1006
    }
  ],
  "GroupID": 1001,
  "GroupName": "datagroup2",
  "PrimaryNode": 1003,
  "Role": 0,
  "Status": 1,
  "Version": 4,
  "_id": {
    "$oid": "555d7b5fd1cbaf20ed74e7de"
  }
}</pre>
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
##语法##
***rg.detachNode(&lt;host&gt;,&lt;service&gt;, [options])***

分离组内的一个节点。其配置信息不会被删除。搭配 rg.attachNode() 使用。

##参数描述##

参数名    参数类型    描述                        是否必填
--------- ----------- --------------------------- ----------
host      string      节点的主机名或者主机 IP。   是
service   string      节点服务名或者端口。        是
options   Json 对象   attachNode 选项。           否

##options 选项##

参数名     参数类型   描述                           默认值
---------- ---------- ------------------------------ --------
KeepData   bool       是否保留目标节点原有的数据。   false

**Note:**

分离后的节点将不再受集群管理，请尽快加入到其他复制组中。

##常见的错误##

+----------+----------------------+------------------------------------------------------+
| 错误码   | 可能的原因           | 解决方法                                             |
+==========+======================+======================================================+
|          |                      |                                                      |
| -15      | 网络错误             | 1.检查 sdbcm 状态是否正常                            |
|          |                      | 2.检查填写的 host 是否正确                           |
+----------+----------------------+------------------------------------------------------+
| -155     | 节点不属于当前复制组 | 检查节点是否属于当前复制组                           |
+----------+----------------------+------------------------------------------------------+
| -204     | 复制组只包含一个节点 | 当复制组中只剩余一个节点时，无法进行 detachNode 操作 |
+----------+----------------------+------------------------------------------------------+
## 语法##
***rg.getDetail()***

返回分区组的信息。

## 示例##

<pre class="prettyprint lang-javascript">
> rg.getDetail()</pre>

返回：
<pre class="prettyprint lang-diy">
{
  "Group": [
    {
      "dbpath": "/opt/sequoiadb/data/11800",
      "HostName": "vmsvr2-suse-x64",
      "Service": [
        {
          "Type": 0,
          "Name": "11800"
        },
        {
          "Type": 1,
          "Name": "11801"
        },
        {
          "Type": 2,
          "Name": "11802"
        },
        {
          "Type": 3,
          "Name": "11803"
        }
      ],
      "NodeID": 1000
    },
    {
      "dbpath": "/opt/sequoiadb/data/11850",
      "HostName": "vmsvr2-suse-x64",
      "Service": [
        {
          "Type": 0,
          "Name": "11850"
        },
        {
          "Type": 1,
          "Name": "11851"
        },
        {
          "Type": 2,
          "Name": "11852"
        },
        {
          "Type": 3,
          "Name": "11853"
        }
      ],
      "NodeID": 1001
    }
  ],
  "GroupID": 1001,
  "GroupName": "group",
  "PrimaryNode": 1001,
  "Role": 0,
  "Status": 1,
  "Version": 3,
  "_id": {
    "$oid": "517b2fc33d7e6f820fc0eb57"
  }
}</pre>
##语法##
***rg.getMaster()***

返回分区组的主节点。

## 示例##

* 返回分区组的主节点

<pre class="prettyprint lang-javascript">
> rg.getMaster()</pre>

返回：

<pre class="prettyprint lang-diy">
vmsvr2-suse-x64:11850</pre>
##语法##
***rg.getNode(&lt;nodename|hostname&gt;,&lt;servicename&gt;)***

返回指定节点信息。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| nodename | string | 节点名称。 | nodename 与 hostname 二选一 |
| hostname | string | 主机名。 | hostname 与 nodename 二选一 |
| servicename | string | 服务器名称。 | 是 |

## 格式##

rg.getNode() 方法定义了两个参数，第一个参数可是节点名称也可以是主机名，第二个参数为服务器名称。两个参数的类型都是字符串型，且必填。

<pre class="prettyprint lang-diy">
("&lt;节点名称&gt;|&lt;主机名&gt;","&lt;服务器名称&gt;")</pre>

## 示例##

* 返回指定主机名和服务器名的节点

<pre class="prettyprint lang-javascript">
> rg.getNode("vmsvr2-suse-x64","11800")</pre>

返回：

<pre class="prettyprint lang-diy">
vmsvr2-suse-x64:11800</pre>
##语法##
***rg.getSlave()***

返回分区组的从节点。

## 示例##

*  返回分区组的从节点

<pre class="prettyprint lang-javascript">
> rg.getSlave()</pre>

返回：

<pre class="prettyprint lang-diy">
vmsvr2-suse-x64:11800</pre>
##语法##
***rg.reelect([options])***

在复制组中进行重新选举。

##参数描述##

参数名    参数类型    描述         是否必填
--------- ----------- ------------ ----------
options   Json 对象   参数集合。   否

##options 选项##

参数名    参数类型   描述                           默认值
--------- ---------- ------------------------------ --------
Seconds   int        重新选举需要在多少秒内完成。   30

**Note:**

-   返回超时错误代表在规定时间内重选没有完成。通过 db.listReplicaGroups() 观察最终结果。
-   只有复制组中存在主节点时才可以进行重新选举。

##示例##

-   在“datagroup1”中进行重新选举，超时时间为60s。

<pre class="prettyprint lang-javascript">
> var rg = db.getRG("datagroup1") ;
> rg.reelect({Seconds:60});</pre>
##语法##
***rg.removeNode(&lt;host&gt;,&lt;service&gt;,[config])***

删除分区组中的指定节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
|-----|-----|-----|-----|
| host | string | 节点主机名。 | 是 |
| service | int/string | 节点端口号。 | 是 |
| config | Json 对象 | 节点配置信息。 | 否 |

## 格式##

rg.removeNode() 方法的定义格式有三个参数：host，service，config，如上表所示，格式如下：

<pre class="prettyprint lang-diy">
("<主机名>","<端口号>"[,{<configParam>:value,...}])</pre>


## 示例##

* 在分区组 group 中删除节点命令如下

<pre class="prettyprint lang-javascript">
> rg.removeNode("vmsvr2-suse-x64",11800)</pre>
##语法##
***rg.start()***

启动分区组。分区组启动之后才能创建节点及其他操作。也可以使用方法 db.startRG(< name >)) 启动指定的节点。

## 示例##

* 启动分区组命令：

<pre class="prettyprint lang-javascript">
> rg.start() //等价于 db.startRG("group")</pre>
##语法##
***rg.stop()***

停止分区组。停止之后就不能创建节点等相关操作。

## 示例##

* 停止分区组

<pre class="prettyprint lang-javascript">
> rg.stop()</pre>
##语法##
***node.connect()***

将数据库连接到指定节点。连接之后能进行一系列的操作，可以使用 node.connect().help() 查看相关的操作。

## 示例##

* 将数据库连接到节点名为 node 上

<pre class="prettyprint lang-javascript">
> node.connect()</pre>

返回：

<pre class="prettyprint lang-diy">        
vmsvr2-suse-x64:11800</pre>
##语法##
***node.getHostName()***

返回节点的主机名。

## 示例##

* 返回节点名称为 node 的主机名

<pre class="prettyprint lang-javascript">
> node.getHostName()</pre>

返回：
<pre class="prettyprint lang-diy">
vmsvr2-suse-x64</pre>
##语法##
***node.getNodeDetail()***

返回当前节点信息。

## 示例##

* 返回节点名称为 node 的信息

<pre class="prettyprint lang-javascript">
> node.getNodeDetail()</pre>

返回：

<pre class="prettyprint lang-diy">
1000:vmsvr2-suse-x64:11800(group)
其中"1000"为节点 ID（NodeID）；"vmsvr2-suse-x64"为主机名（HostName）；"11800"为服务器名（ServiceName），"（group）"为节点所在的分区组名。</pre>
## 语法##
***node.getServiceName()***

返回节点的服务器名。

## 示例##

* 返回节点名为 node 的服务器名

<pre class="prettyprint lang-javascript">
> node.getServiceName()</pre>

返回：

<pre class="prettyprint lang-diy">    
11800</pre>
##语法##
***node.start()***

启动当前节点。

## 示例##

* 启动当前名称为 node 的节点 。

<pre class="prettyprint lang-javascript">
> node.start()</pre>
## 语法##
***node.stop()***

停止当前节点。

## 示例##

* 停止当前名称为 node 的节点 。

<pre class="prettyprint lang-javascript">
> node.stop()</pre>
##语法##
***domain.alter(&lt;options&gt;)***

修改域的属性。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 需要修改的属性列表。 | 是 |

## 格式##

目前通过 options 可设置域的属性有：

| 属性名 | 描述 | 格式 |
| ------ | ------ | ------ |
| Groups | 包含的复制组。 | Groups:['data1','data2'] |
| AutoSplit | 自动切分。 | AutoSplit:true|false |

**Note:**

* 删除复制组前必须保证其不包含任何数据。
* AutoSplit 的更改不对之前创建的集合和集合空间产生影响。

## 示例##

* 首先创建一个域，包含两个复制组，开启自动切分。
	<pre class="prettyprint lang-javascript">
	> var domain = db.createDomain('mydomain',['data1','data2'],{AutoSplit:true})</pre>

	从域中删除一个复制组 data2，添加另一个复制组 data3，最后域中包含 data1 和 data3 两个复制组。
	<pre class="prettyprint lang-javascript">
	> domain.alter({Groups:['data1','data3']})</pre>

* 首先创建一个域，包含一个复制组，复制组中包含表 foo.bar。
	<pre class="prettyprint lang-javascript">
	> var domain = db.createDomain('mydomain',['group1'])</pre>

	从域中删除原复制组，添加另一个复制组，将因把拥有数据的 group1 从域中删除而报错。
	<pre class="prettyprint lang-javascript">
	> domain.alter({Groups:['group2']})
	(nofile):0 uncaught exception: -256
	> getErr(-256)
	Domain is not empty</pre>
##语法##
***domain.listCollections()***

枚举集合，执行此方法会将指定域下的集合信息全部显示出来。

## 示例##

<pre class="prettyprint lang-javascript">
> domain.listCollections()</pre>

返回：

<pre class="prettyprint lang-diy">
{"Name": "foo.bar"}</pre>
##语法##
***domain.listCollectionSpaces()***

枚举域中所有的集合空间。

## 示例##

<pre class="prettyprint lang-javascript">
> domain.listCollectionSpaces()</pre>

返回：

<pre class="prettyprint lang-diy">
{"Name":"foo"}</pre>
##语法##
***oma.close()***

关闭 oma 连接对象

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| - | - | - | - |

**Note:**

* oma对象为连接到目标（本地/远端机器）集群控制器（sdbcm）获得的连接对象。
* 关闭的 oma 连接对象必须存在，否则出现异常。

## 示例##

* 关闭 oma

<pre class="prettyprint lang-javascript">
var oma = new Oma("localhost", 11790);
oma.close();</pre>
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

* 在本地中创建一个本地端口号为11780，http端口为8000，web路径为/opt/sequoiadb/web的sdbom进程。

<pre class="prettyprint lang-javascript">
var oma = new Oma("localhost", 11790);
oma.createOM(11780,"/opt/sequoiadb/database/sms/11780",{httpname:8000,wwwpath:"/opt/sequoiadb/web"})</pre>
##语法##
***oma.removeCoord(< svcname >)***

在集群中删除一个 coord 节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |

**Note:**

* oma对象为连接到目标（本地/远端机器）集群控制器（sdbcm）获得的连接对象。
* 指定删除的节点必须存在，否则出现异常。

## 示例##

* 在集群的ubuntu1机器上删除一个端口号为11810的 coord 节点

<pre class="prettyprint lang-javascript">
var oma = new Oma("ubuntu1", 11790);
oma.removeCoord(11810)</pre>
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
##语法##
***oma.startNode(< svcname >)***

在目标集群控制器（sdbcm）所在的机器中启动一个节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |

**Note:**

* oma对象为连接到目标（本地/远端机器）集群控制器（sdbcm）获得的连接对象。
* 指定启动的节点必须存在，否则出现异常。

## 示例##

* 在本地启动一个端口号为11830的节点

<pre class="prettyprint lang-javascript">
var oma = new Oma("localhost", 11790);
oma.startNode(11830);</pre>
##语法##
***oma.stopNode(< svcname >)***

在目标集群控制器（sdbcm）所在的机器中停止一个节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |

**Note:**

* oma对象为连接到目标（本地/远端机器）集群控制器（sdbcm）获得的连接对象。
* 指定停止的节点必须存在，否则出现异常。

## 示例##

* 在本地停止一个端口号为11830的节点

<pre class="prettyprint lang-javascript">
var oma = new Oma("localhost", 11790);
oma.stopNode(11830);</pre>
##Json 格式##

***{ "$binary" : <数据>, "$type" : <类型> }***

##函数格式##

+--------------------------------------------------+----------------+
| 格式                                             | 描述           |
+==================================================+================+
| BinData(字符串: base64加密后的内容,字符串: 类型) | 指定二进制对象 |
+--------------------------------------------------+----------------+

##示例##

-   插入二进制类型记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({bin:{ "$binary" : "aGVsbG8gd29ybGQ=", "$type" : "1" }})
> db.foo.bar.insert({bin:BinData("aGVsbG8gd29ybGQ=", "1" )})</pre>##Json 格式##

***{"$maxKey":1 }***

##函数格式##

+----------+-------------+
| 格式     | 描述        |
+==========+=============+
| MaxKey() | MaxKey 对象 |
+----------+-------------+
##Json 格式##

***{"$minKey":1 }***

##函数格式##

+----------+-------------+
| 格式     | 描述        |
+==========+=============+
| MinKey() | MinKey 对象 |
+----------+-------------+

##示例##

-   插入一个 MinKey 对象

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({key:MinKey()})</pre>##函数格式##

+------------------------+------------------------+
| 格式                   | 描述                   |
+========================+========================+
| NumberLong(数字:值)    | 使用数字指定64位整数   |
+------------------------+------------------------+
| NumberLong(字符串:值)  | 使用字符串指定64位整数 |
+------------------------+------------------------+

**Note:**

NumerLong 中数字类型的参数类型为 double。当操作超过精度的值时使用字符串类型的参数。

##示例##

-   插入一个长整型

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({number:NumberLong(100)})
> db.foo.bar.insert({number:NumberLong("100")})</pre>##Json 格式##

***{ "$oid" : &lt;24字节16进制字符串&gt; }***

##函数格式##

+------------------------------+----------------------+
| 格式                         | 描述                 |
+==============================+======================+
| ObjectId()                   | 生成一个 OID         |
+------------------------------+----------------------+
| ObjectId(24字节16进制字符串) | 生成一个指定值的 OID |
+------------------------------+----------------------+

##示例##

-   按照 OID 查询

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({"_id":{"$oid": "55713f7953e6769804000001"}})
> db.foo.bar.find({_id:ObjectId("55713f7953e6769804000001")})</pre>

-   按照 OID 查询

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({id:ObjectId()})</pre>##Json 格式##

***{ "$regex" : <正则表达式>, "&#36;options" : <类型> }***

##函数格式##

+-------------------------------------+----------------+
| 格式                                | 描述           |
+=====================================+================+
| Regex(字符串: 表达式, 字符串: 类型) | 指定正则表达式 |
+-------------------------------------+----------------+

##示例##

-   使用正则匹配

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({reg: { "key" : { "$regex" : "^W", "$options" : "i" } }});
> db.foo.bar.find({reg:Regex("^W", "i")})</pre>##Json 格式##

***{ "$date" : &lt;YYYY-MM-DD&gt; }***

##函数格式##

+----------------------------+----------+
| 格式                       | 描述     |
+============================+==========+
| SdbDate()                  | 当前日期 |
+----------------------------+----------+
| SdbDate(字符串:YYYY-MM-DD) | 指定日期 |
+----------------------------+----------+

##示例##

-   插入一个日志类型的记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({date:{$date:"2015-03-13"}})
> db.foo.bar.insert({date:SdbDate("2015-03-13")})</pre>##Json 格式##

***{ "$timestamp" : &lt;YYYY-MM-DD-HH.mm.ss.ffffff&gt; }***

##函数格式##

+----------------------------------------------+------------------------+
| 格式                                         | 描述                   |
+==============================================+========================+
| Timestamp()                                  | 当前时间戳             |
+----------------------------------------------+------------------------+
| Timestamp(字符串:YYYY-MM-DD-HH.mm.ss.ffffff) | 指定时间的时间戳       |
+----------------------------------------------+------------------------+
| Timestamp(数字:秒数, 数字:微秒)              | 使用绝对秒数指定时间戳 |
+----------------------------------------------+------------------------+

##示例##

-   插入一个时间戳类型的记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({time: {"$timestamp": "2015-06-05-16.10.33.000000"}})
> db.foo.bar.insert({time:Timestamp("2015-06-05-16.10.33.000000")})
> db.foo.bar.insert({time:Timestamp(1433492413, 0)})</pre>## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$all:[<值1>,<值2>,...<值N>]}}</pre>

## 描述##

$all 的操作对象是数组类型的字段名，选择“<字段名>”包含所有给定数组（[<值1>,<值2>,...<值N>]）中的值。

## 示例##

* 选择集合 bar 下 name 字段的值包含“Tom”和“Mike”的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({name:{$all:["Tom","Mike"]}})</pre>

因此，上面的语句会匹配集合 bar 中有 name 字段，且值形如下面数组的记录：

<pre class="prettyprint lang-diy">
["Tom","Mike",..]
["Tom","Jhon","Mike",...]</pre>

但是不会匹配集合 bar 下 name 字段值形如下面数组的记录

<pre class="prettyprint lang-diy">
["Tom","Jhon"]</pre>

**Note:**

使用 $all 操作一个非数组类型的字段的话，例如：

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$all:[20]}}) 它等价于 db.foo.bar.find({age:20})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{$and:[{<表达式1>}，{<表达式2>},...,{<表达式N>}]}</pre>

## 描述##

$and 是一个逻辑“与”操作。它的作用是选择满足所有表达式（<表达式1>，<表达式2>,...,<表达式N>）的记录，但是如果第一个表达式（<表达式1>）的计算结果为 false，SequoiaDB 将不会再执行后面的表达式。

## 示例##

* 选择集合 bar 下 age 字段值为20，price 字段值小于10的记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({$and:[{age:20},{price:{$lt:10}}]})</pre>

**Note:**

SequoiaDB 提供了一种隐式的 and 操作，用逗号（,）隔开个表达式，例如

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:20,price:{$lt:10}})</pre>

当使用 and 操作对同一个字段名时，如{age：{$lt:20}}and{age:{$exists:1}}。那么可以用 $and 操作两个分开的表达式，也可以合并这两个表达式{age:{$lt:20,$exists:1}}。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$inc:{salary:200}},{$and:[{age:{$lt:20}},{age:{$exists:1}}]})
> db.foo.bar.update({$inc:{salary:200}},{age:{$lt:20,$exists:1}})</pre>

两个操作的结果相同，首先查询集合 bar 下存在 age 字段并且 age 的值小于20的记录，然后对这些记录的 salary 字段的值增加200。
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$elemMatch:{子字段名:<值>,...}}}</pre>

## 描述##

选择集合中“<字段名>”匹配指定“{子字段名:<值>,....}”的记录。

## 示例##

* 嵌套 JSON 对象匹配

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({content:{$elemMatch:{name:"Tom",phone:123}}})</pre>

字段 content 是一个 JSON 嵌套对象，此操作匹配 content 内字段 name 值为“Tom”，phone 值为123的记录。
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$et:<值>}}</pre>

## 描述##

$et 选择满足“字段名”的值等于（=）指定“值”的记录。

## 示例

* 返回集合 bar 中 age 字段值等于 20 的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$et:20}}) 等价于 db.foo.bar.find({age:20})</pre>

* $et 匹配嵌套对象中的字段名。使用 update() 方法更新嵌套对象 service 中的 type 字段值等于15的记录，将这些记录的 age 字段值设定为25。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{age:25}},{"service.type":{$et:15}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$exists:<0|1>}}</pre>

## 描述##

选择集合中是否存在指定“<字段名>”的记录。“0”表示选择不存在指定“<字段名>”的记录，“1”表示选择存在指定“<字段名>”的记录。

## 示例##

* 选择集合 bar 中存在字段 age 的记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$exists:1}})</pre>

* 选择集合 bar 中嵌套对象 content 不存在 name 字段的记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({"content.name":{$exists:0}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{ <字段名1>: { $field: <字段名2> }, ...}

或者

{ <字段名1>: { <匹配符>: { $field: <字段名2> } }, ...}</pre>

## 描述##

$field 是字段符，选择满足“字段名1”匹配“字段名2”的记录。

## 示例##

* 返回集合 bar 中 t1 字段值等于 t2 字段的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find( { t1: { $field: "t2" } } )</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "521d5446e2d3c4e31c000001"
  },
  "t1": 100
  "t2": 100
}
...</pre>

* 返回集合 bar 中 t1 字段值大于 t2 字段的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find( { t1: { $gt: { $field: "t2" } } } )</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "521d5446e2d3c4e31c000000"
  },
  "t1": 3
  "t2": 0
}
...</pre>## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$gt:<值>}}</pre>

## 描述##

$gt 选择满足“字段名”的值大于（>）指定“值”的记录。

## 示例##

* 返回集合 bar 中 age 字段值大于20的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gt:20}})</pre>

* $gt 匹配嵌套对象中的字段名。使用 update() 方法更新嵌套对象 service 中的 ID 字段值大于2的记录，将这些记录的 age 字段值设定为25。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{age:25}},{"service.ID":{$gt:2}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$gte:<值>}}</pre>

## 描述##

$gte 选择满足“字段名”的值大于等于（>=）指定“值”的记录。

## 示例##

* 选择查询集合空间 foo 下集合 bar 中字段名为 age 的值大于等于20的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$gte:20}})</pre>

* $gte 匹配嵌套对象中的字段名。使用 update() 方法更新嵌套对象 service 中的 ID 字段值大于等于2的记录，将这些记录的 age 字段值设定为25。
	
<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{age:25}},{"service.ID":{$gte:2}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{"字段名.$+标识符":value}</pre>


## 描述##

$+标识符是一种特殊的命令符，这种命令符只作用于数组对象，标识符是一个整数，如 $1，$3，标识符相当于一个临时的存储，会把匹配成功的数组元素的索引存储起来。下面这些是错误的书写格式：$5.4，$a2，$3c，$MA。

这种命令符只作用于数组，用来代替数组的索引 Key，并且可以把匹配到的第一个索引值传递到方法 update 的 rule 参数中。

## 示例##

* 查询

有记录：{a:[1,2,3,4,5]};{a:[1,4,5]};{a:[4,2,1]}现在要查询出数组中存在元素5的记录，使用如下命令

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({"a.$1":5},{a:1})</pre>

只要记录中数组对象 a 存在元素5，就能返回。返回结果如下：

<pre class="prettyprint lang-diy">
{ "a": [ 1, 4, 5 ] }
{ "a": [ 1, 2, 3, 4, 5 ] }</pre>

* 更新

（1） 有记录 { a : [ 1, 2, 3, 4, 5 ] }，现在要修改数组 a 中的元素，把值为4的元素改成100，使用如下命令

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{"a.$1":100}},{"a.$1":4})</pre>

在匹配时元素4的索引 Key 是3，因此在更新规则 { "$set" : { "a.$1": 100 } } 中，$1的值为3，系统会自动把更新规则转换成 { "$set" : { "a.3" : 100 } }

更新后记录为：

<pre class="prettyprint lang-diy">
{ a : [ 1, 2, 3, 100, 5 ] }</pre>

（2） 有记录 { a : [ 1, 2, 3, 4, 5 ], b : [ 6, 7, 8 ] }，现要修改数组 a 中的元素，把值为4的元素改成100，且把数组 b 中值为6的元素修改为200，使用如下命令

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({ "$set" : { "a.$1" : 100, "b.$2" : 200 } },{ "a.$1": 4, "b.$2" : 6 })</pre>

更新后记录为：

<pre class="prettyprint lang-diy">
{ a : [ 1, 2, 3, 100, 5 ], b : [ 200, 7, 8 ] }</pre>

**Note:**

如果有多个元素符合规则，那么只会修改第一个。如下例：

（3） 有记录 {  a : [ 1, 2, 2, 2, 5 ] }，现要修改数组 a 中的元素，把值为2的元素改成100，使用如下命令

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({ "$set" : { "a.$1" : 100 } },{ "a.$1": 2 })</pre>

更新后记录为：

<pre class="prettyprint lang-diy">
{ a : [ 1, 100, 2, 2, 5 ]  }</pre>
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$in:[<值1>,<值2>,...<值N>]}}</pre>

## 描述##

选择集合中“<字段名>”值匹配给定数组（[<值1>,<值2>,...<值N>]）中任意一个值的记录；如果“<字段名>”本身是数组类型，那么只要满足“<字段名>”中任意一个值等于给定数组（[<值1>,<值2>,...<值N>]）中值的记录都会返回。

## 示例##

* 选择集合 bar 下 age 字段的值是20或25的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$in:[20,25]}})</pre>

* $in 匹配嵌套数组对象中的元素。选择集合 bar 中数组对象 name 存在元素“Tom”或“Mike”的记录，并将这些记录的 age 字段删除。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{age:""}},{name:{$in:["Tom","Mike"]}})</pre>

**Note:**

当给定数组只有一个值时，即{<字段名>:{$in:[<值>]}}，等价于{<字段名>:<值>}

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$in:[20]}})等价于db.foo.bar.find({age:20})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$isnull: &lt;0|1&gt;}}</pre>

## 描述##

选择集合中指定的“<字段名>”是否为空，或不存在。“0”代表期望该字段存在且不为 null；“1”代表期望该字段不存在或为 null。

## 示例##

* 选择集合 bar 中 age 字段不为空且存在的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$isnull:0}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$lt:<值>}}</pre>

## 描述##

$lt 选择满足“字段名”的值小于（<）指定“值”的记录。

## 示例##

* 查询集合 bar 中字段名为 age，其值小于20的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$lt:20}})</pre>

* $lt 匹配嵌套对象中的字段名。使用 update() 方法更新嵌套对象 service 中的 ID 字段值小于2的记录，将这些记录的 age 字段值设定为25。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{age:25}},{"service.ID":{$lt:15}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$lte:<值>}}</pre>

## 描述##

$lte 选择满足“字段名”的值小于等于（<=）指定“值”的记录。

## 示例##

* 选择查询集合空间 foo 下集合 bar 中字段名为 age 的值小于等于20的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$lte:20}})</pre>

* $lte 匹配一个嵌套对象中的字段名。使用 update() 方法更新嵌套对象 service 中的 ID 字段值小于等于2的记录，将这些记录的 age 字段值设定为25。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{age:25}},{"service.ID":{$lte:2}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$mod:[value1,value2]},...}</pre>

## 描述##

$mod 是取模匹配符，返回指定字段名的值对 value1 取模的值等于 value2 的记录。

**Note:**

* 参数 value1 是除0以外的整型数；如果是浮点型，那只会截取整数部分；不能为其他基础类型。
* 参数 value2 是整型数；如果是浮点型，也只截取整数部分；其他类型以0处理。

## 示例##

* 返回集合 bar 中 age 字段值对5取模后的值等于3的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$mod:[5,3]}})</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "521d5446e2d3c4e31c000000"
  },
  "age": 3
}
...</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$mod:[2.3,1.5]}})</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "521d5446e2d3c4e31c000000"
  },
  "age": 3
}
{
  "_id": {
    "$oid": "521d544ee2d3c4e31c000002"
  },
  "age": 5
}</pre>

对数组[2.3,1.5]中的两个元素只截取了整数部分。
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>：{$ne:<值>}}</pre>

## 描述##

$ne 选择满足“字段名”的值不等于（!=）指定“值”的记录。

## 示例##

* 返回集合 bar 中 age 字段值不等于 20 的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$ne:20}})</pre>

* $ne 匹配嵌套对象中的字段名。使用 update() 方法更新嵌套对象 service 中的 type 字段值不等于15的记录，将这些记录的 age 字段值设定为25。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{age:25}},{"service.type":{$ne:15}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$nin:[<值1>,<值2>,...<值N>]}}</pre>

## 描述##

选择集合中“<字段名>”值不等于给定数组（[<值1>,<值2>,...<值N>]）中任意一个值的记录或者不存在给定字段名的记录；如果“<字段名>”本身是数组类型，那么选择“<字段名>”中任意一个值都不等于给定数组（[<值1>,<值2>,...<值N>]）中值的记录。

## 示例##

* 选择集合 bar 下 age 字段的值不等于20和25或集合 bar 下不存在 age 字段的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$nin:[20,25]}})</pre>

* $nin 匹配数组对象中的元素。选择集合 bar 中存在数组对象 name 且其元素不包含“Tom”和“Mike”或者选择集合 bar 中不存在数组对象 name 的记录，并将这些记录的 age 字段删除。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{age:""}},{name:{$nin:["Tom","Mike"]}})</pre>

**Note:**

当给定数组只有一个值时，即{<字段名>:{$nin:[<值>]}}，等价于{<字段名>:{$ne:<值>}}

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$nin:[20]}})等价于db.foo.bar.find({age:{$ne:20}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{$not:[{<表达式1>},{<表达式2>},...,{<表达式N>}]}</pre>

## 描述##

$not 是一个逻辑“非”操作。它的作用是选择不匹配表达式（<表达式1><表达式2>,...,<表达式N>）的记录。只要不满足其中的任意一个表达式，记录就会返回。

## 示例##

* 选择集合 bar 下 age 字段值不等于20或 price 字段值不小于10的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({$not:[{age:20},{price:{$lt:10}}]})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{$or:[{<表达式1>},{<表达式2>},...,{<表达式N>}]}</pre>

## 描述##

$or 是一个逻辑“或”操作。它的作用是选择满足表达式（<表达式1>,<表达式2>,...,<表达式N>）其中一个表达式的记录。只要有一个表达式的计算结果为 true，记录就会返回。

## 示例##

* 选择集合 bar 下 name 字段值为“Tom”，且 age 字段值为20或 price 字段值小于10的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({name:"Tom",$or:[{age:20},{price:{$lt:10}}]})</pre>

* \$or 匹配嵌套对象中的字段名。选择 age 字段值小于20或者嵌套对象 snapshot 中的 type 字段值为“system”的记录，并使用 $inc 更新这些记录的 salary 字段值。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$inc:{salary:200}},{$or:[{age:{$lt:20}},{"snapshot.type":"system"}]})</pre>
## 描述##

$regex 操作提供正则表达式模式匹配字符串查询功能。SequoiaDB 使用的是 PCRE 正则表达式。

**Note:**

$regex 与 $options 配套使用。

### $options##

$options 提供四种选择标志：

-    **i：** 设置这个修饰符，模式中的字母进行大小写不敏感匹配。

-    **m：** 默认情况下，pcre认为目标字符串是由单行字符组成的，“行首”元字符（^）仅匹配字符串的开始位置，而“行末”元字符（$）仅匹配字符串末尾，或者最后的换行符。当这个修饰符设置之后，“行首”和“行末”就会匹配目标字符串中任意换行符之前或之后，另外，还分别匹配目标字符串的最开始和最末尾位置，如果目标字符串中没有“\n”，或者模式中没出现“^”或“$”，设置这个修饰符不产生任何影响。

-    **x：** 设置这个修饰符，模式中没有经过转义的或不在字符类中的空白数据字符总会被忽略，并且位于一个未转义的字符类外部的#字符和下一行换行符之间的字符也被忽略。

-    **s：** 设置这个修饰符，模式中的点号元字符匹配所有字符，包含换行符，如果没有这个修饰符，点号不匹配换行符。

## 示例##

* 返回集合 bar 下 str 字段值匹配不区分大小写的正则表达式 dh.*fj 的记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({str:{$regex:'dh.*fj',$options:'i'}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{"<字段名>":{$size:"<值>"}}</pre>

## 描述##

$size 的操作对象为数组型字段，匹配数组长度为指定“<值>”的记录。

## 示例##

* 返回集合 bar 中数组类型字段 arr 的长度为2的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({arr:{$size:2}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{<字段名>:{$type:&lt;BSON type&gt;}}</pre>

## 描述##

选择集合中的“<字段名>”值的类型等于指定“&lt;BSON type&gt;”的值。

## BSON Type##

| Type | 描述 | 值 |
| ------ | ------ | ------ |
| 32-bit integer | 整型，范围-2147483648至2147483647 | 16 |
| 64-bit integer | 长整型，范围-9223372036854775808至9223372036854775807。如果用户指定的数值无法适用于整数，则 SequoiaDB 自动将其转化为长整数。 | 18 |
| double | 浮点数，范围1.7E-308至1.7E+308 | 1 |
| string | 字符串 | 2 |
| ObjectID | 十二字节对象 ID | 7 |
| boolean | 布尔（true \| false） | 8 |
| date | 日期（YYYY-MM-DD） | 9 |
| timestamp | 时间戳（YYYY-MM-DD-HH.mm.ss.ffffff） | 17 |
| Binary data | Base64 形式的二进制数据 | 5 |
| Regular expression | 正则表达式 | 11 |
| Object | 嵌套 JSON 文档对象 | 3 |
| Array | 嵌套数组对象 | 4 |
| null | 空 | 10 |

## 示例##

* 选择集合 bar 下 age 字段是整型的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({age:{$type:16}})</pre>

* 选择集合 bar 下嵌套对象 content 中的 arr 字段是数组类型的记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({"content.arr":{$type:4}})</pre>
##说明##

返回数字的绝对值，非数字类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$abs:1}})</pre>

**Note:**

{$abs:1}中1没有特殊含义，仅作为占位符出现。##说明##

返回字段值加上某个数值的结果。非数字类型返回 null

##示例##

-   返回a字段加上10的结果

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$add:10}})</pre>##说明##

将字段转化为指定类型。其中指定的类型可以使用字符串表示(大小写不敏感)，也可以使用数字表示，如下表：

+-----------+-------------+----------+
| 目标类型  | 字符串表示  | 数字表示 |
+===========+=============+==========+
| MinKey    | "minkey"    | -1       |
+-----------+-------------+----------+
| Double    | "double"    | 1        |
+-----------+-------------+----------+
| String    | "string"    | 2        |
+-----------+-------------+----------+
| Object    | "object"    | 3        |
+-----------+-------------+----------+
| ObjectId  | "oid"       | 7        |
+-----------+-------------+----------+
| Bool      | "bool"      | 8        |
+-----------+-------------+----------+
| Date      | "date"      | 9        |
+-----------+-------------+----------+
| Null      | "null"      | 10       |
+-----------+-------------+----------+
| Int32     | "int32"     | 16       |
+-----------+-------------+----------+
| Timestamp | "timestamp" | 17       |
+-----------+-------------+----------+
| Int64     | "int64"     | 18       |
+-----------+-------------+----------+
| MaxKey    | "maxkey"    | 127      |
+-----------+-------------+----------+

转换关系如下：

+-----------+-----------------+---------------------------------------+----------+
| 目标类型  | 源类型          | 转换格式备注                          | 异常返回 |
+===========+=================+=======================================+==========+
| MinKey    | ALL             | --                                    | --       |
+-----------+-----------------+---------------------------------------+----------+
| Double    | <br>            | <br>                                  |          |
|           | String<br>      | 数字字符串:对应的数值，其他:0.0<br>   | 0.0      |
|           | Bool<br>        | true:1.0，false:0.0<br>               |          |
|           | Int32<br>       | <br>                                  |          |
|           | Int64<br>       | <br>                                  |          |
+-----------+-----------------+---------------------------------------+----------+
| String    | <br>            |                                       |          |
|           | Int32<br>       |                                       | null     |
|           | Int64<br>       |                                       |          |
|           | Double<br>      |                                       |          |
|           | Date<br>        |                                       |          |
|           | Timestamp<br>   |                                       |          |
|           | Oid<br>         |                                       |          |
|           | Object<br>      |                                       |          |
|           | Array<br>       |                                       |          |
|           | Bool<br>        |                                       |          |
+-----------+-----------------+---------------------------------------+----------+
| Object    | String          | 标准Json                              | null     |
+-----------+-----------------+---------------------------------------+----------+
| ObjectId  | String          | 24个字符                              | null     |
+-----------+-----------------+---------------------------------------+----------+
| Bool      | <br>            | <br>                                  |          |
|           | Int32<br>       | 0:false，其他:true<br>                | --       |
|           | Int64<br>       | 0:false，其他:true<br>                |          |
|           | Double<br>      | 0:false，其他:true<br>                |          |
+-----------+-----------------+---------------------------------------+----------+
| Date      | <br>            | <br>                                  |          |
|           | Int64<br>       | 绝对毫秒<br>                          | null     |
|           | Double<br>      | 绝对毫秒<br>                          |          |
|           | Stirng<br>      | 形如 "2015-08-19"<br>                 |          |
|           | Timestamp<br>   | <br>                                  |          |
|           | Int32<br>       | 绝对毫秒<br>                          |          |
+-----------+-----------------+---------------------------------------+----------+
| Null      | ALL             | --                                    | --       |
+-----------+-----------------+---------------------------------------+----------+
| Int32     | <br>            | <br>                                  |          |
|           | String<br>      | <br>                                  | 0        |
|           | Bool<br>        | <br>                                  |          |
|           | Int64<br>       | <br>                                  |          |
|           | Double<br>      | <br>                                  |          |
|           | Timestamp<br>   | 绝对毫秒<br>                          |          |
|           | Date<br>        | 绝对毫秒<br>                          |          |
+-----------+-----------------+---------------------------------------+----------+
| Timestamp | <br>            | <br>                                  |          |
|           | Int32<br>       | 绝对毫秒<br>                          | null     |
|           | Int64<br>       | 绝对毫秒<br>                          |          |
|           | Double<br>      | 绝对毫秒<br>                          |          |
|           | Stirng<br>      | 形如 "2015-08-19-17.59.05.918488"<br> |          |
|           | Date<br>        | <br>                                  |          |
+-----------+-----------------+---------------------------------------+----------+
| Int64     | <br>            | <br>                                  |          |
|           | Int32<br>       | <br>                                  | 0        |
|           | Double<br>      | <br>                                  |          |
|           | Stirng<br>      | <br>                                  |          |
|           | Timestamp<br>   | 绝对毫秒<br>                          |          |
|           | Date<br>        | 绝对毫秒<br>                          |          |
+-----------+-----------------+---------------------------------------+----------+
| MaxKey    | ALL             | --                                    | --       |
+-----------+-----------------+---------------------------------------+----------+

**Note:**

时间类型字段转换为Int32类型可能会发生溢出。

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.insert({a:"123"})
> db.foo.bar.find({}, {a:{$cast:"int32"}})</pre>

返回：

<pre class="prettyprint lang-diy">
{a:123}</pre>##说明##

返回大于目标字段值的最小整数值，非数字类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$ceiling:1}})</pre>

**Note:**

{$ceiling:1}中1没有特殊含义，仅作为占位符出现。##说明##

选择某个字段。当字段不存在时返回默认值。可简写为{&lt;fieldName&gt;:&lt;defaultValue&gt;}。

##示例##

为方便理解，给出原始数据样例：

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 }
  ]
}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:[], teacher:{$default:"Mr Liu"}})</pre>

返回

<pre class="prettyprint lang-diy">
{
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 },
  ],
  teacher:"Mr Liu"
}</pre>##说明##

返回字段值除以某个数值的结果。非数字类型返回 null

##示例##

返回a字段除以10的结果

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$divide:10}})</pre>

**Note:**

除数不能为0。##说明##

返回数组内满足条件的元素的集合

##示例##

为方便理解，给出原始数据样例：

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 }
  ]
}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$elemMatch:{age:18}}})</pre>

返回

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"WangErmazi", age:18 },
  ]
}</pre>##说明##

返回数组内满足条件的第一个元素的集合

##示例##

为方便理解，给出原始数据样例：

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 }
  ]
}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$elemMatchOne:{age:18}}})</pre>

返回

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 }
  ]
}</pre>##说明##

返回小于目标字段值的最大整数值，非数字类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$floor:1}})</pre>

**Note:**

{$floor:1}中1没有特殊含义，仅作为占位符出现。##说明##

选择(!=0)或移除(=0)某个字段

##示例##

为方便理解，给出原始数据样例：

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 }
  ]
}</pre>

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1},{students:{$include:1}})</pre>

返回 student 字段

<pre class="prettyprint lang-diy">
{
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 },
  ]
}</pre>

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1},{_id:{$include:0}})</pre>

返回移除 _id 字段

<pre class="prettyprint lang-diy">
{
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 },
  ]
}</pre>##说明##

返回字符串中字符改变为小写的结果。非字符串类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$lower:1}})</pre>

**Note:**

{a:{$lower:1}}中的1作为占位符出现。##说明##

去掉字符串左侧开头的空格(或制表符)。非字符串类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$ltrim:1}})</pre>

**Note:**

{a:{$ ltrim:1}}中的1作为占位符出现。##说明##

返回取模的结果，非数字类型返回 null。

##示例##

-   返回字段a对10取模的结果

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$mod:10}})</pre>

**Note:**

不能对零取模。##说明##

返回字段值与某个数值相乘的结果。非数字类型返回 null

##示例##

-   返回a字段乘以10的结果

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$multiply:10}})</pre>##说明##

去掉字符串右侧开头的空格(及制表符)。非字符串类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$rtrim:1}})</pre>

**Note:**

 {a:{$ rtrim:1}}中的1作为占位符出现。##说明##

返回数组的切片

##示例##

为方便理解，给出原始数据样例：

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 },
    {name:"LiSi", age:19 },
    {name:"WangErmazi", age:18 }
  ]
}</pre>

-    <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$slice:1}})</pre>

返回下标为0，长度为1的切片

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 }
  ]
}</pre>

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$slice:2}})</pre>

返回下标为0，长度为2的切片

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"ZhangSan", age:18 }，
    {name:"LiSi", age:19 }
  ]
}</pre>

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$slice:-2}})</pre>

返回倒数前2个元素的切片

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"LiSi", age:19 }，
    {name:"WangErmazi", age:18 }
  ]
}</pre>

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$slice:[1,2]}})</pre>

返回下标为1，长度为2的切片

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"LiSi", age:19 }，
    {name:"WangErmazi", age:18 }
  ]
}</pre>

-   <pre class="prettyprint lang-javascript">
> db.foo.bar.find({class:1}, {students:{$slice:[-1,2]}})</pre>

返回倒数第1个开始，长度为2的切片（长度不足，只返回1个元素）

<pre class="prettyprint lang-diy">
{
  _id:1,
  class:1,
  students:
  [
    {name:"WangErmazi", age:18 }
  ]
}</pre>##说明##

返回字符串长度，不包括终止符。非字符串返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$strlen:1})</pre>

**Note:**

{$strlen:1}中1没有特殊含义，仅作为占位符出现。##说明##

返回字符串的子串，非字符串类型返回 null

##示例##

记录；**{a:"abcdefg"}**

-   返回下标为0，长度为2的子串

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$substr:2}})</pre>

返回：**{a:" ab"}**

-   返回倒数第二个字符开始的子串

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$substr:-2}})</pre>

返回：**{a:"fg"}**

-   返回下标为0，长度为3的子串

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$substr:[2,3]}})</pre>

返回：**{a:"cde"}**

-   返回倒数第二个字符开始，长度为3的子串

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$substr:[-2, 3]}})</pre>

返回：**{a:"fg"}**(长度不足3)##说明##

返回字段值减去某个数值的结果。非数字类型返回 null

##示例##

-   返回a字段减去10的结果

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({}, {a:{$subtract:10}})</pre>##说明##

去掉字符串两侧开头的空格(及制表符)。非字符串类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$trim:1}})</pre>

**Note:**

{a:{$ trim:1}}中的1作为占位符出现。##说明##

返回字符串中字符改变为大写的结果。非字符串类型返回 null

##示例##

<pre class="prettyprint lang-javascript">
> db.foo.bar.find({},{a:{$upper:1}})</pre>

**Note:**

{a:{$ upper:1}}中的1作为占位符出现。##语法##

<pre class="prettyprint lang-diy">
{$addtoset:{<字段名1>:[<值1>,<值2>,...,<值N>]，<字段名2>:[<值1>,<值2>,...,<值N>],...}}</pre>

## 描述##

\$addtoset 是向数组对象中添加元素和值，操作对象必须为数组类型的字段。$addtoset 有如下规则：

* 记录中有指定的字段名（<字段名1>,<字段名2>,...）。

如果指定的值（[<值1>,<值2>,...,<值N>]）在记录中存在，跳过不做任何操作，只向目标数组对象中添加不存在的值。

* 记录中不存在指定的字段名。

如果记录本身不存在指定的字段名（<字段名1>,<字段名2>,...），那么将指定的字段名和值更新到记录中。

## 示例##

* 记录中存在目标数组对象。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,4],age:10,name:"Tom"}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$addtoset:{arr:[1,3,5]}},{arr:{$exists:1}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,2,4,3,5],age:10,name:"Tom"}</pre>

将原记录 arr 数组没有的元素3和5，使用 $addtoset 之后更新到 arr 数组内。

* 记录中不存在指定的数组对象，如有记录：

<pre class="prettyprint lang-diy">
{name:"Mike",age:12}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$addtoset:{arr:[1,3,5]}},{arr:{$exists:0}})</pre>

此操作后，记录更新为：
<pre class="prettyprint lang-diy">
{arr:[1,3,5],age:12,name:"Mike"}</pre>

原记录中没有数组对象 arr 字段，$addtoset 操作将 arr 字段和值更新到记录中。
## 语法##

<pre class="prettyprint lang-diy">
{$inc:{<字段名1>:<值1>,<字段名2>:<值2>,...}}</pre>

## 描述##

$inc 操作是给指定“<字段名>”增加指定的“<值>”。如果原记录中没有指定的字段名，那将字段名和值填充到记录中；如果原记录中存在指定的字段名，那么将字段名的值加上指定的值。

## 示例##

* 选择集合 bar 下 age 字段值大于15的记录，然后更新这些记录，将 age 字段的值增加5、ID 的值添加1。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$inc:{age:5,ID:1}},{age:{$gt:15}})</pre>

* 选择集合 bar 中存在数组对象 arr 的记录，将数组对象 arr 的第二个元素值添加1。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$inc:{"arr.1":1}},{arr:{$exists:1}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{$pop:{<字段名1>:<N>,<字段名2>:<N>,...}}</pre>

## 描述##

$pop 操作是删除指定数组对象（<字段名1>,<字段名2>,...）最后 N 个元素，操作对象必须为数组类型的字段。如果记录中不存在指定的数组对象，跳过不做任何操作；如果指定的 N 值大于数组对象的长度，数组对象的长度更新为0，即它的元素全部被删除；如果指定的 N 值 < 0，意味着从数组起始删除第 -N 个元素。

## 示例##

* 删除集合 bar 下数组对象 arr 的最后两个元素。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,3,4],age:20,name:"Tom"}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pop:{arr:2}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,2],age:20,name:"Tom"}</pre>

* 删除集合 bar 下数组对象 arr 的最后10个元素。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,3,4],age:20,name:"Tom"}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pop:{arr:10}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[],age:20,name:"Tom"}</pre>

* 删除集合 bar 下数组对象 arr 的前两个元素，即设置N的值为-2。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,3,4],age:20,name:"Tom"}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pop:{arr:-2}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[3,4],age:20,name:"Tom"}</pre>
## 语法##

<pre class="prettyprint lang-diy">
{$pull:{<字段名1>:<值1>,<字段名2>:<值2>,...}}</pre>

## 描述##

$pull 清除指定数组对象（<字段名1>,<字段名2>,...）的指定值（<值1>,<值2>,...）。操作对象必须为数组类型的字段。如果记录中不存在指定的数组对象，跳过不做任何操作；如果指定的值不存在数组对象中，也不做任何操作。

## 示例##

* 清除集合 bar 下数组对象 arr 值为2的元素以及数组对象 name 中元素值为“Tom”的元素。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,4,5],age:10,name:["Tom","Mike"]}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pull:{arr:2,name:"Tom"}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,4,5],age:10,name:["Mike"]}</pre>

* 清除集合 bar 下数组对象 arr 中元素值等于2的元素以及数组对象 name 中元素值为“Tom”的元素。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,3,4,5],age:10,name:["Tom","Mike"]}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pull:{arr:2,name:"Tom"}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,3,4,5],age:10,name:["Mike"]}</pre>

由于 arr 数组对象没有元素值为2的元素，因此对 arr 对象不做任何操作。
## 语法##

<pre class="prettyprint lang-diy">
{$pull_all:{<字段名1>:[<值1>,<值2>,...,<值N>],<字段名2>:[<值1>,<值2>,...,<值N>],...}}</pre>

## 描述##

$pull_all 清除指定数组对象（如<字段名1>）的指定值（[<值1>,<值2>,...,<值N>]）。操作对象必须为数组类型的字段。如果记录中不存在指定的数组对象，跳过不做任何操作；如果指定的值不存在数组对象中，也不做任何操作。

## 示例##

* 清除集合 bar 中数组对象 arr 中值为2和3的元素以及数组对象 name 中元素值为“Tom”的元素。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,4,5],age:10,name:["Tom","Mike"]}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pull_all:{arr:[2,3],name:["Tom"]}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,4,5],age:10,name:["Mike"]}</pre>

* 删除集合 bar 中数组对象 arr 里面的元素值为4和5的元素。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,3,4,5],age:10,name:["Tom","Mike"]}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$pull_all:{arr:[4,5]}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,3],age:10,name:["Tom","Mike"]}</pre>
## 语法##

<pre class="prettyprint lang-diy">
{$push:{<字段名1>:<值1>,<字段名2>:<值2>,...}}</pre>

## 描述##

$push 将给定数值（<值1>）插入到目标数组（<字段名1>）中，操作对象必须为数组类型的字段。如果记录中不存在指定的字段名，将指定的字段名以数组对象的形式推入到记录中并填充其指定的数值；如果记录中存在指定的字段名，且字段名存在指定的数值，指定的数值也会被推入到记录中。

## 示例##

* 向集合 bar 下的 arr 数组对象推入数值1。原记录中 arr 数组对象存在元素1，如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,4],age:10,name:["Tom","Mike"]}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$push:{arr:1}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,2,4,1],age:10,name:["Tom","Mike"]}</pre>

虽然原来 arr 中有元素1，使用 $push 操作符，还是会将元素1推入到 arr 数组对象中。

* 向集合 bar 中推入不存在的数组对象和值。原记录中不存在数组对象 name，如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2],age:20}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$push:{name:"Tom"}},{name:{$exists:0}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,2],age:20,name:["Tom"]}</pre>

原记录中不存在数组对象 name，使用 $push 操作符，会将 name 以数组对象的形式推入到记录中。
## 语法##

<pre class="prettyprint lang-diy">
{$push_all:{<字段名1>:[<值1>,<值2>,...,<值N>],<字段名2>:[<值1>,<值2>,...,<值N>],...}}</pre>

## 描述##

$push_all 向指定数组对象（如<字段名1>）推入每一个指定值（[<值1>,<值2>,...,<值N>]）。操作对象必须为数组类型的字段。如果记录中不存在指定的数组对象，向记录推入指定的数组对象和每一个指定的值（[<值1>,<值2>,...,<值N>]）；如果指定的值存在数组对象中，同样被推入到数组对象中。

## 示例##

* 向集合 bar 下的 arr 数组对象推入[1,2,8,9]数组。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,4,5],age:10,name:["Tom","Mike"]}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$push_all:{arr:[1,2,8,9]}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,2,4,5,1,2,8,9],age:10,name:["Mike"]}</pre>

虽然原来记录 arr 对象有元素1和2，使用 $push_all 操作符，会将[1,2,8,9]全部值推入到数组对象 arr 中。

* 向集合 bar 中推入数组对象 name，假设原记录不存在数组对象 name。如有记录：

<pre class="prettyprint lang-diy">
{arr:[1,3,4,5],age:10}</pre>

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$push_all:{name:["Tom","Jhon"]}},{name:{$exists:0}})</pre>

此操作后，记录更新为：

<pre class="prettyprint lang-diy">
{arr:[1,3,4,5],age:10,name:["Tom","Mike"]}</pre>
##语法##

<pre class="prettyprint lang-diy">
{$replace:{<字段名1>:<值1>,<字段名2>:<值2>,...}}</pre>

## 描述##

$replace 操作是将文档全部替换成"{<字段名1>:<值1>,<字段名2>:<值2>,...}"。除了保留原始的 _id 之外，原始文档的内容会全部清空，并替换成"{<字段名1>:<值1>,<字段名2>:<值2>,...}"。

## 示例##

* 选择集合 bar 下不存在 age 字段的记录，使用 $replace 替换这些记录。

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$replace:{age:0,name:'default'}},{age:{$exists:0}})</pre>
## 语法##

<pre class="prettyprint lang-diy">
{$set:{<字段名1>:<值1>,<字段名2>:<值2>,...}}</pre>

## 描述##

$set 操作是将指定的“<字段名>”更新为指定的“<值>”。如果原记录中没有指定的字段名，那将字段名和值填充到记录中；如果原记录中存在指定的字段名，那么将字段名的值更新为指定的值。

## 示例##

* 选择集合 bar 下不存在 age 字段的记录，使用 $set 更新这些记录

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{age:5,ID:10}},{age:{$exists:0}})</pre>

* 更新集合 bar 下的所有记录，使所有记录的字段 str 的值更新为“abc”

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{str:"abd"}})</pre>

* 使用 $set 更新嵌套数组对象里面的元素。字段名 arr 在集合 bar 中是一个嵌套数组对象，例如有两条记录：{arr:[1,2,3],name:"Tom"},{name:"Mike",age:20}第二条记录没有 arr 字段名

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$set:{"arr.1":4}},{name:{$exists:1}})</pre>

此操作是选择含有 name 字段的所有记录，然后使用 \$set 更新这些记录的数组对象 arr。如果原记录中没有数组对象 arr，使用 $set 会将 arr 字段以嵌套对象的方式插入到记录中。上面两条记录更新之后为：

<pre class="prettyprint lang-diy">
{arr:[1,4,3],name:"Tom"},{arr:{"1":4},name:"Mike",age:20}</pre>
## 语法##

<pre class="prettyprint lang-diy">
{$unset:{<字段名1>:"",<字段名2>:"",...}}</pre>

## 描述##

$unset 操作是删除集合中指定的字段名。如果记录中没有指定的字段名，跳过。

## 示例##

* 删除集合 bar 下记录的 name 字段和 age 字段，如果记录中没有字段 name 或 age，跳过，不做任何处理

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{name:"",age:""}})</pre>

* $unset 删除数组对象中的元素。如有一条记录：

<pre class="prettyprint lang-diy">
{arr:[1,2,3],name:"Tom"}</pre>

使用 $unset 删除第二个元素操作如下：

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{"arr.2":""}})</pre>

此操作后，记录更新为

<pre class="prettyprint lang-diy">
{arr:[1,null,3],name:"Tom"}</pre>

* $unset 删除嵌套对象中的字段。如有一条记录：

<pre class="prettyprint lang-diy">
{content:{ID:1,type:"system",position:"manager"},name:"Tom"}</pre>

content 是一个嵌套对象，它有 ID，type，position 三个字段。使用 $unset 删除 type 字段操作如下：

<pre class="prettyprint lang-javascript">
> db.foo.bar.update({$unset:{"content.type":""}})</pre>

此操作后，记录更新为

<pre class="prettyprint lang-diy">
{content:{ID:1,position:"manager"},name:"Tom"}</pre>
##描述##

$group 实现对结果集的分组，类似 SQL 中的 group by 语句。首先指定分组键（\_id） ，通过“\_id”来标识分组字段，分组字段可以是单个，也可以是多个，格式如下：

单个分组键：

<pre class="prettyprint lang-diy">
{_id:"$field"}</pre>

多个分组键：

<pre class="prettyprint lang-diy">
{_id:{field1:"$field1",field2:"$field2",...}}</pre>

##示例##

* $group 使用如下

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$major",avg_score:{$avg:"$score"},Major:{$first:"$major"}}})</pre>

该操作表示从集合 collection 中读取记录，并按 major 字段进行分组。在返回的结果集中，取各分组的第一条记录的 major 字段，重命名为 Major；对各分组中的 score 字段值求平均值，重命名为 avg_score。返回如下所示：

<pre class="prettyprint lang-diy">
{
  "avg_score": 82,
  "major": "光学"
}
{
  "avg_score": 77.25,
  "major": "物理学"
}</pre>

##$group 支持的聚集函数：##

+-----------+--------------------------------------------------------------------+
| 函数名    | 描述                                                               |
+===========+====================================================================+
| $addtoset | 将字段添加到数组中，相同的字段值只会添加一次                       |
+-----------+--------------------------------------------------------------------+
| $first    | 取分组中第一条记录中的字段值                                       |
+-----------+--------------------------------------------------------------------+
| $last     | 取分组中最后一条记录中的字段值                                     |
+-----------+--------------------------------------------------------------------+
| $max      | 取分组中字段值最大的                                               |
+-----------+--------------------------------------------------------------------+
| $min      | 取分组中字段值最小的                                               |
+-----------+--------------------------------------------------------------------+
| $avg      | 取分组中字段值的平均值                                             |
+-----------+--------------------------------------------------------------------+
| $push     | 将所有字段添加到数组中，即使数组中已经存在相同的字段值，也继续添加 |
+-----------+--------------------------------------------------------------------+
| $sum      | 取分组中字段值的总和                                               |
+-----------+--------------------------------------------------------------------+
| $count    | 对记录分组后，返回表所有的记录条数                                 |
+-----------+--------------------------------------------------------------------+

##$addtoset##

记录分组后，使用 $addtoset 将指定字段值添加到数组中，相同的字段值只会添加一次。对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 如下操作对记录分组后将指定字段值添加到数组中输出

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",Dep:{$first:"$dep"},addtoset_major:{$addtoset:"$major"}}})</pre>

此操作对记录按 dep 字段值进行分组，并使用 \$first 输出每个组第一条记录的 dep 字段，输出字段名为 Dep；又将 major 字段的值使用 \$addtoset 放入数组中返回，输出字段名为 addtoset_major，如下：

<pre class="prettyprint lang-diy">
{
  "Dep": "物电学院",
  "addtoset_major": [
    "物理学",
    "光学",
    "电学"
  ]
}
{
  "Dep": "计算机学院",
  "addtoset_major": [
    "计算机科学与技术",
    "计算机软件与理论",
    "计算机工程"
  ]
}</pre>

##$count##

记录分组后，用 $count 取出分组所包含的总记录条数。

###示例###

* 对记录分组后，返回表所有的记录条数

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{Total:{$count:"$dep"}}})
{
  "Total": 1001
}</pre>

##$first##

记录分组后，取分组中第一条记录指定的字段值，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，输出每个分组第一条记录的指定字段值

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",Dep:{$first:"$dep"},Name:{$first:"$info.name"}}})</pre>

此操作对记录按 dep 字段分组，取每个分组中第一条记录的 dep 字段值和嵌套对象 name 字段值，输出字段名分别为 Dep 和 Name，记录返回如下：

<pre class="prettyprint lang-diy">
{
  "Dep": "物电学院",
  "Name": "Lily"
}
{
  "Dep": "计算机学院",
  "Name": "Tom"
}</pre>

##$avg##

记录分组后，取分组中指定字段的平均值返回，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，返回分组中指定字段的平均值

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",avg_age:{$avg:"$info.age"},max_age:{$max:"$info.age"},min_age:{$min:"$info.age"}}})</pre>

此操作对记录按 dep 字段分组，使用 \$avg 返回每个分组中的嵌套对象 age 字段的平均值，输出字段名为 avg_age；又使用 \$min 返回每个分组中嵌套对象 age 字段的最小值，输出字段名为 min_age，使用 $max 返回每个分组中嵌套对象 age 字段的最大值，输出字段名为 max_age。记录返回如下：

<pre class="prettyprint lang-diy">
{
  "avg_age": 23.727273,
  "max_age": 36,
  "min_age": 15
}
{
  "avg_age": 24.5,
  "max_age": 30,
  "min_age": 20
}</pre>

##$max##

记录分组后，取分组中指定字段的最大值返回，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，返回分组中指定字段的最大值

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",max_score:{$max:"$score"},Name:{$last:"$info.name"}}})</pre>

此操作对记录按 dep 字段分组，使用 \$max 返回每个分组中 score 字段的最大值，输出字段名为 max_score，又使用 \$last 取每个分组中最后一条记录嵌套对象 name 字段值，输出字段名为 Name。记录返回如下：

<pre class="prettyprint lang-diy">
{
  "max_score": 93,
  "Name": "Kate"
}
{
  "max_score": 90,
  "Name": "Jim"
}</pre>

##$min##

记录分组后，取分组中指定字段的最小值返回，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，返回分组中指定字段的最小值

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",min_score:{$min:"$score"},Name:{$last:"$info.name"}}})</pre>

此操作对记录按 dep 字段分组，使用 \$min 返回每个分组中 score 字段的最小值，输出字段名为 min_score，又使用 \$last 取每个分组中最后一条记录嵌套对象 name 字段值，输出字段名为 Name。记录返回如下：

<pre class="prettyprint lang-diy">
{
  "min_score": 72,
  "Name": "Kate"
}
{
  "min_score": 69,
  "Name": "Jim"
}</pre>

##$last##

记录分组后，取分组中最后一条记录指定的字段值，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，输出每个分组最后一条记录的指定字段值

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",Major:{$addtoset:"$major"},Name:{$last:"$info.name"}}})</pre>

此操作对记录按 dep 字段分组，使用 \$last 取每个分组中最后一条记录嵌套对象 name 字段值，输出字段名为 Name，并且将每个分组中的 major 字段值使用 \$addtoset 填充到数组中返回，返回字段名为 Major；记录返回如下：

<pre class="prettyprint lang-diy">
{
  "Major": [
    "物理学",
    "光学",
    "电学"
  ],
  "Name": "Kate"
}
{
  "Major": [
    "计算机科学与技术",
    "计算机软件与理论",
    "计算机工程"
  ],
  "Name": "Jim"
}</pre>

##$push##

记录分组后，使用 $push 将指定字段值添加到数组中，即使数组中已经存在相同的值，也继续添加。对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 如下操作对记录分组后将指定字段值添加到数组中输出

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",Dep:{$first:"$dep"},push_age:{$push:"$info.age"}}})</pre>

此操作对记录按 dep 字段值进行分组，每个分组中嵌套对象 age 字段的值使用 $push 放入数组中返回，输出字段名为 push_age，如下：

<pre class="prettyprint lang-diy">
{
  "Dep": "物电学院",
  "push_age": [
    28,
    18,
    20,
    30,
    28,
    20
  ]
}
{
  "Dep": "计算机学院",
  "push_age": [
    25,
    20,
    22
  ]
}</pre>

##$sum##

记录分组后，返回每个分组中指定字段值的总和，对嵌套对象使用点操作符（.）引用字段名。

###示例###

* 对记录分组后，返回分组中指定字段值的总和

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$group:{_id:"$dep",sum_score:{$sum:"$score"},Dep:{$first:"$dep"}}})</pre>

此操作对记录按 dep 字段分组，使用 \$sum 返回每个分组中 score 字段值的总和，输出字段名为 sum_score；又使用 \$first 取每个分组中第一条记录的 dep 字段值，输出字段名为 Dep。记录返回如下：

<pre class="prettyprint lang-diy">
{
  "sum_score": 888,
  "Dep": "物电学院"
}
{
  "sum_score": 476,
  "Dep": "计算机学院"
}</pre>
## 描述##

$limit 实现在结果集中限制返回的记录条数。如果指定的记录条数大于实际的记录总数，那么返回实际的记录总数。

## 示例##

* 限制返回结果集中的前10条记录

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate( { $limit : 10 } ) </pre>

该操作表示集合 collection 中读取前10条记录。
## 描述##

\$match 与 find() 方法中的 cond 参数完全相同，通过 \$match 可以实现从集合中选择匹配条件的记录。

$match 的语法规则请参考读取操作 find() 方法的 cond 参数介绍。

## 示例

* 下面的示例使用 \$match 执行简单的匹配

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$match:{$and:[{score:80},{"info.name":{$exists:1}}]}})</pre>

该操作表示从集合 collection 中返回符合条件 score 等于80且 info 对象中的子对象 name 字段存在的记录。

* 下面的示例使用 \$match 匹配符合条件的记录，然后使用 \$group 对结果集分组，最后使用 $project 输出结果集中指定的字段名

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$match:{$and:[{score:80},{"info.name":{$exists:1}}]}},{$group:{_id:"$major"}},{$project:{major:1,dep:1}})</pre>

该操作首先集合 collection 中返回符合条件 score 等于80且 info 对象中的子对象 name 字段存在的记录，然后按 major 字段进行分组，最后选择输出结果集中的 major 和 dep 字段。
## 描述##

\$project 类似 SQL 中的 select 语句，通过使用 \$project 操作可以从记录中筛选出所需字段，字段名的值如果为1，表示选出，为0表示不选；还可以实现字段的重命名。

**Note:**

如果记录不存在所选字段，则以如下格式输出："field":null，field 为不存在的字段名。对嵌套对象使用点操作符（.）引用字段名。

## 示例##

* 使用 $project 快速地从结果集中选取所需字段

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({ $project : {title: 0,author: 1}})</pre>

此操作是选出 author 字段，而 title 字段在结果集中不输出。

* 使用 $project 重命名字段名，如下：

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({ $project : {author: 1,name:"$studentName",dep:"$info.department"}})</pre>

此操作将字段名 studentName 重命名为 name 输出，将 info 对象中的子对象 department 字段重命名为 dep。对嵌套对象，字段引用使用点操作符（.）指向。

* 下面的示例使用 $project 选择输出字段，然后使用 $match 按条件匹配记录

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({ $project: {score:1,author:1}},{$match:{score:{$gt:80}}})</pre>

此操作使用 \$project 输出所有记录的 score 和 author 字段值，然后按 $match 输出匹配条件的记录。

**Note:**

由于 \$project 选取了输出字段名，所以 $match 中字段名必须是 $project 中选出的字段名。
## 描述##

$skip 参数控制结果集的开始点，即跳过结果集中指定条数的记录。如果跳过的记录数大于总记录数，返回0条记录。

## 示例##

* 跳过10条记录返回

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate( { $skip : 10 } ) ;</pre>

该操作表示从集合 collection 中读取记录，并跳过前面10条，从第11条记录开始返回。
## 描述##

$sort 用来指定结果集的排序规则。对嵌套对象使用点操作符（.）引用字段名。

## 示例##

<pre class="prettyprint lang-javascript">
> db.collectionspace.collection.aggregate({$sort:{score:-1,name:1}});</pre>

该操作表示从集合 collection 中读取记录，并以 score 的字段值进行降序排序（1表示升序，-1表示降序）；

当记录间 score 字段值相同时，则以 name 字段值进行升序排序。
下表主要是描述 SQL 关键字与 SequoiaDB 聚集操作符的对照表。

+------------+----------------------+
| SQL 关键字 | SequoiaDB 聚集操作符 |
+============+======================+
| where      | $match               |
+------------+----------------------+
| group by   | $group               |
+------------+----------------------+
| having     | $match               |
+------------+----------------------+
| select     | $project             |
+------------+----------------------+
| order by   | $sort                |
+------------+----------------------+
| top        | $limit               |
+------------+----------------------+
| offset     | $skip                |
+------------+----------------------+


下表主要描述标准 SQL 语句与 SequoiaDB 聚集语句之间的对照。

+------------------------------------------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| SQL 语句                                                                                                               | SequoiaDB 语句                                                                                                                                                          | 描述                                                                                                                                                                           |
+========================================================================================================================+=========================================================================================================================================================================+================================================================================================================================================================================+
| select product_id as p_id , price from table                                                                           | db.cs.table.aggregate({$project:{p_id:"$product_id",price:1,date:0}})                                                                                                   | 返回所有记录的 product_id 和 price 字段，其中 product_id 重命名为 p_id，对记录中的 date 字段不返回。                                                                           |
+------------------------------------------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| select sum(price) as total from table                                                                                  | db.cs.table.aggregate({$group:{_id:null,total:{$sum:"$price"}}})                                                                                                        | 对 table 中的字段 price 值求和，并重命名为 total。                                                                                                                             |
+------------------------------------------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| select product_id, sum(price) as total from table group by product_id                                                  | db.cs.table.aggregate({$group:{ _id:"$product_id",product_id:{$first:"$product_id"},total:{$sum:"$price"}}})                                                            | 对表 table 中的记录按 product_id 字段分组；求每个分组中字段 price 值的累加和，并重命名为 total。                                                                               |
+------------------------------------------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| select product_id, sum(price) as total from table group by product_id order by total                                   | db.cs.table.aggregate({$group:{_id:"$product_id",product_id:{$first:"$product_id"},total:{$sum:"$price"}}},{$sort:{total:1}})                                           | 对表 table 中的记录按 product_id 字段分组；求每个分组中字段 price 值的累加和，并重命名为 total；对结果集按字段名 total 的值升序排序。                                          |
+------------------------------------------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| select product_type_id, product_id, sum(price) as total from table group by product_type_id, product_id                | db.cs.table.aggregate({$group:{ _id:{product_type_id:"$product_type_id",product_id:"$product_id"},product_id:{$first:"$product_id"},total:{$sum:"$price"}}})            | 对表 table 中的记录按首先按 product_type_id 字段分组，再按 product_id 字段分组；求每个分组中字段 price 值的累加和，并重命名为 total。                                          |
+------------------------------------------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| select product_id, sum(price) as total from table group by product_id having total > 1000                              | db.cs.table.aggregate({$group:{_id:"$product_id",product_id:{$first:"$product_id"},total:{$sum:"$price"}}},{$match:{total:{$gt:1000}}})                                 | 对表 table 中的记录按 product_id 字段分组；求每个分组中字段 price 值的累加和，并重命名为 total；只返回满足条件 total 字段值大于1000的分组。                                    |
+------------------------------------------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| select product_id, sum(price) as total from table where product_type_id = 1001 group by product_id                     | db.cs.table.aggregate({$match:{product_type_id:1001}},{$group:{\_id:"$product_id",product_id:{$first:"$product_id"},total:{$sum:"$price"}}})                            | 选择符合条件 product_type_id = 1001 的记录；对选出的记录按 product_id 进行分组；对每个分组中的 price 字段值就和，并重命名为 total。                                            |
+------------------------------------------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| select product_id, sum(price) as total from table where product_type_id = 1001 group by product_id having total > 1000 | db.cs.table.aggregate({$match:{product_type_id:1001}},{$group:{_id:"$product_id",product_id:{$first:"$product_id"},total:{$sum:"$price"}}},{$match:{total:{$gt:1000}}}) | 选择符合条件 product_type_id = 1001 的记录；对选出的记录按 product_id 进行分组；对每个分组中的 price 字段值就和，并重命名为 total；只返回满足条件 total 字段值大于1000的分组。 |
+------------------------------------------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| select top 10 * from table                                                                                             | db.cs.table.aggregate({$group:{_id:null}},{$limit:10})                                                                                                                  | 返回结果集中的前10条记录。                                                                                                                                                     |
+------------------------------------------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| select * from table offset 50 rows fetch next 10                                                                       | db.cs.table.aggregate({$group:{_id:null}},{$skip:50},{$limit:10})                                                                                                       | 跳过结果集中前50条记录之后，返回接下来的10条记录。                                                                                                                             |
+------------------------------------------------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
## addtoset() 函数##

将多个记录中的字段合并为一个没有重复值的数组。

## 语法##

<pre class="prettyprint lang-javascript">
> addtoset(field name)</pre>

## 示例##

* 将表中多个记录中的字段合并为一个没有重复值的数组

表中原始记录

<pre class="prettyprint lang-diy">
{a:1, b:1}
{a:2, b:2)
{a:2, b:3}
{a:2, b:3}</pre>

<pre class="prettyprint lang-javascript">
SELECT a, ADDTOSET(b) AS b FROM foo.bar GROUP BY a</pre>

得到记录

<pre class="prettyprint lang-diy">
{a:1, b:[1]}
{a:2, b:[2,3]}</pre>## as 语句##

用于为集合名或者字段名指定别名（alias）。

## 语法##
<pre class="prettyprint lang-javascript">
&lt;cs_name.cl_name | (select_set) | field_name&gt; AS &lt;alias_name&gt;</pre>
<br>
**&lt;cs_name&gt;：**集合空间名
**&lt;cl_name&gt;：**集合名
**select_set：**结果集
**field_name：**字段名
**&lt;alias_name&gt;：**别名</pre>


## 示例##

* 集合别名

<pre class="prettyprint lang-javascript">
> db.exec("select T1.age,T1.name from foo.bar as T1 where T1.age>10") </pre>

* 字段别名

<pre class="prettyprint lang-javascript">
> db.exec("select age as 年龄 from foo.bar where age>10")</pre>

* 结果集别名

<pre class="prettyprint lang-javascript">
> db.exec("select T.age,T.name from (select age,name from foo.bar) as T where T.age>10")</pre>
## avg() 函数##

用于求指定字段名的平均值。

## 语法##

<pre class="prettyprint lang-javascript">
avg(field_name) as <alisa_name></pre>

**Note:**

（1） 使用 avg 函数对字段名求平均值，必须使用别名。

（2） 对非数值型字段自动跳过。

## 示例##

* 对集合 bar 中 age 字段进行求平均值：

<pre class="prettyprint lang-javascript">
> db.exec("select avg(age) as 平均年龄 from foo.bar")</pre>## buildobj() 函数##

将记录中多个字段合并为一个对象。

## 语法##

<pre class="prettyprint lang-javascript">
buildobj(field name1,fieldname2,...)</pre>

## 示例##

* 将表中记录中多个字段合并为一个对象

表中原始记录

<pre class="prettyprint lang-diy">
{a:1,b:1,c:1}
{a:2,b:2,c:2}
{a:3,b:3,c:3}</pre>

<pre class="prettyprint lang-javascript">
SELECT a, buildobj(b, c) AS d FROM foo.bar</pre>

得到记录

<pre class="prettyprint lang-diy">
{a:1, d:{b:1, c:1}}
{a:2, d:{b:2, c:2}}
{a:3, d:{b:3, c:3}}</pre>
## count() 函数##

用于计数，返回匹配指定字段名的条数。

## 语法##

<pre class="prettyprint lang-javascript">
count(field_name) as &lt;alisa_name&gt;</pre>

**Note:**

使用 count 函数对字段名计数，必须使用别名。

## 示例##

* 对集合 bar 中 age 字段进行计数：

<pre class="prettyprint lang-javascript">
db.exec("select count(age) as 数量 from foo.bar")</pre>
## create collection 语句##

用于创建集合，必须指定集合所在的集合空间。

## 语法##

<pre class="prettyprint lang-javascript">
create collection &lt;cs_name&gt;.&lt;cl_name&gt;</pre>

-   &lt;cs_name&gt;：数据库中的集合空间名称。
-   &lt;cl_name&gt;：集合名，集合名长度不能超过127Byte，并且不能为空，在同一个集合空间中不能存在相同的集合名。

## 示例##

* 在集合空间foo下创建集合bar。

<pre class="prettyprint lang-javascript">
> db.execUpdate("create collection foo.bar") //等价于 db.foo.createCL("bar")</pre>## create collectionspace 语句##

用于创建数据库中的集合空间。

## 语法##

<pre class="prettyprint lang-javascript">
create collectionspace &lt;cs_name&gt;</pre>

-   &lt;cs_name&gt;：集合空间名称，集合空间名的最大长度为127Byte，并且不能为空。

## 示例##

* 创建名为 foo 的集合空间

<pre class="prettyprint lang-javascript">
> db.execUpdate("create collectionspace foo") //等价于 db.createCS("foo")</pre>## create index 语句##

用于在集合中创建索引。在不读取整个集合的情况下，索引使数据库应用程序可以更快地查找数据。

## 语法##
<pre class="prettyprint lang-javascript">
create [unique] index &lt;index_name&gt; on &lt;cs_name&gt;.&lt;cl_name&gt; (field1_name [asc|desc],...)</pre>

-   [unique]：标识创建的索引是否唯一。在唯一索引所指定的索引键字段上，集合中不可存在一条以上的记录完全重复。

-   &lt;index_name&gt;：索引名称

-   &lt;cs_name&gt;：集合空间名称

-   &lt;cl_name&gt;：集合名称

-   field1_name：创建索引所在的字段名，同一个索引名可以在多个字段名上创建

-   [asc|desc]：排序，asc 表示升序索引某个字段中的值，desc 表示降序索引某个字段中的值，默认为升序。

## 示例##

* 本例会创建一个简单的索引，名为“test_index”，在 foo 集合空间的 bar 集合上的 age 字段：

<pre class="prettyprint lang-javascript">
> db.execUpdate("create index test_index on foo.bar (age)")</pre>

* 如果希望以降序索引某个字段中的值，可以在字段名后面添加保留字 desc：

<pre class="prettyprint lang-javascript">
> db.execUpdate("create index test_index on foo.bar (age desc)")</pre>

* 如果希望索引不止在一个字段上，可以在括号中列出这些字段的名称，用逗号隔开：

<pre class="prettyprint lang-javascript">
> db.execUpdate("create index test_index on foo.bar (age desc,name asc)")</pre>

* 下面的实例会创建一个唯一索引：

<pre class="prettyprint lang-javascript">
> db.execUpdate("create unique index test_index on foo.bar (age)")</pre>
## delete 语句##

用于删除集合中的记录。

## 语法##

<pre class="prettyprint lang-javascript">
delete from &lt;cs_name&gt;.&lt;cl_name&gt; [where &lt;condition&gt;]</pre>

-   &lt;cs_name&gt;：集合空间名

-   &lt;cl_name&gt;：集合名

-   &lt;condition&gt;：条件，只对符合条件的记录删除

## 示例##

* 本例会删除集合中的所有记录：

<pre class="prettyprint lang-javascript">
> db.execUpdate("delete from foo.bar") </pre>

* 本例会删除符合条件 age < 10 的记录：

<pre class="prettyprint lang-javascript">
> db.execUpdate("delete from foo.bar where age &lt;10")</pre>## drop collection 语句##

用于删除集合空间中的集合。

## 语法##

<pre class="prettyprint lang-javascript">
drop collection &lt;cs_name&gt;.&lt;cl_name&gt;</pre>

-    &lt;cs_name&gt;：数据库中的集合空间名，集合空间名必须在数据库中存在；
-    &lt;cl_name&gt;：集合名，集合名也必须在指定的集合空间中存在。

## 示例##

* 删除集合空间 foo 中的集合 bar

<pre class="prettyprint lang-javascript">
> db.execUpdate("drop collection foo.bar") //等价于 db.foo.dropCL("bar")</pre>
## drop collectionspace 语句##

用于删除数据库中的集合空间。

## 语法##

<pre class="prettyprint lang-javascript">
drop collectionspace &lt;cs_name&gt;</pre>

-    &lt;cs_name&gt;：集合空间名，集合空间名必须在数据库中存在。

## 示例##

* 删除名为 foo 的集合空间

<pre class="prettyprint lang-javascript">
> db.execUpdate("drop collectionspace foo") //等价于 db.dropCS("foo")</pre>
## drop index 语句##

用于删除集合中的索引。

## 语法##

<pre class="prettyprint lang-javascript">
drop index &lt;index_name&gt; on &lt;cs_name&gt;.&lt;cl_name&gt;</pre>

-   &lt;index_name&gt;：索引名

-   &lt;cs_name&gt;：集合空间名

-   &lt;cl_name&gt;：集合名

## 示例##

* 删除集合空间 foo 中集合 bar 下名为 test_index 的索引名

<pre class="prettyprint lang-javascript">
> db.execUpdate("drop index test_index on foo.bar") //等价于 db.foo.bar.dropIndex("test_index")</pre>
## first() 函数##

选择范围内第一条数据。

## 语法##

<pre class="prettyprint lang-javascript">
first(field name)</pre>

## 示例##

* 选择表中第一条数据

表中原始记录

<pre class="prettyprint lang-diy">
{a:1, b:2}
{a:2, b:3}
{a:3, b:3)</pre>

<pre class="prettyprint lang-javascript">
SELECT FIRST(a) AS a, b FROM foo.bar GROUP BY b</pre>

得到记录

<pre class="prettyprint lang-diy">
{a:1, b:2}
{a:2, b:3)</pre>
## group by 语句##

用于对集合中的记录进行分组。

## 语法##

用于结合合计函数，根据一个或多个字段名对结果集进行分组。

-   &lt;field_name&gt;：字段名

-   [ asc | desc ]：排序，asc 表示升序，desc 表示降序，默认为 asc

## 示例

* 希望计算每个部门的员工数，并按字段名 dept_no 分组：

<pre class="prettyprint lang-javascript">
> db.exec("select dept_no，count(emp_no) as 员工总数 from foo.bar group by dept_no") </pre>

**Note:**

像 sum，count，min，max，avg 这样的计数函数必须使用别名。
使用hint显示地控制执行计划。

##使用方法##

hint的格式为"/\*+hint1 hint2 ...\*/"。但我们希望控制某个SELECT语句时，只需要在这个SELECT语句结尾处增加hint即可。属于同一个SELECT语句的hint使用空格分隔。

##hint列表##

###use_hash###

* 指定关联方式为哈希关联

<pre class="prettyprint lang-javascript">
SELECT t1.a, t2.b FROM foo.bar1 AS t1 INNER JOIN foo.bar2 AS t2 ON t1.a = t2.b /*+use_hash()*/</pre>

###use_index###

* 指定集合的扫描方式

**使用索引"myindex"进行扫描**
<pre class="prettyprint lang-javascript">
SELECT * FROM foo.bar WHERE a = 1 /*+use_index(myindex)*/</pre>

**在关联中指定某个集合使用索引"myindex"进行扫描**
<pre class="prettyprint lang-javascript">
SELECT t1.a, t2.b FROM foo.bar1 AS t1 INNER JOIN foo.bar2 AS t2 ON t1.a = t2.b /*+use_index(t1, myindex)*/</pre>

**在一个SELECT语句中使用多个hint**
<pre class="prettyprint lang-javascript">
SELECT t1.a, t2.b FROM foo.bar1 AS t1 INNER JOIN foo.bar2 AS t2 ON t1.a = t2.b /*+use_index(t1, myindex1) use_index(t2, myindex2) use_hash()*/</pre>

**指定集合不使用索引**
<pre class="prettyprint lang-javascript">
SELECT * FROM foo.bar WHERE a = 1 /*+use_index(NULL)*/</pre>

**在嵌套查询中的不同SELECT语句中使用hint**
<pre class="prettyprint lang-javascript">
SELECT t1.a, t2.b, t2.cnt
FROM foo.bar1 AS t1
     INNER JOIN (
           SELECT t3.b, t3.cnt, t4.c
           FROM(
               SELECT COUNT(a) AS cnt, b
               FROM foo.bar2
               GROUP BY b
           ) AS t3
                   INNER JOIN foo.bar3 AS t4 ON t3.b = t4.c /*+use_hash() use_index(t4, aaa)*/
      ) AS t2 ON t1.a = t2.b /*+use_index(t1, a) */</pre>## inner join 语句##

用于根据两个或多个集合中的字段名之间的关系，从这些集合中查询数据。

## 语法##

<pre class="prettyprint lang-javascript">
&lt;collection1_name | (select_set1)&gt; as &lt;alias1_name&gt; inner join &lt;collection2_name | (select_set2)&gt; as &lt;alias2_name&gt; [ON condition]</pre>

## 示例##

* 有员工信息表 foo.emp 和部门信息表 foo.dept，查询员工号 emp_no 所在的部门名 dept_name：

<pre class="prettyprint lang-javascript">
> db.exec("select E.emp_no,D.dept_name from foo.emp as E inner join foo.dept as D on E.dept_no=D.dept_no")</pre>

**Note:**

（1）不能包含非联合条件，如下写法是错误的：

<pre class="prettyprint lang-javascript">
select T1.a,T2.b from foo.bar1 as T1 inner join foo.bar2 as T2 on T1.a &lt;10 </pre>

(2）不能在 join 本层使用 select * 语句。
## insert into 语句##

用于向集合中插入新的数据。

## 语法##

<pre class="prettyprint lang-javascript">
insert into &lt;cs_name&gt;.&lt;cl_name&gt;(&lt;field1_name,field2_name,...&gt;) values(&lt;value1,value2,...&gt;)</pre>

或者

<pre class="prettyprint lang-javascript">
insert into &lt;cs_name&gt;.&lt;cl_name&gt; &lt;select_set&gt;</pre>

-   &lt;cs_name&gt;：集合空间名

-   &lt;cl_name&gt;：集合名

-   &lt;field_name&gt;：字段名

-   &lt;value&gt;：字段名所对应的值

-   &lt;select_set&gt;：查询结果集

## 示例##

* 本例会向集合 bar 中插入一条新的数据，字段名为 age 和 name，对应的值分别为（25，“Tom”）：

<pre class="prettyprint lang-javascript">
> db.execUpdate("insert into foo.bar(age,name) values(25,"Tom")")</pre>

* 本例会向集合 bar 中插入批量的数据，这些数据为集合 small 中的查询结果集：

<pre class="prettyprint lang-javascript">
> db.execUpdate("insert into foo.bar select * from big.small")</pre>
## last() 函数##

选择范围内最后一条数据。

## 语法##

<pre class="prettyprint lang-javascript">
last(field name)</pre>

## 示例##

* 选择表中最后一条数据

表中原始记录

<pre class="prettyprint lang-diy">
{a:1, b:2}
{a:2, b:3}
{a:3, b:3)</pre>

<pre class="prettyprint lang-javascript">
SELECT LAST(a) AS a, b FROM foo.bar GROUP BY b</pre>

得到记录

<pre class="prettyprint lang-diy">
{a:1, b:2}
{a:3, b:3)</pre>
## left outer join 语句##

left outer join 会从左边的集合名（collection1_name）中返回所有的记录，即使在右边的集合名（collection2_name）中没有匹配的记录。

## 语法##

<pre class="prettyprint lang-javascript">
&lt;collection1_name | (select_set1)&gt; as &lt;alias1_name&gt; left outer join &lt;collection2_name | (select_set2)&gt; as &lt;alias2_name&gt; [ON condition]</pre>

## 示例

* 有员工信息表 foo.emp 和部门信息表 foo.dept，查询员工号 emp_no 所在的部门名 dept_name：

<pre class="prettyprint lang-javascript">
> db.exec("select E.emp_no,D.dept_name from foo.emp as E left outer join foo.dept as D on E.dept_no=D.dept_no where D.dept_no &lt; 4")</pre>
## limit 语句##

用于限制返回记录个数。

## 语法##

<pre class="prettyprint lang-javascript">
limit&lt;limit_num&gt;</pre>

-   &lt;limit_num&gt;：限制数

## 示例##

* 希望返回集合中前10条记录：

<pre class="prettyprint lang-javascript">
> db.exec("select * from foo.bar limit 10") </pre>## list collections 语句##

枚举集合空间中的集合。

## 语法##

<pre class="prettyprint lang-javascript">
list collections</pre>

## 示例##

* 本例会返回集合空间中的所有集合

<pre class="prettyprint lang-javascript">
> db.exec("list collections") </pre>

结果：

<pre class="prettyprint lang-javascript">
{
	"Name": "testfoo.testbar"
}
{
	"Name":"big.small"
}
Return 2 row(s).
</pre>## list collectionspaces 语句##

枚举数据库中的集合空间。

## 语法##

<pre class="prettyprint lang-javascript">
list collectionspaces</pre>

## 示例##

* 本例会返回数据库中的所有集合空间

<pre class="prettyprint lang-javascript">
> db.exec("list collectionspaces") </pre>

结果：

<pre class="prettyprint lang-javascript">
{
  "Name": "testfoo"
}
{
  "Name":"big"
}
Return 2 row(s).
</pre>
## max() 函数##

用于返回指定字段名的最大值。

## 语法##

<pre class="prettyprint lang-javascript">
max(field_name) as &lt;alisa_name&gt;</pre>

**Note:**

使用 max 函数返回字段名的最大值时，必须使用别名。

## 示例##

* 对集合 bar 中 age 字段返回最大值：

<pre class="prettyprint lang-javascript">
> db.exec("select max(age) as 最大年龄 from foo.bar")</pre>## mergearrayset() 函数##

将多个数组字段合并为一个不包含重复字段的数组。

## 语法##

<pre class="prettyprint lang-javascript">
mergearrayset(field name)</pre>

## 示例##

* 将表中多个数组字段合并为一个不包含重复字段的数组

表中原始记录

<pre class="prettyprint lang-diy">
{a:1, b:[1,2,3]}
{a:1, b:[2,2,3]}</pre>

<pre class="prettyprint lang-javascript">
SELECT a, MERGEARRAYSET(b) AS b FROM foo.bar GROUP BY a</pre>

得到记录

<pre class="prettyprint lang-diy">
{a:1, b:[1,2,3]}</pre>
## min() 函数##

用于返回指定字段名的最小值。

## 语法##

<pre class="prettyprint lang-javascript">
min(field_name) as &lt;alisa_name&gt;</pre>

**Note:**

使用 min 函数返回字段名的最小值时，必须使用别名。

## 示例##

* 对集合 bar 中 age 字段返回最小值：

<pre class="prettyprint lang-javascript">
> db.exec("select min(age) as 最小年龄 from foo.bar")</pre>## offset语句##

用于设置跳过的记录个数。

## 语法##

<pre class="prettyprint lang-javascript">
offset&lt;offset_num&gt;</pre>

-   &lt;offset_num&gt;：跳过记录数

## 示例##

* 希望跳过前5条记录，从第5条后面开始返回：

<pre class="prettyprint lang-javascript">
> db.exec("select * from foo.bar offset 5") </pre>## order by 语句##

用于根据指定的字段名对结果集进行排序，默认为升序排序。

## 语法##

<pre class="prettyprint lang-javascript">
order by &lt;field1_name [ASC|DESC ], ...&gt; </pre>

-   &lt;field_name&gt;：字段名

-   [ asc | desc ]：排序，asc 表示升序，desc 表示降序，默认为 asc

## 示例##

* 希望计算每个部门的员工数，并按字段名 dept_no 分组，并按字段名的降序排序：

<pre class="prettyprint lang-javascript">
> db.exec("select dept_no，count(emp_no) as 员工总数 from foo.bar group by dept_no order by dept_no desc")</pre> 

**Note:**

像 sum，count，min，max，avg 这样的计数函数必须使用别名。
## push() 函数##

将多个记录中的字段合并为一个数组。

## 语法##

<pre class="prettyprint lang-javascript">
push(field name)</pre>

## 示例##

* 将表中多个记录中的字段合并为一个数组

表中原始记录

<pre class="prettyprint lang-diy">
{a:1, b:1}
{a:2, b:2}
{a:2, b:3)</pre>

<pre class="prettyprint lang-javascript">
SELECT a, PUSH(b) AS b FROM foo.bar GROUP BY a</pre>

得到记录

<pre class="prettyprint lang-diy">
{a:1, b:[1]}
{a:2, b:[2,3]}</pre>
## right outer join 语句##

right outer join 会从右边的集合名（collection2_name）中返回所有的记录，即使在左边的集合名（collection1_name）中没有匹配的记录。

## 语法##

<pre class="prettyprint lang-javascript">
&lt;collection1_name | (select_set1)&gt; as &lt;alias1_name&gt; right outer join &lt;collection2_name | (select_set2)&gt; as &lt;alias2_name&gt; [ON condition]</pre>

## 示例##

* 有员工信息表 foo.emp 和部门信息表 foo.dept，查询员工号 emp_no &lt;10 所在的部门名 dept_name：

<pre class="prettyprint lang-javascript">
> db.exec("select E.emp_no,D.dept_name from foo.emp as E right outer join foo.dept as D on E.dept_no=D.dept_no where E.emp_no &lt; 10")</pre>## select 语句##

用于从集合中选取数据，结果被存储在一个结果集中。

## 语法##

<pre class="prettyprint lang-javascript">
select * from &lt;cs_name&gt;.&lt;cl_name&gt;</pre>

或者

<pre class="prettyprint lang-javascript">
select &lt;field1_name,field2_name,...&gt; from &lt;cs_name&gt;.&lt;cl_name&gt;</pre>

-   &lt;cs_name&gt;：集合空间名

-   &lt;cl_name&gt;：集合名

-   &lt;field_name&gt;：字段名

## 示例##

* 本例会选择指定的字段名返回，如果某条符合条件的记录没有指定的字段名，那么返回它的值为 null：

<pre class="prettyprint lang-javascript">
> db.exec("select age,name from foo.bar") </pre>

结果：

<pre class="prettyprint lang-diy">
{
  "age": 10,
  "name": null
}
{
  "age": 10,
  "name": "Tom"
}
...</pre>

* 本例返回集合中的所有记录的所有字段名

<pre class="prettyprint lang-javascript">
> db.exec("select * from foo.bar")</pre>

结果：

<pre class="prettyprint lang-diy">
{
  "_id": 
  {
    "$oid": "51c909b0c5b855e029000000"
  },
  "age": 10
}
{
  "_id": 
  {
    "$oid": "51c909b9c5b855e029000001"
  },
  "age": 10,
  "name": "Tom"
}
{
  "_id": 
  {
    "$oid": "51c909c2c5b855e029000002"
  },
  "age": 10,
  "name": "Tom",
  "phone": 123456
}
...</pre>

**Note:**

（1） 可以选择类似 where，group by，order by，limit，offset 的关键字对要选择的记录做控制。

（2） 如果查询源不为集合，则本层查询中所有字段均需要引用别名（\* 除外），例如：select T.a , T.b from (select \* from foo.bar) as T where T.a &lt; 10

（3） 子查询必须包含别名，子查询中出现的别名只作用于上一层。
## split by 语句##

按照某个数组字段将记录拆分。

## 语法##

<pre class="prettyprint lang-javascript">
split by &lt;field name&gt;</pre>

## 示例##

* 拆分表中原始记录｛a:1,b:2,c:[3,4,5]｝

<pre class="prettyprint lang-javascript">
SELECT * FROM foo.bar SPLIT BY c</pre>

得到结果为：

<pre class="prettyprint lang-diy">
{a:1, b:2, c:3}
{a:1, b:2, c:4}
{a:1, b:2, c:5}</pre>
## sum() 函数##

用于求和。

## 语法##

<pre class="prettyprint lang-javascript">
sum(field_name) as &lt;alisa_name&gt;</pre>

**Note:**

（1） 使用 sum 函数对字段名求和，必须使用别名。

（2） 对非数值型字段自动跳过。


## 示例##

* 对集合 bar 中 age 字段进行求和：

<pre class="prettyprint lang-javascript">
> db.exec("select sum(age) as 年龄总和 from foo.bar")</pre>
## update 语句##

用于修改集合中的记录。

## 语法##

<pre class="prettyprint lang-javascript">
update &lt;cs_name&gt;.&lt;cl_name&gt; set (&lt;field1_name&gt;=&lt;value1&gt;,...) [where &lt;condition&gt;]</pre>

-   &lt;cs_name&gt;：集合空间名

-   &lt;cl_name&gt;：集合名

-   &lt;condition&gt;：条件，只对符合条件的记录更新

## 示例##

* 本例会修改集合中全部的记录，将记录中的 age 字段值修改为20，如果记录中不存在 age 字段，则将 age：20添加到记录中：

<pre class="prettyprint lang-javascript">
> db.execUpdate("update foo.bar set age=20") </pre>

* 本例会修改符合条件的记录，只对符合条件 age &lt; 10 的记录做修改操作：

<pre class="prettyprint lang-javascript">
> db.execUpdate("update foo.bar set age=20 where age &lt; 10")</pre>
+-------------------------------------------------+--------------------------------------------------------------------+
| 表达式                                          | 例子                                                               |
+=================================================+====================================================================+
| 单列运算                                        | select a + 1, b / 2 from foo.bar                                   |
+-------------------------------------------------+--------------------------------------------------------------------+
| 复合运算                                        | select ( a + 1 ) * ( a - 1 ), b + 1 from foo.bar                   |
+-------------------------------------------------+--------------------------------------------------------------------+
| 内层查询                                        | select t.b + 1 from ( select a as b from foo.bar split by a ) as t |
+-------------------------------------------------+--------------------------------------------------------------------+

**Note:** 

 -   单列运算，支持加（+）、减（-）、乘（*）、除（/）、模（%）。
 -   单列运算，一个表达式中只能出现一个字段。
 -   除法、取模运算，被除数为零时返回结果为null。
 -   对非数值型做算术运算时返回的结果为null。
## 概念和术语##

| SQL | SequoiaDB |
| ------ | ------ |
| database | collection space |
| table | collection |
| row | document / BSON document |
| column | field |
| index | index |
| table joins | embedded documents |
| primary key （指定任何唯一的列作为主键） | primary key （在 SequoiaDB 中，primary key 是自动创建到记录的 \_id 字段名中） |

##Create 和 Alter##

下表列出了各种 SQL 语句表级别的操作和在 SequoiaDB 中对应的操作：

| SQL 语句 | SequoiaDB 语句 |
| ------ | ------ |
| create table student (id not null, stu_id varchar(50), age number primary key(id)) | 如果未指定 \_id 字段，\_id 字段自动添加 db.collectionspace.student({stu_id:"01",age:20})，当然你也可以明确的创建一个集合 db.collectionspace.createCL("student") |
| alter table student add name varchar(50) | 集合不描述或强制执行文档的结构，即在集合上没有结构的改动操作，但是 update() 方法可以使用 &#36;set 向文档记录添加不存在的字段。db.collectionspace.student.update({},{&#36;set:{name:"Tom"}}) |
| alter table student drop column name |  集合不描述或强制执行文档的结构，即在集合上没有结构的改动操作，但是 update() 方法可以使用 &#36;unset 向文档记录删除存在的字段。db.collectionspace.student.update({},{&#36;unset:{name:"Tom"}}) |
| create index index_stu_id on student (stu_id) |  db.collectionspace.student.createIndex("index_stu_id",{stu_id:-1}) |  createIndex(),index |
| drop table student | db.collectionspace.dropCL("student") |

##Insert##

下表给出了各种 SQL 语句在表级上的插入操作和 SequoiaDB 上相应的操作：

+-------------------------------------------------+---------------------------------------------------------+
| SQL 语句                                        | SequoiaDB 语句                                          |
+=================================================+=========================================================+
| insert into student(stu_id,age) values("01",20) | db.collectionspace.student.insert({stu_id:"01",age:20}) |
+-------------------------------------------------+---------------------------------------------------------+

##Select##

下表给出了各种 SQL 语句在表级上的读操作和 SequoiaDB 上相应的操作：

+---------------------------------------------------------+----------------------------------------------------------------+
| SQL 语句                                                | SequoiaDB 语句                                                 |
+=========================================================+================================================================+
| select * from student                                   | db.collectionspace.student.find()                              |
+---------------------------------------------------------+----------------------------------------------------------------+
| select stu_id, age from student                         | db.collectionspace.student.find({},{stu_id:"01",age:20})       |
+---------------------------------------------------------+----------------------------------------------------------------+
| select * from student where age > 25                    | db.collectionspace.student.find({age:{&#36;gt:25}})            |
+---------------------------------------------------------+----------------------------------------------------------------+
| select age from student where age = 25 and stu_id= "01" | db.collectionspace.student.find({age:25,stu_id="01"},{age:25}) |
+---------------------------------------------------------+----------------------------------------------------------------+
| select count(*) from student                            | db.collectionspace.student.count()                             |
+---------------------------------------------------------+----------------------------------------------------------------+
| select count(stu_id) from student                       | db.collectionspace.student.count({stu_id:{&#36;exists:1}})     |
+---------------------------------------------------------+----------------------------------------------------------------+

## Update##

下表给出了各种 SQL 语句在表级上的更新操作和 SequoiaDB 上相应的操作：

| SQL 语句 | SequoiaDB 语句 |
| ------ | ------ |
| update student set age = 25 where stu_id = "01" | db.collectionspace.student.update({stu_id:"01"},{&#36;set:{age:25}}) |
| update student set age = age + 2 where stu_id = "01" | db.collectionspace.student.update({stu_id:"01"},{&#36;inc:{age:2}}) |

## Delete##

下表给出了各种 SQL 语句在表级上的删除记录操作和 SequoiaDB 上相应的操作：

| SQL 语句 | SequoiaDB 语句 |
| ------ | ------ |
| delete from student where age = 20 | db.collectionspace.student.remove({age:20}) |
| delete from student | db.collectionspace.student.remove() |
## 文档##

| 描述 | 限制 |
| ------ | ------ |
| 文档最小长度 | 至少包含一个字段 |
| 文档最大长度 | 转为 BSON 结构后16777168字节 |
| 字段名 | 不以“$”起始，不包含“.” |

## 集合##

| 描述 | 限制 |
| ------ | ------ |
| 集合名最大长度 | 127字节 |
| 集合名 | 不以“$”或“SYS”起始，不包含“.” |
| 单节点集合最大容量 | 为集合空间最大容量 |
| 单集合空间集合最大数量 | 4096 |

## 集合空间##

| 描述 | 限制 |
| ------ | ------ |
| 集合空间名最大长度 | 127字节 |
| 集合空间名 | 不以“$”或“SYS”起始，不包含“.” |
| 数据页大小 | 4096、8192、16384、32768、65536 |
| 单节点集合空间最大容量 | 对应每种数据页大小，分别为512GB、1TB、2TB、4TB、8TB |
| 单节点集合空间最大数量 | 4096 |

## 索引##

| 描述 | 限制 |
| ------ | ------ |
| 每条数据索引键最大长度 | 1024字节 |
| 索引定义总长度（包括索引名，索引键名等） | 转为 BSON 后小于等于数据页大小-48字节 |
| 复合索引 | 文档里符合索引所定义的字段中，最多一个字段包含数组 |
| 索引键定义排序值 | 1或者-1 |
| 单集合最大索引数量 | 64 |

## 数据库##

| 描述 | 限制 |
| ------ | ------ |
| 日志文件最小 | 64MB |
| 日志文件最大 | 2GB |

## 节点##

| 描述 | 限制 |
| ------ | ------ |
| 每分区组最大节点数量 | 7 |
| 创建节点 | 必须使用 hostname，而不是 IP 地址 |
| 网络 | 集群中所有系统必须能够使用 hostname 互相访问 |
| 主节点选举条件 | 分区组内至少存在超过半数节点参与选举 |

## 分区##

| 描述 | 限制 |
| ------ | ------ |
| 数据切分 | 同一时刻每个集合只能进行一个范围的切分 |
| 分区键 | 分区键数值在数据插入后不可修改 |
| \_id | 分区集合中 \_id 仅保证分区组内唯一，不保证全局唯一 |
| 唯一索引 | 必须包含分区键中所有字段 |

## 驱动##

| 描述 | 限制 |
| ------ | ------ |
| 线程安全 | 每个连接对象与其下属的子对象为非线程安全不同连接对象之间为线程安全 |
| Description | Error Code |
| ------ | ------ |
| IO 错误 | -1 |
| 无可用内存 | -2 |
| 权限错误 | -3 |
| 文件不存在 | -4 |
| 文件已存在 | -5 |
| 非法输入参数 | -6 |
| 非法长度 | -7 |
| 中断错误 | -8 |
| 文件结束 | -9 |
| 系统错误 | -10 |
| 无剩余空间 | -11 |
| 引擎调度单元状态错误 | -12 |
| 超时错误 | -13 |
| 数据库已暂停 | -14 |
| 网络错误 | -15 |
| 网络已从远程关闭 | -16 |
| 数据库正在关闭 | -17 |
| 应用被强制退出 | -18 |
| 非法路径错误 | -19 |
| 非预期文件类型 | -20 |
| 存储单元中无可用空间 | -21 |
| 集合已存在 | -22 |
| 集合不存在 | -23 |
| 数据记录过大 | -24 |
| 数据记录不存在 | -25 |
| 溢出记录已存在 | -26 |
| 非法记录 | -27 |
| 存储单元需要重组 | -28 |
| 集合结尾 | -29 |
| 上下文已打开 | -30 |
| 上下文已关闭 | -31 |
| 选项暂不支持 | -32 |
| 集合空间已存在 | -33 |
| 集合空间不存在 | -34 |
| 非法存储单元 | -35 |
| 上下文不存在 | -36 |
| 超过一个索引字段包含数组 | -37 |
| 索引键已存在 | -38 |
| 索引键过大 | -39 |
| 索引块无空间 | -40 |
| 索引键不存在 | -41 |
| 最大数量索引已存在 | -42 |
| 初始化索引失败 | -43 |
| 集合已被删除 | -44 |
| 两条记录拥有同样的键值和RID | -45 |
| 同名索引已存在 | -46 |
| 索引不存在 | -47 |
| 非预期索引状态 | -48 |
| 索引结束 | -49 |
| 去重缓冲区已满 | -50 |
| 非法谓词 | -51 |
| 索引不存在 | -52 |
| 索引提示非法 | -53 |
| 无可用临时集合 | -54 |
| 存储空间数量已最大 | -55 |
| \$id索引不可被删除 | -56 |
| 日志不在缓冲区内 | -57 |
| 日志不在文件中 | -58 |
| 复制组不存在 | -59 |
| 复制组已存在 | -60 |
| 非法请求ID | -61 |
| 会话ID不存在 | -62 |
| 系统引擎调度单元不可被终止 | -63 |
| 数据库未连接 | -64 |
| 非预期结果 | -65 |
| 记录损坏 | -66 |
| 备份已开始 | -67 |
| 备份未结束 | -68 |
| 备份正在进行 | -69 |
| 备份文件损坏 | -70 |
| 主节点不存在 | -71 |
| 请求节点不存在 | -72 |
| 引擎帮助参数  | -73 |
| 非法连接状态 | -74 |
| 非法句柄 | -75 |
| 对象已释放或不存在 | -76 |
| 监听端口已被占用 | -77 |
| 无法监听端口 | -78 |
| 无法连接到指定地址 | -79 |
| 连接不存在 | -80 |
| 发送失败 | -81 |
| 定时器标示不存在 | -82 |
| 路由信息不存在 | -83 |
| 消息错误 | -84 |
| 非法网络句柄 | -85 |
| 重组文件不合法 | -86 |
| 重组文件为只读模式 | -87 |
| 集合状态非法 | -88 |
| 集合不为重组状态 | -89 |
| 复制组未激活 | -90 |
| 非法复制组成员 | -91 |
| 集合状态不兼容 | -92 |
| 存储单元版本不兼容 | -93 |
| 本地组版本信息过期 | -94 |
| 非法数据页大小 | -95 |
| 远程组版本信息过期 | -96 |
| 投票失败 | -97 |
| 日志记录损坏 | -98 |
| LSN超出边界 | -99 |
| 未知消息 | -100 |
| 更新信息无变化 | -101 |
| 未知消息 | -102 |
| 空栈错误 | -103 |
| 非主节点 | -104 |
| 数据节点不足 | -105 |
| 数据节点不存在编目信息 | -106 |
| 数据节点编目版本过旧 | -107 |
| 协调节点编目版本过旧 | -108 |
| 超出最大组上限 | -109 |
| 同步日志失败 | -110 |
| 执行日志失败 | -111 |
| HTTP头结构错误 | -112 |
| 协商失败 | -113 |
| 日志元数据移动失败 | -114 |
| 数据文件空间管理段错误 | -115 |
| 应用程序中断 | -116 |
| 应用程序断开连接 | -117 |
| 字符编码错误 | -118 |
| 协调节点查询失败 | -119 |
| 缓冲区数组满 | -120 |
| 子上下文冲突 | -121 |
| 协调节点接收到集合结尾消息 | -122 |
| 日志文件大小不统一 | -123 |
| 日志文件不可识别 | -124 |
| 无可用资源 | -125 |
| 非法LSN编号 | -126 |
| 命名管道请求发送的数据过大 | -127 |
| 编目授权错误 | -128 |
| 节点间正在全量同步 | -129 |
| 协调节点分配数据节点失败 | -130 |
| PHP驱动内部错误 | -131 |
| 协调节点发送失败 | -132 |
| 节点组信息不存在 | -133 |
| 远程节点断开连接 | -134 |
| 无法找到匹配的协调节点信息 | -135 |
| 更新协调节点失败 | -136 |
| 未知操作请求 | -137 |
| 协调节点无法找到本地节点组信息 | -138 |
| DMS数据块损坏 | -139 |
| 远程集群管理失败 | -140 |
| 远程引擎已部分被停止 | -141 |
| 服务正在启动 | -142 |
| 服务已经启动 | -143 |
| 服务正在重启 | -144 |
| 节点已存在 | -145 |
| 节点不存在 | -146 |
| 锁定失败 | -147 |
| DMS状态与当前请求不兼容 | -148 |
| 数据库重建已开始 | -149 |
| 数据库重建正在进行 | -150 |
| 协调节点的缓存中无数据 | -151 |
| 求值过程发生错误 | -152 |
| 分区组已经存在 | -153 |
| 分区组不存在 | -154 |
| 节点不存在 | -155 |
| 启动节点失败 | -156 |
| 节点配置冲突 | -157 |
| 空分区组 | -158 |
| 该操作仅适用于协调节点 | -159 |
| 在节点上执行操作失败 | -160 |
| 已经存在互斥任务 | -161 |
| 指定任务不存在 | -162 |
| 系统集合数据损坏 | -163 |
| \$shard索引不可被删除 | -164 |
| 该节点不能运行该命令 | -165 |
| 该服务平面不能运行该命令 | -166 |
| 该Group信息不存在 | -167 |
| Group名称冲突 | -168 |
| 非分区集合 | -169 |
| 记录不包含合法的分区键 | -170 |
| 存在一个不兼容的任务 | -171 |
| 集合在指定复制组中不存在 | -172 |
| 指定的任务不存在 | -173 |
| 记录包含超过一条分区键 | -174 |
| 已存在互斥任务 | -175 |
| 指定的分区键不合法或不在源节点范围内 | -176 |
| 唯一索引必须包含分区键中的所有字段 | -177 |
| 分区键不可被更新 | -178 |
| 没有权限 | -179 |
| 编目节点地址未指定 | -180 |
| 当前记录已被删除 | -181 |
| 搜索条件无法满足任何条件的匹配 | -182 |
| 索引页重组后指定位置存在不同左子节点 | -183 |
| 记录内含有重复字段名 | -184 |
| 插入操作尝试写入过多数据 | -185 |
| 合并连接只接受相等谓词 | -186 |
| 跟踪已经启动 | -187 |
| 跟踪缓冲区不存在 | -188 |
| 跟踪文件不合法 | -189 |
| 请求的事务锁不兼容 | -190 |
| 系统正在执行回滚操作 | -191 |
| 导入数据库时遇到无效的记录 | -192 |
| 发现相同的变量名 | -193 |
| 列名存在歧义 | -194 |
| SQL中存在语法错误 | -195 |
| 无效的事务操作 | -196 |
| 加入锁的等待队列中 | -197 |
| 记录已被删除 | -198 |
| 索引被删除或非法状态 | -199 |
| 重复创建编目节点集群 | -200 |
| 解析json文件错误 | -201 |
| 解析CSV文件错误 | -202 |
| 日志文件超长 | -203 |
| 不能删除组内唯一的节点 | -204 |
| 需要手工完成清理工作 | -205 |
| 系统存在其它组时不能删除编目节点和组 | -206 |
| 分区组不存在 | -207 |
| 无法删除存在非空组 | -208 |
| 到达Queue队列结尾 | -209 |
| 集合不存在分区键索引,不能按百分比分区 | -210 |
| 指定参数字段不存在 | -211 |
| 跟踪断点数量过多 | -212 |
| 预取器繁忙 | -213 |
| 域不存在 | -214 |
| 域已存在 | -215 |
| 组不存在指定域中 | -216 |
| 分区类型不为哈稀 | -217 |
| 分区百分比过低 | -218 |
| 后台任务已完成 | -219 |
| 集合已处于装载状态 | -220 |
| 进行装载操作回滚 | -221 |
| RouteID与当前节点不一致 | -222 |
| 服务已经存在 | -223 |
| 未找到字段 | -224 |
| csv字段行结束 | -225 |
| 未知的文件类型 | -226 |
| 部分节点导出配置文件失败 | -227 |
| 非主空节点 | -228 |
| 索引文件特征值与数据文件不匹配 | -229 |
| 引擎版本参数 | -230 |
| 客户端帮助参数 | -231 |
| 客户端版本参数 | -232 |
| 存储过程不存在 | -233 |
| 非法删除集合分区 | -234 |
| 重复关联集合分区 | -235 |
| 无效的分区集合 | -236 |
| 新增区间与现有区间冲突 | -237 |
| 新增区间不合法 | -238 |
| 达到高水位 | -239 |
| 该备份已存在 | -240 |
| 该备份不存在 | -241 |
| 无效的集合分区 | -242 |
| 后台任务被取消 | -243 |
| 分区集合的分区类型必须是范围分区 | -244 |
| 未包含合法的分区键字段 | -245 |
| 分区集合不支持此操作 | -246 |
| 重复定义索引 | -247 |
| 正在删除CS | -248 |
| 节点数量达到上限 | -249 |
| 节点处于业务故障状态 | -250 |
| 节点信息过期 | -251 |
| 等待备节点同步该操作失败 | -252 |
| 未开启事务功能 | -253 |
| 客户端连接池已满 | -254 |
| 文件描述符已达到上限 | -255 |
| 域非空 | -256 |
| REST接收的数据大小超过最大值 | -257 |
| 构建bson失败 | -258 |
| 存储过程参数越界 | -259 |
| 未知的REST命令 | -260 |
| 在数据节点上执行命令失败 | -261 |
| 域中不包含任何数据组 | -262 |
| 提示用户修改登陆密码 | -263 |
| 部分节点未返回成功 | -264 |
| 不同版本的OMAgent已运行 | -265 |
| 无法找到后台任务信息 | -266 |
| 后台任务正在回滚 | -267 |
| 大对象的序列不存在 | -268 |
| 大对象处于不可用状态 | -269 |
| 数据格式非UTF-8编码 | -270 |
| 后台任务失败 | -271 |
| 大对象没有打开 | -272 |
| 大对象已打开 | -273 |
| 节点正在启动恢复 | -274 |
| 集合空间中存在有效集合 | -275 |
| 'localhost'和'127.0.0.1'不能与其它hostname和IP地址混合使用 | -276 |
| 如果使用'localhost'和'127.0.0.1'作为节点主机名，协调节点与编目节点必须在同一主机上 | -277 |
| 指定分区组不是数据分区组 | -278 |
| 建立集合时如果指定autoindexid为false，则无法进行update/delete | -279 |
| 当存在主节点或者本地LSN不为最大值时无法强行升主 | -280 |
| 镜像地址和本集群地址冲突 | -281 |
| 数据组不存在镜像 | -282 |
| 数据组存在镜像 | -283 |
| 镜像处于生效状态 | -284 |
| 集群未配置镜像 | -285 |
| 本集群和镜像集群都为可写状态 | -286 |
| 集群为只读模式 | -287 |
| 在’查询并修改‘操作使用排序时必须使用索引 | -288 |
| ’查询并修改‘操作不能在多个节点或子表上执行skip和limit操作 | -289 |
| 指定目录不为空 | -290 |
| 已经存在一个可以处理本场景的索引 | -291 |
| 集群已配置镜像 | -292 |
| 命令为本地运行模式 | -293 |
| 不是一个特殊类型的Sdb Shell对象 | -294 |
| 指定用户已存在 | -295 |
| 表记录为空 | -296|
| 大对象的序列已经存在 | -297 |
| 集群不存在 | -298 |
| 业务不存在 | -299 |SequoiaDB 数据库是一款新型企业级分布式非关系型数据库，帮助企业用户降低 IT 成本，并对大数据的存储与分析提供了一个坚实，可靠，高效与灵活的底层平台。

##SequoiaDB version 2.0 版本说明##

**接口变更：**

- 增加事务锁超时时间配置transactiontimeout
- createIndex接口增加SortBufferSize参数
- createCS接口增加IndexEngineType参数（社区版）
- upsert增加$setOnInsert操作符
- update增加$replace操作符
- 增加findAndRemove和findAndUpdate接口
- 增加createIdIndex和dropIdIndex接口
- 复制组增加对节点的attach及detach接口
- C驱动增加sdbGetCLName 及 sdbGetCLFullName接口

**主要特性：**

- 新版SequoiaDB OM（业务操作管理系统）
- Rocksdb作为可选索引存储引擎（社区版）
- 基于字典的数据压缩算法功能
- 索引重建机制优化
- 支持事务超时机制
- 支持手动触发深度刷盘机制
- 优化数据组节点心跳检测
- 优化sdb_fdw访问性能
- 支持findAndRemove和findAndModify原子操作
- 支持update $replace操作符
- 支持内置SQL使用hint语法选择指定索引
- 支持selector的数学运算，字符串和cast操作
- 支持手工创建删除ID索引功能
- 增加SYSSpare组，用于管理后备节点


**工具优化：**

- 新版高性能多并发导入工具

**注意事项：**

- 社区版要求系统安装glibc 2.15以及libstdc++ 6.0.18以上版本

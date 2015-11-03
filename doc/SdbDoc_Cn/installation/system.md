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

**Note: **每台作为数据库服务器的机器都需要配置。

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

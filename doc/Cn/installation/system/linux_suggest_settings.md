如果您采用Linux操作系统，在安装 SequoiaDB 产品之前，建议您按照下述操作和步骤调整Linux系统的环境配置，以保障系统的稳定高效运行。

##Linux 推荐配置##

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
	> cat /proc/sys/vm/min_free_kbytes
	> cat /proc/sys/vm/overcommit_memory
	> cat /proc/sys/vm/overcommit_ratio</pre>

	2.  添加下列参数至 /etc/sysctl.conf 文件调整内核参数：

	<pre class="prettyprint lang-diy">
	vm.swappiness = 0
	vm.dirty_ratio = 100
	vm.dirty_background_ratio = 40
	vm.dirty_expire_centisecs = 3000
	vm.vfs_cache_pressure = 200
	vm.min_free_kbytes = &lt;物理内存大小的8%，单位KB&gt;
	vm.overcommit_memory = 2
	vm.overcommit_ratio = 85</pre>

	**Note:**
		
	当数据库可用物理内存不足 8GB 时不需使用 vm.swappiness = 0；上述 dirty 类参数只是建议值，具体系统设置时请按原则（控制系统的 flush 进程只采用脏页超时机制刷新脏页，而不采用脏页比例超支刷新脏页）进行设置。

	3.  执行如下命令，使配置生效：

	<pre class="prettyprint lang-javascript">
	> /sbin/sysctl -p</pre>

	4.	停用transparent_hugepage，编辑/etc/rc.local，在第一行“#!/bin/sh”的下一行添加如下两行内容：

	<pre class="prettyprint lang-diy">
	echo never > /sys/kernel/mm/transparent_hugepage/enabled
	echo never > /sys/kernel/mm/transparent_hugepage/defrag</pre>
	
	5.  执行如下命令，使配置生效：
	
    <pre class="prettyprint lang-javascript">
	> source /etc/rc.local</pre>

    6.	分别执行如下两条命令，输出结果中都有“[never]”则表示成功关闭了transparent_hugepage，如果是“never”并且有“[always]”或者“[madvise]”则关闭失败：

	<pre class="prettyprint lang-javascript">
	> cat /sys/kernel/mm/transparent_hugepage/enabled
	> cat /sys/kernel/mm/transparent_hugepage/defrag</pre>    

-   NUMA的影响
    
    Linux系统默认开启NUMA，NUMA默认的内存分配策略是优先在进程所在CPU节点的本地内存中分配，会导致CPU节点之间内存分配不均衡，比如当某个CPU节点的内存不足时，会导致swap产生，而不是从远程节点分配内存，即使另一个CPU节点上有足够的物理内存。这种内存分配策略的初衷是好的，为了内存更接近需要它的进程，但不适合数据库这种大规模内存使用的应用场景，不利于充分利用系统的物理内存。我们建议用户在使用SequoiaDB时关闭NUMA。

-   关闭NUMA
    
    关闭Linux系统的NUMA的方法主要有两种，一种是通过BIOS禁用NUMA；另一种是通过修改gurb的配置文件，CentOS、SUSE、Ubuntu的grub配置文件有差异，同一款Linux的不同版本配置也略有不同，此处会介绍CentOS6.4和Ubuntu12.04的配置方法以供参考，SUSE和CentOS修改方法类似。建议通过设置BIOS来禁用NUMA。

    1.  开机按快捷键进入BIOS设置界面，关闭NUMA。不同品牌的主板或服务器，具体操作略有差异，此处不作详细介绍。
    
    2.  修改grub的配置文件，关闭NUMA：
    
        1）  CentOS6.4的grub配置文件修改，以root权限编辑/etc/grub.conf，找到kernel行，该行类似如下（不同的版本内容略有差异，但开头有“kernel /vmlinuz-”）：
    
        <pre class="prettyprint lang-diy">
        kernel /vmlinuz-2.6.32-358.el6.x86_64 ro root=/dev/mapper/vg_centos64001-lv_root rd_NO_LUKS rd_LVM_LV=vg_centos64001/lv_root rd_NO_MD rd_LVM_LV=vg_centos64001/lv_swap crashkernel=128M LANG=zh_CN.UTF-8  KEYBOARDTYPE=pc KEYTABLE=us rd_NO_DM rhgb quiet</pre>

        在kernel行的末尾，空格再添加“numa=off”，如果有多个kernel行，则每个kernel行都要添加。

        2）  Ubuntu12.04的grub文件修改，以root权限编辑/boot/grub/grub.cfg，找到Linux引导行，该行类似如下（不同版本内容略有差异，但开头有“linux   /boot/vmlinuz-”）：
    
        <pre class="prettyprint lang-diy">
        linux   /boot/vmlinuz-3.2.0-31-generic root=UUID=92191cd8-3690-4cd4-9f42-95d392c9d828 ro</pre>

        在Linux引导行的末尾，空格再添加“numa=off”，如果有多个Linux引导行，则每个Linux引导行都要添加。

    3.   验证NUMA是否成功关闭，shell执行如下命令：
    
    <pre class="prettyprint lang-javascript">
    > numastat</pre>

    如果输出结果中只有node0，则表示成功禁用了NUMA，如果有node1出现则失败。


	**Note:** 

	每台作为数据库服务器的机器都需要配置。

-   数据库目录结构

    用户应尽可能使数据目录，索引目录与日志目录存放在不同物理磁盘中，以减少顺序 I/O 与随机 I/O 之间的竞争。并且保证安装路径上的文件夹具有可读和可执行权限。
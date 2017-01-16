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

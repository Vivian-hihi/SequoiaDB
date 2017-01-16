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

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

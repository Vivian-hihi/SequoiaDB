SequoiaDB 自带一个 JavaScript shell，可以从命令行与 SequoiaDB 实例交互。这个 shell 非常有用，通过它可以执行管理操作、检查运行实例，亦或做其他尝试。这个 shell 对于使用 SequoiaDB 来说是至关重要的工具。

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

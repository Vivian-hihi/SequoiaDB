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

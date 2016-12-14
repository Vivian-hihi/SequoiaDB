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

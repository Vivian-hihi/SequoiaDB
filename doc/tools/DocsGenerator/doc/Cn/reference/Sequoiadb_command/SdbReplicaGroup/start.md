##语法##
***rg.start()***

启动分区组。分区组启动之后才能创建节点及其他操作。也可以使用方法 db.startRG(< name >)) 启动指定的节点。

## 示例##

* 启动分区组命令：

<pre class="prettyprint lang-javascript">
> rg.start() //等价于 db.startRG("group")</pre>

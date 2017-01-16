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

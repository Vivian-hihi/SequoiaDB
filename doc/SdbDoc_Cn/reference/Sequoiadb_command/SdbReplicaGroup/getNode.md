##语法##
***rg.getNode(&lt;nodename|hostname&gt;,&lt;servicename&gt;)***

返回指定节点信息。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| nodename | string | 节点名称。 | nodename 与 hostname 二选一 |
| hostname | string | 主机名。 | hostname 与 nodename 二选一 |
| servicename | string | 服务器名称。 | 是 |

## 格式##

rg.getNode() 方法定义了两个参数，第一个参数可是节点名称也可以是主机名，第二个参数为服务器名称。两个参数的类型都是字符串型，且必填。

<pre class="prettyprint lang-diy">
("&lt;节点名称&gt;|&lt;主机名&gt;","&lt;服务器名称&gt;")</pre>

## 示例##

* 返回指定主机名和服务器名的节点

<pre class="prettyprint lang-javascript">
> rg.getNode("vmsvr2-suse-x64","11800")</pre>

返回：

<pre class="prettyprint lang-diy">
vmsvr2-suse-x64:11800</pre>

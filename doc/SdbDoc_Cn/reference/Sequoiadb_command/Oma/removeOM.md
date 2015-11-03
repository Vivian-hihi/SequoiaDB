##语法##
***oma.removeOM(< svcname >)***

删除一个 om 节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |

**Note:**

* 指定删除的节点必须存在，否则出现异常。

## 示例##

* 在集群中删除一个端口号为11830的 om 节点

<pre class="prettyprint lang-javascript">
oma.removeOM(11830)</pre>

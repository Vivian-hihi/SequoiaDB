##语法##
***oma.removeCoord(< svcname >)***

在集群中删除一个 coord 节点。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| svcname | string | 节点端口号。 | 是 |

**Note:**

* 指定删除的节点必须存在，否则出现异常。

## 示例##

* 在集群中删除一个端口号为11810的 coord 节点

<pre class="prettyprint lang-javascript">
oma.removeCoord(11810)</pre>

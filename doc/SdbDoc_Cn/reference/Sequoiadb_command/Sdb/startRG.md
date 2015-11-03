## 语法##
***db.startRG(&lt;name&gt;)***

启动指定的分区组。分区组启动后才能在分区组上创建节点。这个方法等价于 rg.start()。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 分区组的名称 | 是 |

## 格式##

db.startRG() 的方法定义包含 name 一个参数，参数类型为字符串，为要启动的分区组名。

<pre class="prettyprint lang-diy">
("&lt;分区组名&gt;")</pre>

**Note:**

指定的分区组名必须存在，不然返回异常；如果指定的分区组已经启动，再使用该方法同样会出现异常。

## 示例##

* 启动分区组名为 group 的命令如下：

<pre class="prettyprint lang-javascript">
> db.startRG("group")</pre>

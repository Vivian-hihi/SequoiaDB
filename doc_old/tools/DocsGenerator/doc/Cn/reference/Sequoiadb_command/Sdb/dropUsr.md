## 语法##
***db.dropUsr(&lt;name&gt;,&lt;password&gt;)***

删除数据库已有的用户名和密码。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 用户名。 | 是 |
| password | string | 密码 | 是 |

## 格式##

dropUsr() 方法的定义格式 name 和 password 两个参数，两个参数都是字符串类型。

<pre class="prettyprint lang-diy">
("<用户名>","<密码>")</pre>

## 示例##

* 删除用户名为 root，密码为 admin 的数据库权限。

<pre class="prettyprint lang-javascript">
> db.dropUsr("root","admin")</pre>

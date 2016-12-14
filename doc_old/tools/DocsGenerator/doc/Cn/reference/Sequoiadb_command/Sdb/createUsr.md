##语法##
***db.createUsr(&lt;name&gt;,&lt;password&gt;)***

为数据库创建用户名和密码。防止非法用户对数据库进行非法操作。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 用户名 | 是 |
| password | string | 密码 | 是 |

## 格式##

createUsr() 方法的定义格式 name 和 password 两个参数，两个参数都是字符串类型，且可以为空串。

<pre class="prettyprint lang-diy">
("<用户名>","<密码>")</pre>

**Note:**

* 当为数据库设置了用户名和密码时，那么只能使用正确的用户名和密码才能登录数据库进行相关操作。此时登录数据库的命令如下格式：


## 示例##

* 为数据库创建用户名为 root，密码为 admin 的命令如下：

<pre class="prettyprint lang-javascript">
> db = new Sdb("&lt;hostname&gt;","&lt;port&gt;","&lt;name&gt;","&lt;password&gt;")
> db.createUsr("root","admin")</pre>

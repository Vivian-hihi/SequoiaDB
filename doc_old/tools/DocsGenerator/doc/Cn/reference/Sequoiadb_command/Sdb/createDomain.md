##语法##
***db.createDomain(&lt;name&gt;,&lt;groups&gt;,[options])***

创建一个域。域中可以包含若干个复制组（Replica Group）。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| name | string | 域名，全局唯一。 | 是 |
| groups | Json 数组 | 域包含的复制组。 | 是 |
| options | Json 对象 | 在创建域时可以通过 options 设置其他属性。 | 否 |

##格式##

目前通过 options 可设置域的属性有：

| 属性名 | 描述 | 格式 |
| ------ | ------ | ------ |
| AutoSplit | 自动切分散列分区集合。 | AutoSplit:true\|false |

**Note:**

* AutoSplit 只作用于散列分区集合。
* 不能在空域（不包含复制组）创建集合空间。

##示例##

* 创建一个域，包含两个复制组。

<pre class="prettyprint lang-javascript">
> db.createDomain('mydomain',['datagroup1','datagroup2'])</pre>

* 创建一个域，包含两个复制组，并且指定自动切分。

<pre class="prettyprint lang-javascript">
> db.createDomain('mydomain',['datagroup1','datagroup2'],{AutoSplit:true})</pre>

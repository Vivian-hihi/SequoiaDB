##语法##
***db.flushConfigure(&lt;rule&gt;)***

将配置刷盘至配置文件

##参数描述##

参数名    参数类型    描述         是否必填
--------- ----------- ------------ ----------
rule      Json 对象   刷盘规则     是

##rule 格式##

属性名  |  描述                       |                             格式
---------| ----------------------------------------------------- |  --------
Global   | true 表示将全系统配置刷盘，false 表示只将本节点配置刷盘  | Global:true

##示例##

-   刷盘数据库配置

<pre class="prettyprint lang-javascript">
> db.flushConfigure({Global:true});</pre>


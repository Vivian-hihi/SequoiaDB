##语法##
***db.flushConfigure(&lt;rule&gt;)***

刷盘配置至配置文件

##参数描述##

参数名    参数类型    描述         是否必填
--------- ----------- ------------ ----------
rule      Json 对象   刷盘规则     是

##rule 格式##

属性名  |  描述                       |                             格式
---------| ----------------------------------------------------- |  --------
Global   | true 表示刷盘全系统配置，false 表示只刷盘本节点配置  | Global:true

##示例##

-   刷盘数据库配置

<pre class="prettyprint lang-javascript">
> db.flushConfigure({Global:true});</pre>

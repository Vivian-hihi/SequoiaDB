##语法##
***rg.reelect([options])***

在复制组中进行重新选举。

##参数描述##

参数名    参数类型    描述         是否必填
--------- ----------- ------------ ----------
options   Json 对象   参数集合。   否

##options 选项##

参数名    参数类型   描述                           默认值
--------- ---------- ------------------------------ --------
Seconds   int        重新选举需要在多少秒内完成。   30

**Note:**

-   返回超时错误代表在规定时间内重选没有完成。通过 db.listReplicaGroups() 观察最终结果。
-   只有复制组中存在主节点时才可以进行重新选举。

##示例##

-   在“datagroup1”中进行重新选举，超时时间为60s。

<pre class="prettyprint lang-javascript">
> var rg = db.getRG("datagroup1") ;
> rg.reelect({Seconds:60});</pre>

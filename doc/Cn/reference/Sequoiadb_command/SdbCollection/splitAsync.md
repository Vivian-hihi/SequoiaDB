##语法##
***db.collectionspace.collection. splitAsync(&lt;source group&gt;,&lt;target group&gt;,&lt;percent(0~100)|condition&gt;, [endcondition])***

该操作与 [split()](SdbDoc_Cn/reference/Sequoiadb_command/SdbCollection/split.html) 功能相同，但该操作为异步分区操作，分区任务建立后立即返回任务 ID。

/************************************
*@Description: 指定唯一索引收集统计信息
*@author:      zhaoyu
*@createdate:  2017.11.9
*@testlinkCase:seqDB-11615
**************************************/
function main()
{
   var clName = COMMCLNAME + "_11615";
   var insertNum = 4000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   
   //创建索引
   commCreateIndex( dbcl, "a", {a:1}, true, true);
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	
	//检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:1000};
   var actExplains = getCommonExplain( dbcl, findConf);
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:1}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( actExplains, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( actExplains, expExplains )
	
	println("check result before analyze success!");

   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName, Index: "a"} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:1000};
   var actExplains = getCommonExplain( dbcl, findConf);
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:1}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( actExplains, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( actExplains, expExplains )
   
   println("check result after analyze success!");
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );
  
 }
 main()
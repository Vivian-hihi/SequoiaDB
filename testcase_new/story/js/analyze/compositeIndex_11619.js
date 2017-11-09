/************************************
*@Description: 指定复合索引收集统计信息
*@author:      zhaoyu
*@createdate:  2017.11.9
*@testlinkCase:seqDB-11619
**************************************/
function main()
{
   var clName = COMMCLNAME + "_11619";
   var insertNum = 2000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   
   //创建索引
   commCreateIndex( dbcl, "a", {a:1,a0:-1,a1:1,a2:-1});
   
   //插入记录
	insertDatas( dbcl, insertNum );
	
	//检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:9000,a0:9000,a1:9000,a2:9000,a3:9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
	
	println("check result before analyze success!");

   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName, Index: "a"} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:9000,a0:9000,a1:9000,a2:9000,a3:9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
   
   println("check result after analyze success!");
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );
  
 }
 main()
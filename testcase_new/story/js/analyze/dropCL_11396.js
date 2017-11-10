/************************************
*@Description: drop cl清空统计信息
*@author:      zhaoyu
*@createdate:  2017.11.8
*@testlinkCase:seqDB-11396
**************************************/
function main()
{
   var clName = COMMCLNAME + "_11396";
   var insertNum = 2000;
	var sameValues = 9000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   
   //创建索引
   commCreateIndex( dbcl, "a", {a:1});
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues );
	
	//检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
	
	println("check result before analyze success!");

   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName});
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
   
   println("check result after analyze success!");
   
   //drop cl
   commDropCL( db, COMMCSNAME, clName) ;
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", false, false );
   
   println("check result after drop cl success!");
   
   //再次创建cl、创建索引、插入相同数据
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   commCreateIndex( dbcl, "a", {a:1});
	
   insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( dbcl, findConf, null, null, expExplains )
   
   println("check result after create the same index success!");
   
   //清空环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" ) ;
  
 }
 main()
/************************************
*@Description: drop cs清空统计信息
*@author:      zhaoyu
*@createdate:  2017.11.8
*@testlinkCase:seqDB-11397
**************************************/
function main()
{
   csName = COMMCSNAME + "_11397";
   clName = COMMCLNAME + "_11397";
   
   //清理环境
   commDropCS( db, csName, true, "drop cs before test" );
   
   //创建cs、cl
   commCreateCS( db, csName, true);
   var dbcl = commCreateCL( db, csName, clName);
   
   //创建索引
   commCreateIndex( dbcl, "a", {a:1});
   
   //插入记录
	insertDatas( dbcl );
	
	//检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:10001}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, csName, clName, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, csName, clName, findConf, null, null, expExplains )
	
	println("check result before analyze success!");

   //执行统计
   analyze( db );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:10001}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, csName, clName, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, csName, clName, findConf, null, null, expExplains )
   
   println("check result after analyze success!");
   
   //drop cs
   commDropCS( db, csName);
   
   //检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   
   println("check result after drop cs success!");
   
   //再次创建cs、cl、创建索引、插入相同数据
   commCreateCS( db, csName, true);
   var dbcl = commCreateCL( db, csName, clName);
   commCreateIndex( dbcl, "a", {a:1});
   insertDatas( dbcl );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:10001}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, csName, clName, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, csName, clName, findConf, null, null, expExplains )
   
   println("check result after create the same index success!");
   
   //清空环境
   commDropCS( db, csName);
 }
 main()
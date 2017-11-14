/************************************
*@Description: drop cs清空统计信息
*@author:      zhaoyu
*@createdate:  2017.11.8
*@testlinkCase:seqDB-11397
**************************************/
function main()
{
   var csName1 = COMMCSNAME + "_11397_1";
   var csName2 = COMMCSNAME + "_11397_2";
   var clName = COMMCLNAME + "_11397";
   var insertNum = 2000;
	var sameValues = 9000;
   
   //清理环境
   commDropCS( db, csName1, true, "drop cs before test" );
   commDropCS( db, csName2, true, "drop cs before test" );
   
   //创建cs、cl
   commCreateCS( db, csName1, true);
   commCreateCS( db, csName2, true);
   var dbcl1 = commCreateCL( db, csName1, clName);
   var dbcl2 = commCreateCL( db, csName2, clName);
   
   //创建索引
   commCreateIndex( dbcl1, "a", {a:1});
   commCreateIndex( dbcl2, "a", {a:1});
   
   //插入记录
	insertDiffDatas( dbcl1, insertNum );
	insertSameDatas( dbcl1, insertNum, sameValues );
	
	insertDiffDatas( dbcl2, insertNum );
	insertSameDatas( dbcl2, insertNum, sameValues );
	
	//获取主备节点
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary1 = db.getCS(csName1).getCL(clName);
   var dbclPrimary2 = db.getCS(csName2).getCL(clName);
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave1 = db.getCS(csName1).getCL(clName);
	var dbclSlave2 = db.getCS(csName2).getCL(clName);
	
	//检查统计信息
   checkStat( db, csName1, clName, "a", false, false );
   checkStat( db, csName2, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary1, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave1, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclPrimary2, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave2, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check result before analyze success!");

   //执行统计
   analyze( db, {CollectionSpace : csName1} );
   
   //检查统计信息
   checkStat( db, csName1, clName, "a", true, true );
   checkStat( db, csName2, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary1, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave1, findConf);
   checkExplain( actExplains, expExplains );
   
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary2, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave2, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after analyze success!");
   
   //drop cs
   commDropCS( db, csName1);
   
   //检查统计信息
   checkStat( db, csName1, clName, "a", false, false );
   checkStat( db, csName2, clName, "a", false, false );
   
   println("check result after drop cs success!");
   
   //再次创建cs、cl、创建索引、插入相同数据
   commCreateCS( db, csName1, true);
   var dbcl1 = commCreateCL( db, csName1, clName);
   commCreateIndex( dbcl1, "a", {a:1});
   
	insertDiffDatas( dbcl1, insertNum );
	insertSameDatas( dbcl1, insertNum, sameValues );
	
	//获取主备节点
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db.getCS(csName1).getCL(clName);
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db.getCS(csName1).getCL(clName);
   
   //检查统计信息
   checkStat( db, csName1, clName, "a", false, false );
   checkStat( db, csName2, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary1, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave1, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclPrimary2, findConf);
   checkExplain( actExplains, expExplains );
   var actExplains = getCommonExplain( dbclSlave2, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after create the same index success!");
   
   //清空环境
   commDropCS( db, csName1);
   commDropCS( db, csName2);
 }
 main()
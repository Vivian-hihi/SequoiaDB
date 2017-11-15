/************************************
*@Description: 数据有变化时执行统计
*@author:      zhaoyu
*@createdate:  2017.11.15
*@testlinkCase:seqDB-11732
**************************************/
function main()
{
   var csName = COMMCSNAME + "_11732";
   var clName = COMMCLNAME + "_11732";
   var insertNum = 2000;
	var sameValues = 9000;
   
   //清理环境
   commDropCS( db, csName, true, "drop cs before test" );
   
   //创建cl
   var dbcl = commCreateCL( db, csName, clName);
   
   //创建索引
   commCreateIndex( dbcl, "a", {a:1});
   
	//获取主备节点
   var db1 = new Sdb(db);
   db1.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db1.getCS(csName).getCL(clName);
   var db2 = new Sdb(db);
   db2.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db2.getCS(csName).getCL(clName);
	
	//执行统计
   analyze( db, {Collection: csName + "." + clName, Index: "a"} );
   
	//检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
	println("analyze index success!");
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues );
	
   //执行统计
   analyze( db, {Collection: csName + "." + clName, Index: "a"} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("analyze index after data change success!");
   
   //清空数据
   dbcl.truncate();
   
   //执行统计
   analyze( db, {Collection: csName + "." + clName} );
   
	//检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
	println("analyze cl success!");
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues );
	
   //执行统计
   analyze( db, {Collection: csName + "." + clName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("analyze cl after data change success!");
   
   //清空数据
   dbcl.truncate();
   
   //执行统计
   analyze( db, {CollectionSpace: csName} );
   
	//检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
	println("analyze cs success!");
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues );
	
   //执行统计
   analyze( db, {CollectionSpace: csName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("analyze cs after data change success!");
   
   //清理环境
   commDropCS( db, csName);
   db1.close();
   db2.close();
  
 }
 main()
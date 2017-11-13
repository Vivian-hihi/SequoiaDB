/************************************
*@Description: 指定cs清空统计信息,cs下存在1个/多个cl,且cl存在索引和记录
*@author:      zhaoyu
*@createdate:  2017.11.8
*@testlinkCase:seqDB-11608
**************************************/
function main()
{
	var csName = COMMCSNAME + "_11608";
   var clName1 = COMMCLNAME + "_11608_1";
   var clName2 = COMMCLNAME + "_11608_2";
   var clName3 = COMMCLNAME + "_11608_3";
   var insertNum = 2000;
	var sameValues = 9000;
   
   //清理环境
   commDropCS( db, csName);
   
   //创建cl
   var dbcl1 = commCreateCL( db, csName, clName1);
   var dbcl2 = commCreateCL( db, csName, clName2);
   var dbcl3 = commCreateCL( db, csName, clName3);
   
   //创建索引
   commCreateIndex( dbcl1, "a", {a:1});
   commCreateIndex( dbcl2, "a", {a:1});
   commCreateIndex( dbcl3, "a", {a:1});
   
   //插入记录
	insertDiffDatas( dbcl1, insertNum );
	insertSameDatas( dbcl1, insertNum, sameValues );
	insertDiffDatas( dbcl2, insertNum );
	insertSameDatas( dbcl2, insertNum, sameValues );
	insertDiffDatas( dbcl3, insertNum );
	insertSameDatas( dbcl3, insertNum, sameValues );
	
	//获取主备节点
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary1 = db.getCS(csName).getCL(clName1);
   var dbclPrimary2 = db.getCS(csName).getCL(clName2);
   var dbclPrimary3 = db.getCS(csName).getCL(clName3);
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave1 = db.getCS(csName).getCL(clName1);
   var dbclSlave2 = db.getCS(csName).getCL(clName2);
   var dbclSlave3 = db.getCS(csName).getCL(clName3);
	
	//检查统计信息
   checkStat( db, csName, clName1, "a", false, false );
   println("check cl:" + clName1 + " before analyze success!");
   
   checkStat( db, csName, clName2, "a", false, false );
   println("check cl:" + clName2 + " before analyze success!");
   
   checkStat( db, csName, clName3, "a", false, false );
   println("check cl:" + clName3 + " before analyze success!");
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains1 = getCommonExplain( dbclPrimary1, findConf);
   var actExplains2 = getCommonExplain( dbclPrimary2, findConf);
   var actExplains3 = getCommonExplain( dbclPrimary3, findConf);
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );
   checkExplain( actExplains3, expExplains );
   
   println("check primary node find before analyze success!");
   
   var actExplains1 = getCommonExplain( dbclSlave1, findConf);
   var actExplains2 = getCommonExplain( dbclSlave2, findConf);
   var actExplains3 = getCommonExplain( dbclSlave3, findConf);
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );
   checkExplain( actExplains3, expExplains );
   
   println("check slave node find before analyze success!");
   
   //执行统计
   analyze( db, {CollectionSpace: csName} );
   
   //检查统计信息
   checkStat( db, csName, clName1, "a", true, true );
   println("check cl:" + clName1 + " after analyze success!");
   
   checkStat( db, csName, clName2, "a", true, true );
   println("check cl:" + clName2 + " after analyze success!");
   
   checkStat( db, csName, clName3, "a", true, true );
   println("check cl:" + clName3 + " after analyze success!");
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains1 = getCommonExplain( dbclPrimary1, findConf);
   var actExplains2 = getCommonExplain( dbclPrimary2, findConf);
   var actExplains3 = getCommonExplain( dbclPrimary3, findConf);
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );
   checkExplain( actExplains3, expExplains );
   
   println("check primary node find after analyze success!");
   
   var actExplains1 = getCommonExplain( dbclSlave1, findConf);
   var actExplains2 = getCommonExplain( dbclSlave2, findConf);
   var actExplains3 = getCommonExplain( dbclSlave3, findConf);
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );
   checkExplain( actExplains3, expExplains );
   
   println("check slave node find after analyze success!");

   //truncate 其中一个cl的记录,再次执行统计
   dbcl1.truncate();
   analyze( db, {CollectionSpace: csName} );
   
   //检查统计信息
   checkStat( db, csName, clName1, "a", false, false );
   println("check cl:" + clName1 + " after truncate cl success!");
   
   checkStat( db, csName, clName2, "a", true, true );
   println("check cl:" + clName2 + " after truncate cl success!");
   
   checkStat( db, csName, clName3, "a", true, true );
   println("check cl:" + clName3 + " after truncate cl success!");
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains1 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains1 = getCommonExplain( dbclPrimary1, findConf);
   var actExplains2 = getCommonExplain( dbclPrimary2, findConf);
   var actExplains3 = getCommonExplain( dbclPrimary3, findConf);
   checkExplain( actExplains1, expExplains1 );
   checkExplain( actExplains2, expExplains2 );
   checkExplain( actExplains3, expExplains2 );
   
   println("check primary node find after truncate cl success!");
   
   var actExplains1 = getCommonExplain( dbclSlave1, findConf);
   var actExplains2 = getCommonExplain( dbclSlave2, findConf);
   var actExplains3 = getCommonExplain( dbclSlave3, findConf);
   checkExplain( actExplains1, expExplains1 );
   checkExplain( actExplains2, expExplains2 );
   checkExplain( actExplains3, expExplains2 );
   
   println("check slave node find after truncate cl success!");
   
   //删除其中一个cl中的索引,再次执行统计
   commDropIndex( dbcl3, "a" );
   analyze( db, {CollectionSpace: csName} );
   
   //检查统计信息
   checkStat( db, csName, clName1, "a", false, false );
   println("check cl:" + clName1 + " after drop index success!");
   
   checkStat( db, csName, clName2, "a", true, true );
   println("check cl:" + clName2 + " after drop index success!");
   
   checkStat( db, csName, clName3, "a", true, false );
   println("check cl:" + clName3 + " after drop index success!");
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains1 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   var expExplains3 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains1 = getCommonExplain( dbclPrimary1, findConf);
   var actExplains2 = getCommonExplain( dbclPrimary2, findConf);
   var actExplains3 = getCommonExplain( dbclPrimary3, findConf);
   checkExplain( actExplains1, expExplains1 );
   checkExplain( actExplains2, expExplains2 );
   checkExplain( actExplains3, expExplains3 );
   
   println("check primary node find after drop index success!");
   
   var actExplains1 = getCommonExplain( dbclSlave1, findConf);
   var actExplains2 = getCommonExplain( dbclSlave2, findConf);
   var actExplains3 = getCommonExplain( dbclSlave3, findConf);
   checkExplain( actExplains1, expExplains1 );
   checkExplain( actExplains2, expExplains2 );
   checkExplain( actExplains3, expExplains3 );
   
   println("check slave node find after drop index success!");
   
   //删除其他2个cl，再次执行统计
   commDropCL( db, csName, clName1, true, true,"drop CL in the beginning" ) ;
   commDropCL( db, csName, clName3, true, true,"drop CL in the beginning" ) ;
   analyze( db, {CollectionSpace: csName} );
   
   //检查统计信息
   checkStat( db, csName, clName2, "a", true, true );
   println("check only one cl:" + clName2 + " success!");
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains2 = getCommonExplain( dbclPrimary2, findConf);
   checkExplain( actExplains2, expExplains2 );
   
   var actExplains2 = getCommonExplain( dbclSlave2, findConf);
   checkExplain( actExplains2, expExplains2 );
   
   //清空环境
   commDropCS( db, csName);
  
 }
 main()
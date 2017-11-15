/************************************
*@Description: 将所有统计信息加载至缓存再清空 
*@author:      zhaoyu
*@createdate:  2017.11.13
*@testlinkCase:seqDB-11630
**************************************/
function main()
{
   var csName1 = COMMCSNAME + "_11630_1";
   var clName1 = COMMCLNAME + "_11630_1";
   var csName2 = COMMCSNAME + "_11630_2";
   var clName2 = COMMCLNAME + "_11630_2";
   var insertNum = 2000;
	var sameValues = 9000;
   
   //清理环境
   commDropCS( db, csName1, true, "drop cs before test" );
   commDropCS( db, csName2, true, "drop cs before test" );
   
   //创建cl
   commCreateCS( db, csName1, true);
   var dbcl11 = commCreateCL( db, csName1, clName1);
   var dbcl12 = commCreateCL( db, csName1, clName2);
   
   commCreateCS( db, csName2, true);
   var dbcl21 = commCreateCL( db, csName2, clName1);
   var dbcl22 = commCreateCL( db, csName2, clName2);
   
   //创建索引
   commCreateIndex( dbcl11, "a", {a:1});
   commCreateIndex( dbcl12, "a", {a:1});
   commCreateIndex( dbcl21, "a", {a:1});
   commCreateIndex( dbcl22, "a", {a:1});
   
   //插入记录
	insertDiffDatas( dbcl11, insertNum );
	insertSameDatas( dbcl11, insertNum, sameValues );
	
	insertDiffDatas( dbcl12, insertNum );
	insertSameDatas( dbcl12, insertNum, sameValues );
	
	insertDiffDatas( dbcl21, insertNum );
	insertSameDatas( dbcl21, insertNum, sameValues );
	
	insertDiffDatas( dbcl22, insertNum );
	insertSameDatas( dbcl22, insertNum, sameValues );
	
	//获取主备节点
	
	var db1 = new Sdb(db);
   db1.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary11 = db1.getCS(csName1).getCL(clName1);
   var dbclPrimary12 = db1.getCS(csName1).getCL(clName2);
   var dbclPrimary21 = db1.getCS(csName2).getCL(clName1);
   var dbclPrimary22 = db1.getCS(csName2).getCL(clName2);
   
   var db2 = new Sdb(db);
   db2.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave11 = db2.getCS(csName1).getCL(clName1);
   var dbclSlave12 = db2.getCS(csName1).getCL(clName2);
   var dbclSlave21 = db2.getCS(csName2).getCL(clName1);
   var dbclSlave22 = db2.getCS(csName2).getCL(clName2);
   
   //执行统计
   analyze( db );
   
   //检查统计信息
   checkStat( db, csName1, clName1, "a", true, true );
   checkStat( db, csName1, clName2, "a", true, true );
   checkStat( db, csName2, clName1, "a", true, true );
   checkStat( db, csName2, clName2, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains11 = getCommonExplain( dbclPrimary11, findConf);
   checkExplain( actExplains11, expExplains );
   var actExplains11 = getCommonExplain( dbclSlave11, findConf);
   checkExplain( actExplains11, expExplains );
   
   var actExplains12 = getCommonExplain( dbclPrimary12, findConf);
   checkExplain( actExplains12, expExplains );
   var actExplains12 = getCommonExplain( dbclSlave12, findConf);
   checkExplain( actExplains12, expExplains );
   
   var actExplains21 = getCommonExplain( dbclPrimary21, findConf);
   checkExplain( actExplains21, expExplains );
   var actExplains21 = getCommonExplain( dbclSlave21, findConf);
   checkExplain( actExplains21, expExplains );
   
   var actExplains22 = getCommonExplain( dbclPrimary22, findConf);
   checkExplain( actExplains22, expExplains );
   var actExplains22 = getCommonExplain( dbclSlave22, findConf);
   checkExplain( actExplains22, expExplains );
   
   println("check result after analyze success!");
   
   //生成默认统计信息
   analyze( db, {Mode:3, Collection: csName1 + "." + clName1} );
   
   //检查统计信息
   checkStat( db, csName1, clName1, "a", true, false );
   checkStat( db, csName1, clName2, "a", true, true );
   checkStat( db, csName2, clName1, "a", true, true );
   checkStat( db, csName2, clName2, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains1 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains11 = getCommonExplain( dbclPrimary11, findConf);
   checkExplain( actExplains11, expExplains1 );
   var actExplains11 = getCommonExplain( dbclSlave11, findConf);
   checkExplain( actExplains11, expExplains1 );
   
   var actExplains12 = getCommonExplain( dbclPrimary12, findConf);
   checkExplain( actExplains12, expExplains2 );
   var actExplains = getCommonExplain( dbclSlave12, findConf);
   checkExplain( actExplains12, expExplains2 );
   
   var actExplains21 = getCommonExplain( dbclPrimary21, findConf);
   checkExplain( actExplains21, expExplains2 );
   var actExplains21 = getCommonExplain( dbclSlave21, findConf);
   checkExplain( actExplains21, expExplains2 );
   
   var actExplains22 = getCommonExplain( dbclPrimary22, findConf);
   checkExplain( actExplains22, expExplains2 );
   var actExplains22 = getCommonExplain( dbclSlave22, findConf);
   checkExplain( actExplains22, expExplains2 );
   
   println("check result analyze mode set 3 success!");
   
   //手工修改主备节点统计信息
   var mcvValues = [{a:8000},{a:sameValues},{a:9001}];
   var fracs = [500,9000,500];
   updateIndexStateInfo( db, csName1, clName1, "a", mcvValues, fracs );
   
   var mcvValues = [{a:8000},{a:sameValues},{a:9001}];
   var fracs = [500,100,9400];
   updateIndexStateInfo( db, csName2, clName2, "a", mcvValues, fracs );
   
   //统计信息加载至缓存
   analyze( db, {Mode:4} );
   
   //检查统计信息
   checkStat( db, csName1, clName1, "a", true, true );
   checkStat( db, csName1, clName2, "a", true, true );
   checkStat( db, csName2, clName1, "a", true, true );
   checkStat( db, csName2, clName2, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   var expExplains2 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains11 = getCommonExplain( dbclPrimary11, findConf);
   checkExplain( actExplains11, expExplains1 );
   var actExplains11 = getCommonExplain( dbclSlave11, findConf);
   checkExplain( actExplains11, expExplains1 );
   println("check cs:" + csName1 + " cl:" + clName1 + " analyze mode set 4 success!");
   
   var actExplains12 = getCommonExplain( dbclPrimary12, findConf);
   checkExplain( actExplains12, expExplains1 );
   var actExplains12 = getCommonExplain( dbclSlave12, findConf);
   checkExplain( actExplains12, expExplains1 );
   println("check cs:" + csName1 + " cl:" + clName2 + " analyze mode set 4 success!");
   
   var actExplains21 = getCommonExplain( dbclPrimary21, findConf);
   checkExplain( actExplains21, expExplains1 );
   var actExplains21 = getCommonExplain( dbclSlave21, findConf);
   checkExplain( actExplains21, expExplains1 );
   println("check cs:" + csName2 + " cl:" + clName1 + " analyze mode set 4 success!");
   
   var actExplains22 = getCommonExplain( dbclPrimary22, findConf);
   checkExplain( actExplains22, expExplains2 );
   var actExplains22 = getCommonExplain( dbclSlave22, findConf);
   checkExplain( actExplains22, expExplains2 );
   println("check cs:" + csName2 + " cl:" + clName2 + " analyze mode set 4 success!");
   
   //清空统计信息
   analyze( db, {Mode:5} );
   
   //检查统计信息
   checkStat( db, csName1, clName1, "a", true, true );
   checkStat( db, csName1, clName2, "a", true, true );
   checkStat( db, csName2, clName1, "a", true, true );
   checkStat( db, csName2, clName2, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   var expExplains2 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains11 = getCommonExplain( dbclPrimary11, findConf);
   checkExplain( actExplains11, expExplains1 );
   var actExplains11 = getCommonExplain( dbclSlave11, findConf);
   checkExplain( actExplains11, expExplains1 );
   println("check cs:" + csName1 + " cl:" + clName1 + " analyze mode set 5 success!");
   
   var actExplains12 = getCommonExplain( dbclPrimary12, findConf);
   checkExplain( actExplains12, expExplains1 );
   var actExplains12 = getCommonExplain( dbclSlave12, findConf);
   checkExplain( actExplains12, expExplains1 );
   println("check cs:" + csName1 + " cl:" + clName2 + " analyze mode set 5 success!");
   
   var actExplains21 = getCommonExplain( dbclPrimary21, findConf);
   checkExplain( actExplains21, expExplains1 );
   var actExplains21 = getCommonExplain( dbclSlave21, findConf);
   checkExplain( actExplains21, expExplains1 );
   println("check cs:" + csName2 + " cl:" + clName1 + " analyze mode set 5 success!");
   
   var actExplains22 = getCommonExplain( dbclPrimary22, findConf);
   checkExplain( actExplains22, expExplains2 );
   var actExplains22 = getCommonExplain( dbclSlave22, findConf);
   checkExplain( actExplains22, expExplains2 );
   println("check cs:" + csName2 + " cl:" + clName2 + " analyze mode set 5 success!");
   
   //再次更新统计信息
   var mcvValues = [{a:8000},{a:sameValues},{a:9001}];
   var fracs = [500,100,9400];
   updateIndexStateInfo( db, csName1, clName1, "a", mcvValues, fracs );
   
   var mcvValues = [{a:8000},{a:sameValues},{a:9001}];
   var fracs = [500,9000,500];
   updateIndexStateInfo( db, csName2, clName2, "a", mcvValues, fracs );
   
   //检查统计信息
   checkStat( db, csName1, clName1, "a", true, true );
   checkStat( db, csName1, clName2, "a", true, true );
   checkStat( db, csName2, clName1, "a", true, true );
   checkStat( db, csName2, clName2, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   var expExplains2 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains11 = getCommonExplain( dbclPrimary11, findConf);
   checkExplain( actExplains11, expExplains1 );
   var actExplains11 = getCommonExplain( dbclSlave11, findConf);
   checkExplain( actExplains11, expExplains1 );
   println("check cs:" + csName1 + " cl:" + clName1 + " after update index stat but no analyze success!");
   
   var actExplains12 = getCommonExplain( dbclPrimary12, findConf);
   checkExplain( actExplains12, expExplains1 );
   var actExplains12 = getCommonExplain( dbclSlave12, findConf);
   checkExplain( actExplains12, expExplains1 );
   println("check cs:" + csName1 + " cl:" + clName2 + " after update index stat but no analyze success!");
   
   var actExplains21 = getCommonExplain( dbclPrimary21, findConf);
   checkExplain( actExplains21, expExplains1 );
   var actExplains21 = getCommonExplain( dbclSlave21, findConf);
   checkExplain( actExplains21, expExplains1 );
   println("check cs:" + csName2 + " cl:" + clName1 + " after update index stat but no analyze success!");
   
   var actExplains22 = getCommonExplain( dbclPrimary22, findConf);
   checkExplain( actExplains22, expExplains2 );
   var actExplains22 = getCommonExplain( dbclSlave22, findConf);
   checkExplain( actExplains22, expExplains2 );
   println("check cs:" + csName2 + " cl:" + clName2 + " after update index stat but no analyze success!");
   
   //再次清空缓存
   analyze( db, {Mode:5} );
   
   //检查统计信息
   checkStat( db, csName1, clName1, "a", true, true );
   checkStat( db, csName1, clName2, "a", true, true );
   checkStat( db, csName2, clName1, "a", true, true );
   checkStat( db, csName2, clName2, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   var expExplains2 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains11 = getCommonExplain( dbclPrimary11, findConf);
   checkExplain( actExplains11, expExplains2 );
   var actExplains11 = getCommonExplain( dbclSlave11, findConf);
   checkExplain( actExplains11, expExplains2 );
   println("check cs:" + csName1 + " cl:" + clName1 + " update index stat and analyze mode set 5 success!");
   
   var actExplains12 = getCommonExplain( dbclPrimary12, findConf);
   checkExplain( actExplains12, expExplains1 );
   var actExplains12 = getCommonExplain( dbclSlave12, findConf);
   checkExplain( actExplains12, expExplains1 );
   println("check cs:" + csName1 + " cl:" + clName2 + " update index stat and analyze mode set 5 success!");
   
   var actExplains21 = getCommonExplain( dbclPrimary21, findConf);
   checkExplain( actExplains21, expExplains1 );
   var actExplains21 = getCommonExplain( dbclSlave21, findConf);
   checkExplain( actExplains21, expExplains1 );
   println("check cs:" + csName2 + " cl:" + clName1 + " update index stat and analyze mode set 5 success!");
   
   var actExplains22 = getCommonExplain( dbclPrimary22, findConf);
   checkExplain( actExplains22, expExplains1 );
   var actExplains22 = getCommonExplain( dbclSlave22, findConf);
   checkExplain( actExplains22, expExplains1 );
   println("check cs:" + csName2 + " cl:" + clName2 + " update index stat and analyze mode set 5 success!");
   
   //清理环境
   commDropCS( db, csName1);
   commDropCS( db, csName2);
   db1.close();
   db2.close();
  
 }
 main()
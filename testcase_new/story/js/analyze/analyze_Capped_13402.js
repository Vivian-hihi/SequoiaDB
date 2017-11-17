/************************************
*@Description: 指定固定集合空间/固定集合收集统计信息 
*@author:      zhaoyu
*@createdate:  2017.11.16
*@testlinkCase:seqDB-13402
**************************************/
function main()
{
   var csName = COMMCSNAME + "_13402";
   var clName = COMMCLNAME + "_13402";
   var insertNum = 2000;
	var sameValues = 9000;
   
   //清理环境
   commDropCS( db, csName, true, "drop cs before test" );
   //创建cl
   csOptions = {Capped:true};
   commCreateCS( db, csName, false, "create capped cs", csOptions);
   
   var clOption = {Capped:true, Size:1024, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption);
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues );
	
   //获取主备节点
   var db1 = new Sdb(db);
   db1.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db1.getCS(csName).getCL(clName);
   var db2 = new Sdb(db);
   db2.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db2.getCS(csName).getCL(clName);
   
   //检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result before analyze success!");
   
   //指定cs执行统计
   analyze( db, {CollectionSpace: csName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after analyze cs success!");
   
   //生成默认统计信息
   analyze( db, {Mode:3, Collection: csName + "." + clName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 3 success!");
   
   //执行统计
   analyze( db, {Collection: csName + "." + clName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after analyze cl success!");
   
   //生成默认统计信息
   analyze( db, {Mode:3, Collection: csName + "." + clName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 3 success!");
 
   //手工修改主节点统计信息
   var mcvValues = [{a:8000},{a:9000},{a:9001}];
   var fracs = [500,9000,500];
   updateIndexStateInfo( db, csName, clName, "a", mcvValues, fracs );
   
   //统计信息加载至缓存
   analyze( db, {Mode:4, Collection: csName + "." + clName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 4 success!");
   
   //清空统计信息
   analyze( db, {Mode:5, Collection: csName + "." + clName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 5 success!");
   
   //再次更新统计信息
   var mcvValues = [{a:8000},{a:9000},{a:9001}];
   var fracs = [50,50,50];
   updateIndexStateInfo( db, csName, clName, "a", mcvValues, fracs );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after update index stat but no analyze success!");
   
   //再次清空缓存
   analyze( db, {Mode:5, Collection: csName + "." + clName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after update index stat and analyze mode set 5 success!");
   
   //清理环境
   commDropCS( db, csName );
   db1.close();
   db2.close();
  
 }
 main()
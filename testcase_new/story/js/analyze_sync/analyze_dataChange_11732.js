/************************************
*@Description: 数据有变化时执行统计
*@author:      zhaoyu
*@createdate:  2017.11.15
*@testlinkCase:seqDB-11732
**************************************/
function main()
{
   try
	{
	   //判断独立模式
	   if( true == commIsStandalone( db ) )
      {
         println( "run mode is standalone" );
         return;
      }
   }catch( e )
   {
      throw e;
   } 
   
   var csName = COMMCSNAME + "_11732";
   var clName = COMMCLNAME + "_11732";
   var insertNum = 2000;
	var sameValues = 9000;
   
   //清理环境
   commDropCS( db, csName, true, "drop cs before test" );
   
   //创建cl
   var dbcl = commCreateCL( db, csName, clName);
   var groupName = getSrcGroup( csName, clName );
   var primaryNode = db.getRG(groupName).getMaster();
   var nodeId = parseInt(primaryNode.getNodeDetail().split(":")[0]);
   println("nodeId:" + nodeId);
   
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
   analyze( db, {GroupName: groupName} );
   
	//检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
	println("analyze group success!");
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues );
	
   //执行统计
   analyze( db, {GroupName: groupName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("analyze group after data change success!");
   
   //清空数据
   dbcl.truncate();
   
   //执行统计
   analyze( db, {NodeID: nodeId} );
   
	//检查统计信息
   checkStat( db, csName, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
	println("analyze node success!");
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues );
	
   //执行统计
   analyze( db, {NodeID: nodeId} );
   
   //检查统计信息
   checkStat( db, csName, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("analyze node after data change success!");
   
   //清理环境
   commDropCS( db, csName);
   db1.close();
   db2.close();
  
 }
 main()
/************************************
*@Description: 指定node将统计信息重新加载至缓存再清空 
*@author:      zhaoyu
*@createdate:  2017.11.15
*@testlinkCase:seqDB-11634
**************************************/
function main()
{
   //独立模式及1节点模式不执行该用例
   try
	{
	   //判断独立模式
	   if( true == commIsStandalone( db ) )
      {
         println( "run mode is standalone" );
         return;
      } 
          
      //判断1节点模式
      var groups = new Array();
      groups[0] = commGetGroups( db )[0][0].GroupName;
      var nodes = getNodesInGroups(db, groups);  
      if( 1 === nodes[0].length )
      {
         println("only one node");
         return ;
      }
   }
   catch( e )
   {
      throw e;
   }
   
   var clName = COMMCLNAME + "_11634";
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
	
	//获取主备节点
   var db1 = new Sdb(db);
   db1.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db1.getCS(COMMCSNAME).getCL(clName);
   var db2 = new Sdb(db);
   db2.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db2.getCS(COMMCSNAME).getCL(clName);
	
   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after analyze success!");
   
   //生成默认统计信息
   analyze( db, {Mode:3, Collection: COMMCSNAME + "." + clName} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 3 success!");
   
   //手工修改主备节点统计信息
   var mcvValues = [{a:8000},{a:sameValues},{a:9001}];
   var fracs = [500,9000,500];
   updateIndexStateInfo( db, COMMCSNAME, clName, "a", mcvValues, fracs );
   
   //统计信息加载至缓存
   var groupName = getSrcGroup( COMMCSNAME, clName );
   var primaryNode = db.getRG(groupName).getMaster();
   var nodeId = parseInt(primaryNode.getNodeDetail().split(":")[0]);
   println("nodeId:" + nodeId);
   analyze( db, {Mode: 4, NodeID: nodeId} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   println("check primary node result analyze mode set 4 success!");
   
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   println("check slave node result analyze mode set 4 success!");
   
   //清空统计信息
   analyze( db, {Mode:5, NodeID: nodeId} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 5 success!");
   
   //再次更新统计信息
   var mcvValues = [{a:8000},{a:sameValues},{a:9001}];
   var fracs = [500,100,9400];
   updateIndexStateInfo( db, COMMCSNAME, clName, "a", mcvValues, fracs );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after update index stat but no analyze success!");
   
   //再次清空缓存
   analyze( db, {Mode:5, NodeID: nodeId} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after update index stat and analyze mode set 5 success!");
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );
   db1.close();
   db2.close();
  
 }
 main()
/************************************
*@Description: 指定3个参数组合收集统计信息
*@author:      zhaoyu
*@createdate:  2017.11.14
*@testlinkCase:seqDB-11639/seqDB-11641
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
   
   var clName = COMMCLNAME + "_11639";
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
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db.getCS(COMMCSNAME).getCL(clName);
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db.getCS(COMMCSNAME).getCL(clName);
	
	//检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check result before analyze success!");

   //指定cl+index+group执行统计
   var groupName = getSrcGroup( COMMCSNAME, clName );
   analyze( db, {Collection: COMMCSNAME + "." + clName, Index: "a", GroupName: groupName} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after analyze set cl+index+group success!");
   
   //清空统计信息
   analyze( db, {Mode:3, Collection: COMMCSNAME + "." + clName} );
   
   //指定cs+group+node执行统计
   var primaryNode = db.getRG(groupName).getMaster();
   var nodeId = parseInt(primaryNode.getNodeDetail().split(":")[0]);
   println("nodeId:" + nodeId);
   analyze( db, {CollectionSpace: COMMCSNAME, GroupName: groupName, NodeID: nodeId} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after analyze set cs+group+node success!");
   
   //清空统计信息
   analyze( db, {Mode:3, Collection: COMMCSNAME + "." + clName} );
   
   //指定cl+group+node执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName, GroupName: groupName, NodeID: nodeId} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after analyze set cl+group+node success!");
   
   //清空统计信息
   analyze( db, {Mode:3, Collection: COMMCSNAME + "." + clName} );
   
   //指定cl+index+node执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName, Index: "a", NodeID: nodeId} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after analyze set cl+index+node success!");
   
   //清空统计信息
   analyze( db, {Mode:3, Collection: COMMCSNAME + "." + clName} );
   
   //指定cl+index+group+node执行统计,seqDB-11641
   analyze( db, {Collection: COMMCSNAME + "." + clName, Index: "a", GroupName: groupName, NodeID: nodeId} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after analyze set cl+index+node success!");
   
   //清空统计信息
   analyze( db, {Mode:3, Collection: COMMCSNAME + "." + clName} );
   
   //指定不支持的组合参数执行统计
   analyzeInvalidPara( db, {CollectionSpace: COMMCSNAME, Collection: COMMCSNAME + "." + clName, Index: "a"} );
   println("check result after analyze set cs+cl+index success!");
   analyzeInvalidPara( db, {CollectionSpace: COMMCSNAME, Collection: COMMCSNAME + "." + clName, GroupName: groupName} );
   println("check result after analyze set cs+cl+group success!");
   analyzeInvalidPara( db, {CollectionSpace: COMMCSNAME, Collection: COMMCSNAME + "." + clName, NodeID: nodeId} );
   println("check result after analyze set cs+cl+node success!");
   analyzeInvalidPara( db, {CollectionSpace: COMMCSNAME, Index: "a", GroupName: groupName} );
   println("check result after analyze set cs+index+group success!");
   analyzeInvalidPara( db, {CollectionSpace: COMMCSNAME, Index: "a", NodeID: nodeId} );
   println("check result after analyze set cs+index+node success!");
   analyzeInvalidPara( db, {Index: "a", GroupName: groupName, NodeID: nodeId} );
   println("check result after analyze set index+group+node success!");
   
   //seqDB-11641
   analyzeInvalidPara( db, {CollectionSpace: COMMCSNAME, Collection: COMMCSNAME + "." + clName, Index: "a", GroupName: groupName} );
   println("check result after analyze set cs+cl+index+group success!");
   analyzeInvalidPara( db, {CollectionSpace: COMMCSNAME, Collection: COMMCSNAME + "." + clName, Index: "a", NodeID: nodeId} );
   println("check result after analyze set cs+cl+index+node success!");
   analyzeInvalidPara( db, {CollectionSpace: COMMCSNAME, Collection: COMMCSNAME + "." + clName, GroupName: groupName, NodeID: nodeId} );
   println("check result after analyze set cs+cl+group+node success!");
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );
  
 }
 main()
 
function analyzeInvalidPara( db, options )
{
   try
   {
      db.analyze(options);
      throw "NEED_ERR";
   }catch(e)
   {
      if(e !== -6 )
      {
         throw e;
      }
   }
}
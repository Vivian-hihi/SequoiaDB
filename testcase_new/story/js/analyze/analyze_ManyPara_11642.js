/************************************
*@Description: 指定所有参数生成默认统计信息并手工修改统计信息再清空
*@author:      zhaoyu
*@createdate:  2017.11.15
*@testlinkCase:seqDB-11642
**************************************/
function main()
{
   //独立模式及1组模式不执行该用例
   try
	{
	   //判断独立模式
	   if( true == commIsStandalone( db ) )
      {
         println( "run mode is standalone" );
         return;
      } 
          
      //判断1组模式
      var allGroupName = getGroupName( db );       
      if( 1 === allGroupName.length )
      {
         println("only one group");
         return ;
      }
   }
   catch( e )
   {
      throw e;
   }
   
   var clName = COMMCLNAME + "_11642";
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
   
   //收集统计信息
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

   //指定cl+index+group+node生成默认统计信息
   var groupName = getSrcGroup( COMMCSNAME, clName );
   var primaryNode = db.getRG(groupName).getMaster();
   var nodeId = parseInt(primaryNode.getNodeDetail().split(":")[0]);
   println("nodeId:" + nodeId);
   analyze( db, {Mode:3, Collection: COMMCSNAME + "." + clName, Index: "a", GroupName: groupName, NodeID: nodeId} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, false );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("mode=3 and option set cl+index+group+node success!");
   
   //手工修改主备节点统计信息
   var mcvValues = [{a:8000},{a:sameValues},{a:9001}];
   var fracs = [500,9000,500];
   updateIndexStateInfo( db, COMMCSNAME, clName, "a", mcvValues, fracs );
   
   //统计信息加载至缓存
   analyze( db, {Mode:4, Collection: COMMCSNAME + "." + clName, Index: "a", GroupName: groupName, NodeID: nodeId} );
   
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
   
   println("mode=4 and option set cl+index+group+node success!");
   
   //清空缓存
   analyze( db, {Mode:5, Collection: COMMCSNAME + "." + clName, Index: "a", GroupName: groupName, NodeID: nodeId} );
   
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
   
   println("mode=5 and option set cl+index+group+node success!");
   
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
   
   println("option set cs+cl+group+node after update index stat but no analyze success!");
   
   //再次清空缓存
   analyze( db, {Mode:5, Collection: COMMCSNAME + "." + clName, Index: "a", GroupName: groupName, NodeID: nodeId} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("option set cl+index+group+node after update index stat and analyze mode set 5 success!");
   
   //指定不支持的参数
   analyzeInvalidPara( db, {CollectionSpace: COMMCSNAME, Collection: COMMCSNAME + "." + clName, Index: "a", GroupName: groupName} );
   println("check result after analyze set cs+cl+index+ group success!");
   analyzeInvalidPara( db, {CollectionSpace: COMMCSNAME, Collection: COMMCSNAME + "." + clName, Index: "a", NodeID: nodeId} );
   println("check result after analyze set cs+cl+index+node success!");
   analyzeInvalidPara( db, {CollectionSpace: COMMCSNAME, Collection: COMMCSNAME + "." + clName, GroupName: groupName, NodeID: nodeId} );
   println("check result after analyze set cs+cl+group+node success!");
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );
   db1.close();
   db2.close();
  
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
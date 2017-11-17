/************************************
*@Description: 指定node收集统计信息
*@author:      zhaoyu
*@createdate:  2017.11.13
*@testlinkCase:seqDB-11621
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
      
   var clName = COMMCLNAME + "_11621";
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

   //指定主节点执行统计
   var groupName = getSrcGroup( COMMCSNAME, clName );
   var primaryNode = db.getRG(groupName).getMaster();
   var nodeId = parseInt(primaryNode.getNodeDetail().split(":")[0]);
   println("nodeId:" + nodeId);
   analyze( db, {NodeID:nodeId} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after analyze primary node success!");
   
   //指定备节点执行统计
   var slaveNode = db.getRG(groupName).getSlave();
   var nodeId = parseInt(slaveNode.getNodeDetail().split(":")[0]);
   println("nodeId:" + nodeId);
   try
   {
      db.analyze({NodeID:nodeId});
      throw "NEED_AN_ERR";
   }catch(e)
   {
      if(e !== -264)
      {
         throw e;
      }
   }
   println("check result after analyze slave node success!");
   
   //指定cata节点执行统计
   var cataNode = db.getRG("SYSCatalogGroup").getMaster();
   var nodeId = parseInt(cataNode.getNodeDetail().split(":")[0]);
   println("nodeId:" + nodeId);
   analyze( db, {NodeID:nodeId} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   println("check result after analyze cata node success!");
   
   //指定不存在节点执行统计
   try
   {
      db.analyze({NodeID:2233});
      throw "NEED_AN_ERR";
   }catch(e)
   {
      if(e !== -155)
      {
         throw e;
      }
   }
   println("check result after analyze not exists node success!");
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );
   db1.close();
   db2.close();
  
 }
 main()
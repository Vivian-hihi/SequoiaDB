/************************************
*@Description: 指定切分表收集统计信息
*@author:      zhaoyu
*@createdate:  2017.11.11
*@testlinkCase:seqDB-11612
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
   
   var clName = COMMCLNAME + "_11612";
   var insertNum = 4000;
	var sameValues = 9000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建切分表
   var clOption = {ShardingKey: {"a": 1}, ShardingType: "range"};
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, clOption);
   
   //执行切分
   var groups = ClSplitOneTimes( COMMCSNAME, clName, {a:2000}, {a:4000} ); 
   
   //创建索引
   commCreateIndex( dbcl, "a0", {a0:1});
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues );
	
	//获取主备节点
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db.getCS(COMMCSNAME).getCL(clName);
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db.getCS(COMMCSNAME).getCL(clName);
	
	//检查统计信息
   checkStat( db, COMMCSNAME, clName, "$shard", false, false );
   checkStat( db, COMMCSNAME, clName, "a0", false, false );
   
   //使用shard索引，检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"$shard", GroupName:groups[0].GroupName, ReturnNum:insertNum}];
   
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check $shard index query explain before analyze success!");
	
	//使用普通索引，检查主备节点访问计划
   var findConf = {a0:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a0", GroupName:groups[1].GroupName, ReturnNum:0},
                      {ScanType:"ixscan", IndexName:"a0", GroupName:groups[0].GroupName, ReturnNum:insertNum}];
   
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check common index query explain before analyze success!");

   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "$shard", true, true );
   checkStat( db, COMMCSNAME, clName, "a0", true, true );
   
   //使用shard索引，检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", GroupName:groups[0].GroupName, ReturnNum:insertNum}];
   
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check $shard index query explain after analyze success!");
	
	//使用普通索引，检查主备节点访问计划
   var findConf = {a0:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a0", GroupName:groups[1].GroupName, ReturnNum:0},
                      {ScanType:"tbscan", IndexName:"", GroupName:groups[0].GroupName, ReturnNum:insertNum}];
   
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check common index query explain after analyze success!");
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );

 }
 main()
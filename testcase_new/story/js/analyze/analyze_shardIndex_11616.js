/************************************
*@Description: 指定shard索引收集统计信息
*@author:      zhaoyu
*@createdate:  2017.11.11
*@testlinkCase:seqDB-11616
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
   var clName = COMMCLNAME + "_11616";
   var insertNum = 4000;
	var sameValues1 = 4000;
	var sameValues2 = 2000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建切分表
   var clOption = {ShardingKey: {"a": 1}, ShardingType: "range"};
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, clOption);
   
   //执行切分
   var groups = ClSplitOneTimes( COMMCSNAME, clName, {a:2000}, {a:4000} ); 
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues1 );
	insertSameDatas( dbcl, insertNum, sameValues2 );
	
	//检查统计信息
   checkStat( db, COMMCSNAME, clName, "$shard", false, false );
   
   //使用shard索引，检查主备节点访问计划
   var findConf = {a:{$in:[sameValues1,sameValues2]}};
   var actExplains = getSplitExplain( dbcl, findConf);
   var expExplains = [{ScanType:"ixscan", IndexName:"$shard", GroupName:groups[0].GroupName, ReturnNum:insertNum},
                      {ScanType:"ixscan", IndexName:"$shard", GroupName:groups[1].GroupName, ReturnNum:insertNum + 1}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( actExplains, expExplains );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( actExplains, expExplains );
	
	println("check $shard index query explain before analyze success!");
	
   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "$shard", true, true );
   
   //使用shard索引，检查主备节点访问计划
   var findConf = {a:{$in:[sameValues1,sameValues2]}};
   var actExplains = getSplitExplain( dbcl, findConf); 
   var expExplains = [{ScanType:"tbscan", IndexName:"", GroupName:groups[0].GroupName, ReturnNum:insertNum},
                      {ScanType:"tbscan", IndexName:"", GroupName:groups[1].GroupName, ReturnNum:insertNum + 1}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( actExplains, expExplains );
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( actExplains, expExplains );
	
	println("check $shard index query explain after analyze success!");
	
   //清理环境
   //commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );
   
 }
 main()
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
   
   var csName = COMMCSNAME + "_hash_11612";
   var clName = COMMCLNAME + "_11612";
   var insertNum = 4000;
	var sameValues = 9000;
	var domainName = "mydomain";
   
   //清理环境
   commDropCS( db, csName);
   try
   {
      db.dropDomain(domainName);
   }catch(e)
   {
      if(e !== -214)
      {
         throw e;
      }
   }
   
   //获取组
   var groups = getGroupName( db );
   
   //创建域
   db.createDomain(domainName,[groups[0][0],groups[1][0]],{AutoSplit:true});
   
   //创建切分表
   csOption = {Domain: domainName}
   commCreateCS( db, csName, false, "", csOption );
   var clOption = {ShardingKey: {"a": 1}, ShardingType: "hash"};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption);
   
   //创建索引
   commCreateIndex( dbcl, "a0", {a0:1});
   
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
   checkStat( db, csName, clName, "$shard", false, false );
   checkStat( db, csName, clName, "a0", false, false );
   
   //使用shard索引，检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"$shard", GroupName:groups[1][0], ReturnNum:insertNum}];
   
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check $shard index query explain before analyze success!");
	
	//使用普通索引，检查主备节点访问计划
   var findConf = {a0:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a0", GroupName:groups[0][0], ReturnNum:0},
                      {ScanType:"ixscan", IndexName:"a0", GroupName:groups[1][0], ReturnNum:insertNum}];
   
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check common index query explain before analyze success!");

   //执行统计
   analyze( db, {Collection: csName + "." + clName} );
   
   //检查统计信息
   checkStat( db, csName, clName, "$shard", true, true );
   checkStat( db, csName, clName, "a0", true, true );
   
   //使用shard索引，检查主备节点访问计划
   var findConf = {a:sameValues};
   var expExplains = [{ScanType:"tbscan", IndexName:"", GroupName:groups[1][0], ReturnNum:insertNum}];
   
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check $shard index query explain after analyze success!");
	
	//使用普通索引，检查主备节点访问计划
   var findConf = {a0:sameValues};
   var expExplains = [{ScanType:"ixscan", IndexName:"a0", GroupName:groups[0][0], ReturnNum:0},
                      {ScanType:"tbscan", IndexName:"", GroupName:groups[1][0], ReturnNum:insertNum}];
   
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
	println("check common index query explain after analyze success!");
   
   //清理环境
   commDropCS( db, csName);
   db.dropDomain(domainName);
   db1.close();
   db2.close();

 }
 main()
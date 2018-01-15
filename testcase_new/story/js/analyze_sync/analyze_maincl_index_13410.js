/************************************
*@Description: 主子表中指定索引收集统计信息
*@author:      zhaoyu
*@createdate:  2017.12.22
*@testlinkCase:seqDB-13410
**************************************/
var maincsName = COMMCSNAME + "_maincs_13410";
var subcsName1 = COMMCSNAME + "_subcs_13410_1";
var subcsName2 = COMMCSNAME + "_subcs_13410_2";
var mainclName = COMMCLNAME + "_maincl_13410";
var subclName1 = COMMCLNAME + "_subcl_13410_1";
var subclName2 = COMMCLNAME + "_subcl_13410_2";
var subclName3 = COMMCLNAME + "_subcl_13410_3";
var subclName4 = COMMCLNAME + "_subcl_13410_4";
var subclName5 = COMMCLNAME + "_subcl_13410_5";
var subclName6 = COMMCLNAME + "_subcl_13410_6";
var mainclFullName = maincsName + "." + mainclName;
var subclFullName1 = maincsName + "." + subclName1;
var subclFullName2 = maincsName + "." + subclName2;
var subclFullName3 = subcsName1 + "." + subclName3;
var subclFullName4 = subcsName1 + "." + subclName4;
var subclFullName5 = subcsName2 + "." + subclName5;
var subclFullName6 = subcsName2 + "." + subclName6;

var maincl;

var subclGroups1;
var subclGroups2;
var subclGroups3;
var subclGroups4;
var subclGroups5;
var subclGroups6;

var insertDiffNum = 24000;
var insertSameNum = 2000;

var db1;
var db2;
var dbclPrimary;
var dbclSlave;

var subcl2;
var subcl5;


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
   
   //清理环境
   commDropCS( db, subcsName1, true, "drop subcs before test" );
   commDropCS( db, subcsName2, true, "drop subcs before test" );
   commDropCS( db, maincsName, true, "drop maincs before test" );
   
   //创建主表cl
   var mainclOption = {IsMainCL: true, ShardingKey: {"a": 1}, ShardingType: "range"};
   maincl = commCreateCLByOption( db, maincsName, mainclName, mainclOption);
   
   //创建子表cl
   var subclOption1 = {ShardingKey: {"a0": 1}, ShardingType: "range"};
   commCreateCLByOption( db, maincsName, subclName1, subclOption1);
   var subclOption2 = {ShardingKey: {"a0": 1}, ShardingType: "hash"};
   subcl2 = commCreateCLByOption( db, maincsName, subclName2, subclOption2);
   commCreateCLByOption( db, subcsName1, subclName3, subclOption1);
   commCreateCLByOption( db, subcsName1, subclName4, subclOption2);
   subcl5 = commCreateCLByOption( db, subcsName2, subclName5, subclOption1);
   commCreateCLByOption( db, subcsName2, subclName6, subclOption2);
   
   //子表切分
   subclGroups1 = ClSplitOneTimes( maincsName, subclName1, {a0:2000}, {a0:4000} ); 
   subclGroups2 = ClSplitOneTimes( maincsName, subclName2, 50, null );
   subclGroups3 = ClSplitOneTimes( subcsName1, subclName3, {a0:10000}, {a0:12000} ); 
   subclGroups4 = ClSplitOneTimes( subcsName1, subclName4, 50, null );
   subclGroups5 = ClSplitOneTimes( subcsName2, subclName5, {a0:18000}, {a0:20000} ); 
   subclGroups6 = ClSplitOneTimes( subcsName2, subclName6, 50, null );
   
   //attach cl
   maincl.attachCL( subclFullName2, {LowBound: {a:4000}, UpBound:{a:8000}} );
   maincl.attachCL( subclFullName3, {LowBound: {a:8000}, UpBound:{a:12000}} );
   maincl.attachCL( subclFullName5, {LowBound: {a:16000}, UpBound:{a:20000}} );
   maincl.attachCL( subclFullName6, {LowBound: {a:20000}, UpBound:{a:24000}} );
   maincl.attachCL( subclFullName4, {LowBound: {a:12000}, UpBound:{a:16000}} );
   maincl.attachCL( subclFullName1, {LowBound: {a:0}, UpBound:{a:4000}} );
   
   //创建索引
   commCreateIndex( maincl, "a1", {a1:1});
   
   //插入记录
	insertDiffDatas( maincl, insertDiffNum );
	insertSameDatas( maincl, insertSameNum, 0 );
	insertSameDatas( maincl, insertSameNum, 10000 );
	
	//获取主备节点
   db1 = new Sdb(db);
   db1.setSessionAttr( { PreferedInstance: "m" } );
   dbclPrimary = db1.getCS(maincsName).getCL(mainclName);
   db2 = new Sdb(db);
   db2.setSessionAttr( { PreferedInstance: "s" } );
   dbclSlave = db2.getCS(maincsName).getCL(mainclName);
   
   //检查统计
   checkStat( db, maincsName, subclName1, "$shard", false, false );
   checkStat( db, maincsName, subclName2, "$shard", false, false );
   checkStat( db, subcsName1, subclName3, "$shard", false, false );
   checkStat( db, subcsName1, subclName4, "$shard", false, false );
   checkStat( db, subcsName2, subclName5, "$shard", false, false );
   checkStat( db, subcsName2, subclName6, "$shard", false, false );
   
   checkStat( db, maincsName, subclName1, "a1", false, false );
   checkStat( db, maincsName, subclName2, "a1", false, false );
   checkStat( db, subcsName1, subclName3, "a1", false, false );
   checkStat( db, subcsName1, subclName4, "a1", false, false );
   checkStat( db, subcsName2, subclName5, "a1", false, false );
   checkStat( db, subcsName2, subclName6, "a1", false, false );
   
   //主备节点上检查访问计划
   checkExplainBeforeAnalyze();
   println("---check all explain before analyze success");
   
   //指定主表cl收集shard索引的统计信息
   analyze( db, {Collection: mainclFullName, Index:"$shard"} );
   
   //检查统计
   checkStat( db, maincsName, subclName1, "$shard", true, true );
   checkStat( db, maincsName, subclName2, "$shard", true, true );
   checkStat( db, subcsName1, subclName3, "$shard", true, true );
   checkStat( db, subcsName1, subclName4, "$shard", true, true );
   checkStat( db, subcsName2, subclName5, "$shard", true, true );
   checkStat( db, subcsName2, subclName6, "$shard", true, true );
   
   checkStat( db, maincsName, subclName1, "a1", true, false );
   checkStat( db, maincsName, subclName2, "a1", true, false );
   checkStat( db, subcsName1, subclName3, "a1", true, false );
   checkStat( db, subcsName1, subclName4, "a1", true, false );
   checkStat( db, subcsName2, subclName5, "a1", true, false );
   checkStat( db, subcsName2, subclName6, "a1", true, false );
   
   //主备节点上检查访问计划
   checkExplainAfterAnalyzeMainclShardIndex();
   println("---check all explain after anlyze shard index success");
   
   //生成默认统计信息
   analyze( db, {Collection: mainclFullName, Mode:3} );
   
   //检查统计
   checkStat( db, maincsName, subclName1, "$shard", true, false );
   checkStat( db, maincsName, subclName2, "$shard", true, false );
   checkStat( db, subcsName1, subclName3, "$shard", true, false );
   checkStat( db, subcsName1, subclName4, "$shard", true, false );
   checkStat( db, subcsName2, subclName5, "$shard", true, false );
   checkStat( db, subcsName2, subclName6, "$shard", true, false );
   
   checkStat( db, maincsName, subclName1, "a1", true, false );
   checkStat( db, maincsName, subclName2, "a1", true, false );
   checkStat( db, subcsName1, subclName3, "a1", true, false );
   checkStat( db, subcsName1, subclName4, "a1", true, false );
   checkStat( db, subcsName2, subclName5, "a1", true, false );
   checkStat( db, subcsName2, subclName6, "a1", true, false );
   
   //主备节点上检查访问计划
   checkExplainBeforeAnalyze();
   println("---check all explain before analyze success");
   
   //指定主表cl收集普通索引的统计信息
   analyze( db, {Collection: mainclFullName, Index:"a1"} );
   
   //检查统计
   checkStat( db, maincsName, subclName1, "$shard", true, false );
   checkStat( db, maincsName, subclName2, "$shard", true, false );
   checkStat( db, subcsName1, subclName3, "$shard", true, false );
   checkStat( db, subcsName1, subclName4, "$shard", true, false );
   checkStat( db, subcsName2, subclName5, "$shard", true, false );
   checkStat( db, subcsName2, subclName6, "$shard", true, false );
   
   checkStat( db, maincsName, subclName1, "a1", true, true );
   checkStat( db, maincsName, subclName2, "a1", true, true );
   checkStat( db, subcsName1, subclName3, "a1", true, true );
   checkStat( db, subcsName1, subclName4, "a1", true, true );
   checkStat( db, subcsName2, subclName5, "a1", true, true );
   checkStat( db, subcsName2, subclName6, "a1", true, true );
   
   //主备节点上检查访问计划
   checkExplainAfterAnalyzeMainclCommonIndex();
   println("---check all explain after anlyze common index success");
   
   //删除主表cs、其他cs下的子表中的shard索引
   commDropIndex( subcl2, "a1" );
   commDropIndex( subcl5, "a1" );
   
   //检查统计
   checkStat( db, maincsName, subclName1, "a1", true, true );
   checkStat( db, maincsName, subclName2, "a1", true, false );
   checkStat( db, subcsName1, subclName3, "a1", true, true );
   checkStat( db, subcsName1, subclName4, "a1", true, true );
   checkStat( db, subcsName2, subclName5, "a1", true, false );
   checkStat( db, subcsName2, subclName6, "a1", true, true );
   
   //主备节点上检查访问计划
   checkExplainAfterDropSubclCommIndex();
   
   //指定主表cl的shard索引执行统计
   analyze( db, {Collection: mainclFullName, Index:"a1"} );
   
   //主备节点上检查访问计划
   checkExplainAfterDropSubclCommIndex();
   
   //指定不存在的索引执行统计
   try{
      db.analyze({Collection:mainclFullName,Index:"d"});
      throw "NEED_ERR";
   }catch(e){
      if(-264 !== e){
         throw e;
      }
   }
   
   //清理环境
   //commDropCS( db, subcsName1 );
   //commDropCS( db, subcsName2 );
   //commDropCS( db, maincsName );
   db1.close();
   db2.close();
   
 }
 main()
 
 function checkExplainBeforeAnalyze()
{
   //子表分区键查询
   var findConf = {a0:{$in:[0,10000]}};
   var expExplains = [{GroupName:subclGroups1[0].GroupName, Name: subclFullName1, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups2[0].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups3[0].GroupName, Name: subclFullName3, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups4[0].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups5[0].GroupName, Name: subclFullName5, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups6[0].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups2[1].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups3[1].GroupName, Name: subclFullName3, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups4[1].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups6[1].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0}];
  
   var actExplains = getMainclExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getMainclExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check subcl key before analyze success!");
   
   //索引键查询
   var findConf = {a1:{$in:[0,10000]}};
   var expExplains = [{GroupName:subclGroups1[0].GroupName, Name: subclFullName1, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups1[1].GroupName, Name: subclFullName1, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups2[0].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups2[1].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups3[0].GroupName, Name: subclFullName3, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups3[1].GroupName, Name: subclFullName3, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups4[0].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups4[1].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups5[0].GroupName, Name: subclFullName5, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups5[1].GroupName, Name: subclFullName5, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups6[0].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups6[1].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0}];
   
   var actExplains = getMainclExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getMainclExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check index before analyze success!");
}


function checkExplainAfterAnalyzeMainclShardIndex()
{
   //子表分区键查询
   var findConf = {a0:{$in:[0,10000]}};
   var expExplains = [{GroupName:subclGroups1[0].GroupName, Name: subclFullName1, 
                       ScanType:"tbscan", IndexName:"", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups2[0].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups3[0].GroupName, Name: subclFullName3, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups4[0].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups5[0].GroupName, Name: subclFullName5, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups6[0].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups2[1].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups3[1].GroupName, Name: subclFullName3, 
                       ScanType:"tbscan", IndexName:"", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups4[1].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups6[1].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0}];
  
   var actExplains = getMainclExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getMainclExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check subcl key after analyze shard index success!");
   
   //索引键查询
   var findConf = {a1:{$in:[0,10000]}};
   var expExplains = [{GroupName:subclGroups1[0].GroupName, Name: subclFullName1, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups1[1].GroupName, Name: subclFullName1, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups2[0].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups2[1].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups3[0].GroupName, Name: subclFullName3, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups3[1].GroupName, Name: subclFullName3, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups4[0].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups4[1].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups5[0].GroupName, Name: subclFullName5, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups5[1].GroupName, Name: subclFullName5, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups6[0].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups6[1].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0}];
   
   var actExplains = getMainclExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getMainclExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check index after analyze shard index success!");
}

function checkExplainAfterAnalyzeMainclCommonIndex()
{
   //子表分区键查询
   var findConf = {a0:{$in:[0,10000]}};
   var expExplains = [{GroupName:subclGroups1[0].GroupName, Name: subclFullName1, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups2[0].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups3[0].GroupName, Name: subclFullName3, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups4[0].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups5[0].GroupName, Name: subclFullName5, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups6[0].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups2[1].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups3[1].GroupName, Name: subclFullName3, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups4[1].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0},
                      {GroupName:subclGroups6[1].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"$shard", ReturnNum:0}];
  
   var actExplains = getMainclExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getMainclExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check subcl key after analyze common index success!");
   
   //索引键查询
   var findConf = {a1:{$in:[0,10000]}};
   var expExplains = [{GroupName:subclGroups1[0].GroupName, Name: subclFullName1, 
                       ScanType:"tbscan", IndexName:"", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups1[1].GroupName, Name: subclFullName1, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups2[0].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups2[1].GroupName, Name: subclFullName2, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups3[0].GroupName, Name: subclFullName3, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups3[1].GroupName, Name: subclFullName3, 
                       ScanType:"tbscan", IndexName:"", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups4[0].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups4[1].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups5[0].GroupName, Name: subclFullName5, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups5[1].GroupName, Name: subclFullName5, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups6[0].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups6[1].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0}];
   
   var actExplains = getMainclExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getMainclExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check index after analyze common index success!");
}

function checkExplainAfterDropSubclCommIndex()
{
   //索引键查询
   var findConf = {a1:{$in:[0,10000]}};
   var expExplains = [{GroupName:subclGroups1[0].GroupName, Name: subclFullName1, 
                       ScanType:"tbscan", IndexName:"", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups1[1].GroupName, Name: subclFullName1, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups2[0].GroupName, Name: subclFullName2, 
                       ScanType:"tbscan", IndexName:"", ReturnNum:0},
                      {GroupName:subclGroups2[1].GroupName, Name: subclFullName2, 
                       ScanType:"tbscan", IndexName:"", ReturnNum:0},
                      {GroupName:subclGroups3[0].GroupName, Name: subclFullName3, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups3[1].GroupName, Name: subclFullName3, 
                       ScanType:"tbscan", IndexName:"", ReturnNum:insertSameNum + 1},
                      {GroupName:subclGroups4[0].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups4[1].GroupName, Name: subclFullName4, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups5[0].GroupName, Name: subclFullName5, 
                       ScanType:"tbscan", IndexName:"", ReturnNum:0},
                      {GroupName:subclGroups5[1].GroupName, Name: subclFullName5, 
                       ScanType:"tbscan", IndexName:"", ReturnNum:0},
                      {GroupName:subclGroups6[0].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0},
                      {GroupName:subclGroups6[1].GroupName, Name: subclFullName6, 
                       ScanType:"ixscan", IndexName:"a1", ReturnNum:0}];
   
   var actExplains = getMainclExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getMainclExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check index after drop subcl common index success!");
}

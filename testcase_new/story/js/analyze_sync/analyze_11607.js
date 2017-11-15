/************************************
*@Description: 收集全局统计信息  
*@author:      liuxiaoxuan
*@createdate:  2017.11.10
*@testlinkCase: seqDB-11607
**************************************/
function main()
{
   if (commIsStandalone(db))
   {
      println("skip standalone environment");
      return ;
   }
                                               	
   if (2 > commGetGroupsNum(db))
   {
      println("group less than 2");
      return ;
   }
                                                                	
   var csName1 = COMMCSNAME + "11607_1";
   var csName2 = COMMCSNAME + "11607_2";
   commDropCS( db, csName1, true, "drop CS in the beginning" );
   commDropCS( db, csName2, true, "drop CS in the beginning" );
                                                                    	
   commCreateCS( db, csName1, false, "" );
   commCreateCS( db, csName2, false, "" );
                                                                 	
   //create CLs
   var dbcl = new Array();
   
   var clName1 = COMMCLNAME + "11607_1";
   var dbCommCL1 = commCreateCLByOption( db, csName1, clName1 );
   var dbCommCL2 = commCreateCLByOption( db, csName2, clName1 );
                                                                          
   var clOption2 = {ShardingKey:{a:1}, ShardingType:"hash"};
   var clName2 = COMMCLNAME + "11607_2";
   var dbHashCL1 = commCreateCLByOption( db, csName1, clName2, clOption2, true );
   var dbHashCL2 = commCreateCLByOption( db, csName2, clName2, clOption2, true );
   
   var clOption3 = {ShardingKey:{a:1}, ShardingType:"range"};
   var clName3 = COMMCLNAME + "11607_3";
   var dbRangCL1 = commCreateCLByOption( db, csName1, clName3, clOption3, true );
   var dbRangCL2 = commCreateCLByOption( db, csName2, clName3, clOption3, true );
      
   //create index
   commCreateIndex( dbCommCL1, "a", {a : 1}, false );
   commCreateIndex( dbCommCL2, "a", {a : 1}, false );
   commCreateIndex( dbHashCL1, "b", {b : 1}, false );
   commCreateIndex( dbHashCL2, "b", {b : 1}, false );
   commCreateIndex( dbRangCL1, "b", {b : 1}, false );
   commCreateIndex( dbRangCL2, "b", {b : 1}, false );

   //get master/slave datanode
   var db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "m"} );
   var dbCommCLPrimary1 = db1.getCS(csName1).getCL(clName1);
   var dbCommCLPrimary2 = db1.getCS(csName2).getCL(clName1);
   var dbHashCLPrimary1 = db1.getCS(csName1).getCL(clName2);
   var dbHashCLPrimary2 = db1.getCS(csName2).getCL(clName2);
   var dbRangCLPrimary1 = db1.getCS(csName1).getCL(clName3);
   var dbRangCLPrimary2 = db1.getCS(csName2).getCL(clName3);
   
   db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "s"} );
   var dbCommCLSlave1 = db1.getCS(csName1).getCL(clName1);
   var dbCommCLSlave2 = db1.getCS(csName2).getCL(clName1);
   var dbHashCLSlave1 = db1.getCS(csName1).getCL(clName2);
   var dbHashCLSlave2 = db1.getCS(csName2).getCL(clName2);
   var dbRangCLSlave1 = db1.getCS(csName1).getCL(clName3);
   var dbRangCLSlave2 = db1.getCS(csName2).getCL(clName3);
                                                      	 
   //insert datas
   var insertNums = 3000;
   var sameValues = 9000;
                                           	
   insertDiffDatas( dbCommCL1, insertNums );
   insertSameDatas( dbCommCL1, insertNums, sameValues );
   
   insertDiffDatas( dbCommCL2, insertNums );
   insertSameDatas( dbCommCL2, insertNums, sameValues );
   
   insertDiffDatas( dbHashCL1, insertNums );
   insertSameDatas( dbHashCL1, insertNums, sameValues );
   
   insertDiffDatas( dbHashCL2, insertNums );
   insertSameDatas( dbHashCL2, insertNums, sameValues );
   
   insertDiffDatas( dbRangCL1, insertNums );
   insertSameDatas( dbRangCL1, insertNums, sameValues );
   
   insertDiffDatas( dbRangCL2, insertNums );
   insertSameDatas( dbRangCL2, insertNums, sameValues );
                                                              	
   //split shard cls
   ClSplitOneTimes( csName1, clName2, 50 );
   ClSplitOneTimes( csName2, clName2, 50 );
   ClSplitOneTimes( csName1, clName3, 50 );
   ClSplitOneTimes( csName2, clName3, 50 );
                                                       	
   //check before invoke analyze
   checkStat( db, csName1, clName1, "a", false, false );	
   checkStat( db, csName2, clName1, "a", false, false );	
   checkStat( db, csName1, clName2, "$shard", false, false );
   checkStat( db, csName2, clName2, "$shard", false, false );
   checkStat( db, csName1, clName3, "$shard", false, false );
   checkStat( db, csName2, clName3, "$shard", false, false );
                                                                  	
   //check the query explain of master/slave nodes 
   var groupsHash1 = getSplitGroups( csName1, clName2, 1 );
   var groupsHash2 = getSplitGroups( csName2, clName2, 1 );
   
   var groupsRang1 = getSplitGroups( csName1, clName3, 1 );
   var groupsRang2 = getSplitGroups( csName2, clName3, 1 );
                                                         	
   var srcHashGroupName1 = groupsHash1[0].GroupName;
   var destHashGroupName1 = groupsHash1[1].GroupName;
   var srcHashGroupName2 = groupsHash2[0].GroupName;
   var destHashGroupName2 = groupsHash2[1].GroupName;
   
   var srcRangGroupName1 = groupsRang1[0].GroupName;
   var destRangGroupName1 = groupsRang1[1].GroupName;
   var srcRangGroupName2 = groupsRang2[0].GroupName;
   var destRangGroupName2 = groupsRang2[1].GroupName;
                                                      	
   var findConf = {a : 9000};
   var expCommExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   var expHashExplains1 = [{ScanType:"ixscan", IndexName:"$shard",GroupName :srcHashGroupName1, ReturnNum:insertNums}];
   var expHashExplains2 = [{ScanType:"ixscan", IndexName:"$shard",GroupName :srcHashGroupName2, ReturnNum:insertNums}];
   var expRangExplains1 = [{ScanType:"ixscan", IndexName:"$shard",GroupName :destRangGroupName1, ReturnNum:insertNums}];
   var expRangExplains2 = [{ScanType:"ixscan", IndexName:"$shard",GroupName :destRangGroupName2, ReturnNum:insertNums}];

   //check primary
   var actCommExplains1 = getCommonExplain( dbCommCLPrimary1, findConf); 
   var actCommExplains2 = getCommonExplain( dbCommCLPrimary2, findConf); 
   var actHashExplains1 = getSplitExplain( dbHashCLPrimary1, findConf); 
   var actHashExplains2 = getSplitExplain( dbHashCLPrimary2, findConf); 
   var actRangExplains1 = getSplitExplain( dbRangCLPrimary1, findConf); 
   var actRangExplains2 = getSplitExplain( dbRangCLPrimary2, findConf); 

   checkExplain( actCommExplains1, expCommExplains );
   checkExplain( actCommExplains2, expCommExplains );
   checkExplain( actHashExplains1, expHashExplains1 );                                                    	
   checkExplain( actHashExplains2, expHashExplains2 );
   checkExplain( actRangExplains1, expRangExplains1 );
   checkExplain( actRangExplains2, expRangExplains2 );
   
   //check slave
   var actCommExplains1 = getCommonExplain( dbCommCLSlave1, findConf); 
   var actCommExplains2 = getCommonExplain( dbCommCLSlave2, findConf); 
   var actHashExplains1 = getSplitExplain( dbHashCLSlave1, findConf); 
   var actHashExplains2 = getSplitExplain( dbHashCLSlave2, findConf); 
   var actRangExplains1 = getSplitExplain( dbRangCLSlave1, findConf); 
   var actRangExplains2 = getSplitExplain( dbRangCLSlave2, findConf); 

   checkExplain( actCommExplains1, expCommExplains );
   checkExplain( actCommExplains2, expCommExplains );
   checkExplain( actHashExplains1, expHashExplains1 );                                                    	
   checkExplain( actHashExplains2, expHashExplains2 );
   checkExplain( actRangExplains1, expRangExplains1 );
   checkExplain( actRangExplains2, expRangExplains2 );
                                                              	
   println("check result before analyze success!");
                   
   //analyze
   analyze( db );
                                                             
   //check after analyze
   checkStat( db, csName1, clName1, "a", true, true );	
   checkStat( db, csName2, clName1, "a", true, true );	
   checkStat( db, csName1, clName2, "$shard", true, true );
   checkStat( db, csName2, clName2, "$shard", true, true );
   checkStat( db, csName1, clName3, "$shard", true, true );
   checkStat( db, csName2, clName3, "$shard", true, true );
                                                        
   //check the query explain of master/slave nodes 
   var findConf = {a : 9000};
   var expCommExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   var expHashExplains1 = [{ScanType:"tbscan", IndexName:"",GroupName :srcHashGroupName1, ReturnNum:insertNums}];
   var expHashExplains2 = [{ScanType:"tbscan", IndexName:"",GroupName :srcHashGroupName2, ReturnNum:insertNums}];
   var expRangExplains1 = [{ScanType:"tbscan", IndexName:"",GroupName :destRangGroupName1, ReturnNum:insertNums}];
   var expRangExplains2 = [{ScanType:"tbscan", IndexName:"",GroupName :destRangGroupName2, ReturnNum:insertNums}];

   //check primary
   var actCommExplains1 = getCommonExplain( dbCommCLPrimary1, findConf); 
   var actCommExplains2 = getCommonExplain( dbCommCLPrimary2, findConf); 
   var actHashExplains1 = getSplitExplain( dbHashCLPrimary1, findConf); 
   var actHashExplains2 = getSplitExplain( dbHashCLPrimary2, findConf); 
   var actRangExplains1 = getSplitExplain( dbRangCLPrimary1, findConf); 
   var actRangExplains2 = getSplitExplain( dbRangCLPrimary2, findConf); 

   checkExplain( actCommExplains1, expCommExplains );
   checkExplain( actCommExplains2, expCommExplains );
   checkExplain( actHashExplains1, expHashExplains1 );                                                    	
   checkExplain( actHashExplains2, expHashExplains2 );
   checkExplain( actRangExplains1, expRangExplains1 );
   checkExplain( actRangExplains2, expRangExplains2 );
   
   //check slave
   var actCommExplains1 = getCommonExplain( dbCommCLSlave1, findConf); 
   var actCommExplains2 = getCommonExplain( dbCommCLSlave2, findConf); 
   var actHashExplains1 = getSplitExplain( dbHashCLSlave1, findConf); 
   var actHashExplains2 = getSplitExplain( dbHashCLSlave2, findConf); 
   var actRangExplains1 = getSplitExplain( dbRangCLSlave1, findConf); 
   var actRangExplains2 = getSplitExplain( dbRangCLSlave2, findConf); 

   checkExplain( actCommExplains1, expCommExplains );
   checkExplain( actCommExplains2, expCommExplains );
   checkExplain( actHashExplains1, expHashExplains1 );                                                    	
   checkExplain( actHashExplains2, expHashExplains2 );
   checkExplain( actRangExplains1, expRangExplains1 );
   checkExplain( actRangExplains2, expRangExplains2 );
                                                                  
   println("check result after analyze success!");
     
   db1.close();     
   commDropCS( db, csName1, true, "drop CS in the end" );
   commDropCS( db, csName2, true, "drop CS in the end" );
}
main();

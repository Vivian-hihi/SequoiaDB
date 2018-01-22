/************************************
*@Description: alter cl更新统计信息  
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-12981
**************************************/
function main()
{
   if(commIsStandalone(db))
   {
      println("skip standalone environment");
      return;
   }
   
   if(1 >= commGetGroupsNum(db))
   {
      println("less than two groups");
      return;
   }
	
   var csName = COMMCSNAME + "12981";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                                      	
   commCreateCS( db, csName, false, "" );
		
   //create CLs
   var clName1 = COMMCLNAME + "12981_1";
   var dbcl1 = commCreateCL( db, csName, clName1 );

   var clName2 = COMMCLNAME + "12981_2";
   var dbcl2 = commCreateCL( db, csName, clName2 );
   
   var clFullName1 = csName + "." + clName1;
   var clFullName2 = csName + "." + clName2;
                                                                                                                  	
   //get master/slave datanode
   var db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "m"} );
   var dbclPrimary1 = db1.getCS(csName).getCL(clName1);
   var dbclPrimary2 = db1.getCS(csName).getCL(clName2);
   
   db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "s"} );
   var dbclSlave1 = db1.getCS(csName).getCL(clName1);
   var dbclSlave2 = db1.getCS(csName).getCL(clName2);
                                                                 	
   //create index
   commCreateIndex( dbcl1, "b", {b : 1}, false );
   commCreateIndex( dbcl2, "c", {c : 1}, false );
                                                  	
   //insert data
   var insertNums = 5000;
   var sameValues = 9000;
   insertDiffDatas( dbcl1, insertNums );
   insertSameDatas( dbcl1, insertNums, sameValues );
   insertDiffDatas( dbcl2, insertNums );
   insertSameDatas( dbcl2, insertNums, sameValues );
	
   //check before invoke analyze
   checkStat( db, csName, clName1, "$shard", false, false );
   checkStat( db, csName, clName2, "$shard", false, false );
   checkStat( db, csName, clName1, "b", false, false );
   checkStat( db, csName, clName2, "c", false, false );
                                                        	
   //query from primary/slave node  
   var findConf1 = {a0 : 9000};
   var findConf2 = {a1 : 9000};
   var findConf3 = {b : {'$gte': 9000}};
   var findConf4 = {c : {'$gte': 'test9000'}};
   
   query(dbclPrimary1, findConf1);
	query(dbclPrimary1, findConf2);
   query(dbclPrimary2, findConf1);
	query(dbclPrimary2, findConf2);
	query(dbclSlave1, findConf1);
	query(dbclSlave1, findConf2);
   query(dbclSlave2, findConf1);
	query(dbclSlave2, findConf2);
	
	//check out snapshot access plans
	var accessFindOption1 = { Collection: clFullName1 };
   var accessFindOption2 = { Collection: clFullName2 };
	
   var actAccessPlans1 = getCommonAccessPlans(db, accessFindOption1);
   var actAccessPlans2 = getCommonAccessPlans(db, accessFindOption2);
   
   var expAccessPlans1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
                          {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   var expAccessPlans2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
                          {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                      
   checkSnapShotAccessPlans(clFullName1, expAccessPlans1, actAccessPlans1);
   checkSnapShotAccessPlans(clFullName2, expAccessPlans2, actAccessPlans2);
                                                                                                                                 	
   //invoke analyze
   var options = {CollectionSpace: csName};
   analyze( db, options );
                                                        	
   //check after analyze before alter
   checkStat( db, csName, clName1, "$shard", true, false );
   checkStat( db, csName, clName2, "$shard", true, false );
   checkStat( db, csName, clName1, "b", true, true );
   checkStat( db, csName, clName2, "c", true, true );
   
   //check out snapshot access plans
	var accessFindOption1 = { Collection: clFullName1 };
   var accessFindOption2 = { Collection: clFullName2 };
   
   var actAccessPlans1 = getCommonAccessPlans(db, accessFindOption1);
   var actAccessPlans2 = getCommonAccessPlans(db, accessFindOption2);
   var expAccessPlans = [];
   
   checkSnapShotAccessPlans(clFullName1, expAccessPlans, actAccessPlans1);
   checkSnapShotAccessPlans(clFullName2, expAccessPlans, actAccessPlans2);
   
   //query from primary/slave node  
   var findConf1 = {a0 : 9000};
   var findConf2 = {a1 : 9000};
   var findConf3 = {b : {'$gte': 9000}};
   var findConf4 = {c : {'$gte': 'test9000'}};

   query(dbclPrimary1, findConf1);
	query(dbclPrimary1, findConf2);
   query(dbclPrimary2, findConf1);
	query(dbclPrimary2, findConf2);
	query(dbclSlave1, findConf1);
	query(dbclSlave1, findConf2);
   query(dbclSlave2, findConf1);
	query(dbclSlave2, findConf2);
   
   //check out snapshot access plans
	var accessFindOption1 = { Collection: clFullName1 };
   var accessFindOption2 = { Collection: clFullName2 };
	
   var actAccessPlans1 = getCommonAccessPlans(db, accessFindOption1);
   var actAccessPlans2 = getCommonAccessPlans(db, accessFindOption2);
   
   var expAccessPlans1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
                          {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   var expAccessPlans2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
                          {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                      
   checkSnapShotAccessPlans(clFullName1, expAccessPlans1, actAccessPlans1);
   checkSnapShotAccessPlans(clFullName2, expAccessPlans2, actAccessPlans2);

   println("check result success before alter!");       
                                                 
   //alter CLs
   var alterOption1 = {ShardingKey: {a0 :1}, ShardingType: 'hash'};
   var alterOption2 = {ShardingKey: {a1 :1}, ShardingType: 'range'};
   alterCL( dbcl1, alterOption1 );
   alterCL( dbcl2, alterOption2 );
         
   //check alter before analyze       
   checkStat( db, csName, clName1, "$shard", true, false );
   checkStat( db, csName, clName2, "$shard", true, false );
   checkStat( db, csName, clName1, "b", true, true );
   checkStat( db, csName, clName2, "c", true, true );
   
   //check out snapshot access plans
	var accessFindOption1 = { Collection: clFullName1 };
   var accessFindOption2 = { Collection: clFullName2 };
   
   var actAccessPlans1 = getCommonAccessPlans(db, accessFindOption1);
   var actAccessPlans2 = getCommonAccessPlans(db, accessFindOption2);
   var expAccessPlans = [];
   
   checkSnapShotAccessPlans(clFullName1, expAccessPlans, actAccessPlans1);
   checkSnapShotAccessPlans(clFullName2, expAccessPlans, actAccessPlans2);
   
   //query from primary/slave node  
   var findConf1 = {a0 : 9000};
   var findConf2 = {a1 : 9000};
   var findConf3 = {b : {'$gte': 9000}};
   var findConf4 = {c : {'$gte': 'test9000'}};
   
   query(dbclPrimary1, findConf1);
	query(dbclPrimary1, findConf2);
   query(dbclPrimary2, findConf1);
	query(dbclPrimary2, findConf2);
	query(dbclSlave1, findConf1);
	query(dbclSlave1, findConf2);
   query(dbclSlave2, findConf1);
	query(dbclSlave2, findConf2);

   //check out snapshot access plans
	var accessFindOption1 = { Collection: clFullName1 };
   var accessFindOption2 = { Collection: clFullName2 };
	
   var actAccessPlans1 = getCommonAccessPlans(db, accessFindOption1);
   var actAccessPlans2 = getCommonAccessPlans(db, accessFindOption2);
   
   var expAccessPlans1 = [{ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
                          {ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   var expAccessPlans2 = [{ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums},
                          {ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertNums},
	                       {ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                      
   checkSnapShotAccessPlans(clFullName1, expAccessPlans1, actAccessPlans1);
   checkSnapShotAccessPlans(clFullName2, expAccessPlans2, actAccessPlans2);
                                                                      
   println("check result success after alter before analyze!");  
   
   //split CLs
   var group1 = ClSplitOneTimes( csName, clName1, 50 );
   var group2 = ClSplitOneTimes( csName, clName2, 50 );
   
   var srcGroupName1 = group1[0].GroupName;
   var destGroupName1 = group1[1].GroupName;
   var srcGroupName2 = group2[0].GroupName;
   var destGroupName2 = group2[1].GroupName;
   
   //check out snapshot access plans
	var accessFindOption1 = { Collection: clFullName1 };
   var accessFindOption2 = { Collection: clFullName2 };
	
   var actAccessPlans1 = getSplitAccessPlans(db, accessFindOption1);
   var actAccessPlans2 = getSplitAccessPlans(db, accessFindOption2);
   
   var expAccessPlans1 = [{GroupName: srcGroupName1, ScanType:"ixscan", IndexName:"$shard"},
	                       {GroupName: srcGroupName1, ScanType:"tbscan", IndexName:""},
                          {GroupName: srcGroupName1, ScanType:"ixscan", IndexName:"$shard"},
	                       {GroupName: srcGroupName1, ScanType:"tbscan", IndexName:""}];
   var expAccessPlans2 = [{GroupName: srcGroupName2, ScanType:"ixscan", IndexName:"$shard"},
	                       {GroupName: srcGroupName2, ScanType:"tbscan", IndexName:""},
                          {GroupName: srcGroupName2, ScanType:"ixscan", IndexName:"$shard"},
	                       {GroupName: srcGroupName2, ScanType:"tbscan", IndexName:""} ];
                             
   checkSnapShotAccessPlans(clFullName1, expAccessPlans1, actAccessPlans1);
   checkSnapShotAccessPlans(clFullName2, expAccessPlans2, actAccessPlans2);
                       	
   //query from primary/slave node  
   var findConf1 = {a0 : 9000};
   var findConf2 = {a1 : 9000};
   var findConf3 = {b : {'$gte': 9000}};
   var findConf4 = {c : {'$gte': 'test9000'}};
   
   query(dbclPrimary1, findConf1);
	query(dbclPrimary1, findConf2);
   query(dbclPrimary2, findConf1);
	query(dbclPrimary2, findConf2);
	query(dbclSlave1, findConf1);
	query(dbclSlave1, findConf2);
   query(dbclSlave2, findConf1);
	query(dbclSlave2, findConf2);

   //check out snapshot access plans
	var accessFindOption1 = { Collection: clFullName1 };
   var accessFindOption2 = { Collection: clFullName2 };
	
   var actAccessPlans1 = getSplitAccessPlans(db, accessFindOption1);
   var actAccessPlans2 = getSplitAccessPlans(db, accessFindOption2);
   
   var expAccessPlans1 = [{GroupName: srcGroupName1, ScanType:"ixscan", IndexName:"$shard"},
	                       {GroupName: srcGroupName1, ScanType:"tbscan", IndexName:""},
                          {GroupName: destGroupName1, ScanType:"tbscan", IndexName:""},
                          {GroupName: srcGroupName1, ScanType:"ixscan", IndexName:"$shard"},
	                       {GroupName: srcGroupName1, ScanType:"tbscan", IndexName:""},
                          {GroupName: destGroupName1, ScanType:"tbscan", IndexName:""}];    
   var expAccessPlans2 = [{GroupName: srcGroupName2, ScanType:"ixscan", IndexName:"$shard"},
	                       {GroupName: srcGroupName2, ScanType:"tbscan", IndexName:""},
                          {GroupName: destGroupName2, ScanType:"ixscan", IndexName:"$shard"},
                          {GroupName: destGroupName2, ScanType:"tbscan", IndexName:""},
                          {GroupName: srcGroupName2, ScanType:"ixscan", IndexName:"$shard"},
                          {GroupName: srcGroupName2, ScanType:"tbscan", IndexName:""},
	                       {GroupName: destGroupName2, ScanType:"ixscan", IndexName:"$shard"},
                          {GroupName: destGroupName2, ScanType:"tbscan", IndexName:""}];
                              
   checkSnapShotAccessPlans(clFullName1, expAccessPlans1, actAccessPlans1);
   checkSnapShotAccessPlans(clFullName2, expAccessPlans2, actAccessPlans2);

   println("check result success after split before analyze!"); 
                        
   //check alter after analyze
   var options = {CollectionSpace: csName};
   analyze( db, options );

   
   checkStat( db, csName, clName1, "$shard", true, true );
   checkStat( db, csName, clName2, "$shard", true, true );
   checkStat( db, csName, clName1, "b", true, true );
   checkStat( db, csName, clName2, "c", true, true );
   
   //check out snapshot access plans
	var accessFindOption1 = { Collection: clFullName1 };
   var accessFindOption2 = { Collection: clFullName2 };
   
   var actAccessPlans1 = getSplitAccessPlans(db, accessFindOption1);
   var actAccessPlans2 = getSplitAccessPlans(db, accessFindOption2);
   var expAccessPlans = [];
   
   checkSnapShotAccessPlans(clFullName1, expAccessPlans, actAccessPlans1);
   checkSnapShotAccessPlans(clFullName2, expAccessPlans, actAccessPlans2);
   
   //query from primary/slave node  
   var findConf1 = {a0 : 9000};
   var findConf2 = {a1 : 9000};
   var findConf3 = {b : {'$gte': 9000}};
   var findConf4 = {c : {'$gte': 'test9000'}};
   
   query(dbclPrimary1, findConf1);
	query(dbclPrimary1, findConf2);
   query(dbclPrimary2, findConf1);
	query(dbclPrimary2, findConf2);
	query(dbclSlave1, findConf1);
	query(dbclSlave1, findConf2);
   query(dbclSlave2, findConf1);
	query(dbclSlave2, findConf2);

   //check out snapshot access plans
	var accessFindOption1 = { Collection: clFullName1 };
   var accessFindOption2 = { Collection: clFullName2 };
	
   var actAccessPlans1 = getSplitAccessPlans(db, accessFindOption1);
   var actAccessPlans2 = getSplitAccessPlans(db, accessFindOption2);
   
   var expAccessPlans1 = [{GroupName: srcGroupName1, ScanType:"tbscan", IndexName:""},
	                       {GroupName: srcGroupName1, ScanType:"tbscan", IndexName:""},
                          {GroupName: destGroupName1, ScanType:"tbscan", IndexName:""},
                          {GroupName: srcGroupName1, ScanType:"tbscan", IndexName:""},
	                       {GroupName: srcGroupName1, ScanType:"tbscan", IndexName:""},
                          {GroupName: destGroupName1, ScanType:"tbscan", IndexName:""}];
   var expAccessPlans2 = [{GroupName: srcGroupName2, ScanType:"tbscan", IndexName:""},
                          {GroupName: destGroupName2, ScanType:"tbscan", IndexName:""},
                          {GroupName: destGroupName2, ScanType:"tbscan", IndexName:""},
                          {GroupName: srcGroupName2, ScanType:"tbscan", IndexName:""},
                          {GroupName: destGroupName2, ScanType:"tbscan", IndexName:""},
                          {GroupName: destGroupName2, ScanType:"tbscan", IndexName:""}];
             
   checkSnapShotAccessPlans(clFullName1, expAccessPlans1, actAccessPlans1);
   checkSnapShotAccessPlans(clFullName2, expAccessPlans2, actAccessPlans2);
                                                                           	
   println("check result success after alter after split after analyze!"); 

   db1.close();        
   commDropCS( db, csName, true, "drop CS in the end" );
}

function alterCL( dbcl, alterOption )
{
   try
   {
      dbcl.alter( alterOption );
   }
   catch(e)
   {
      throw buildException("alter CL", e, "alter", "alter success", e);
   }
}

main();

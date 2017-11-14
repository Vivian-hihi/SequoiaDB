/************************************
*@Description: 指定切分表所在cs收集统计信息 
*@author:      liuxiaoxuan
*@createdate:  2017.11.08
*@testlinkCase: seqDB-11609
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
                                                                	
   var csName = COMMCSNAME + "11609";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                                    	
   commCreateCS( db, csName, false, "" );
                                                                 	
   //create CLs
   var clOption1 = {ShardingKey:{a:1}, ShardingType:"hash"};
   var clName1 = COMMCLNAME + "11609_1";
   var dbcl1 = commCreateCLByOption( db, csName, clName1, clOption1, true );
                                                                          
   var clOption2 = {ShardingKey:{a:1}, ShardingType:"range"};
   var clName2 = COMMCLNAME + "11609_2";
   var dbcl2 = commCreateCLByOption( db, csName, clName2, clOption2, true );
                                                                      
   //get master/slave datanode
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary1 = db.getCS(csName).getCL(clName1);
   var dbclPrimary2 = db.getCS(csName).getCL(clName2);
                                                             	
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave1 = db.getCS(csName).getCL(clName1);
   var dbclSlave2 = db.getCS(csName).getCL(clName2);	
                                                       	 
   //insert datas
   var insertNums = 3000;
   var sameValues = 9000;
                                           	
   insertDiffDatas( dbcl1, insertNums );
   insertSameDatas( dbcl1, insertNums, sameValues );
   insertDiffDatas( dbcl2, insertNums );
   insertSameDatas( dbcl2, insertNums, sameValues );
                                                              	
   //split cl
   ClSplitOneTimes( csName, clName1, 50 );
   ClSplitOneTimes( csName, clName2, 50 );
                                                       	
   //check before invoke analyze
   checkStat( db, csName, clName1, "$shard", false, false );
   checkStat( db, csName, clName1, "bc", false, false );	
   checkStat( db, csName, clName2, "$shard", false, false );
   checkStat( db, csName, clName2, "bc", false, false );
                                                                  	
   //check the query explain of master/slave nodes 
   var groups1 = getSplitGroups( csName, clName1, 1 );
   var groups2 = getSplitGroups( csName, clName2, 1 );
                                                         	
   var srcGroupName1 = groups1[0].GroupName;
   var destGroupName1 = groups1[1].GroupName;
   var srcGroupName2 = groups2[0].GroupName;
   var destGroupName2 = groups2[1].GroupName;
                                                      	
   var findConf = {a : 9000};
   var expExplains1 = [{ScanType:"ixscan", IndexName:"$shard",
                        GroupName :srcGroupName1, ReturnNum:insertNums}];
   var expExplains2 = [{ScanType:"ixscan", IndexName:"$shard",
                        GroupName :destGroupName2, ReturnNum:insertNums}];
	
   var actExplains1 = getSplitExplain( dbclPrimary1, findConf);
   checkExplain( actExplains1, expExplains1 );
   var actExplains2 = getSplitExplain( dbclPrimary2, findConf);
   checkExplain( actExplains2, expExplains2 );
                                                             	
   var actExplains1 = getSplitExplain( dbclSlave1, findConf);
   checkExplain( actExplains1, expExplains1 );
   var actExplains2 = getSplitExplain( dbclSlave2, findConf);
   checkExplain( actExplains2, expExplains2 );
                                                              	
   println("check result before analyze success!");
                   
   //invoke analyze
   var options = {CollectionSpace: csName};
   analyze( db, options );
                                                             
   //check after analyze
   checkStat( db, csName, clName1, "$shard", true, true );
   checkStat( db, csName, clName2, "$shard", true, true );
                                                        
   //check the query explain of master/slave nodes 
   var findConf = {a : 9000};
   var expExplains1 = [{ScanType:"tbscan", IndexName:"",
                        GroupName :srcGroupName1, ReturnNum:insertNums}];
   var expExplains2 = [{ScanType:"tbscan", IndexName:"",
                        GroupName :destGroupName2, ReturnNum:insertNums}];
                                                                          	
   var actExplains1 = getSplitExplain( dbclPrimary1, findConf);
   checkExplain( actExplains1, expExplains1 );
   var actExplains2 = getSplitExplain( dbclPrimary2, findConf);
   checkExplain( actExplains2, expExplains2 );
                                                                           	
   var actExplains1 = getSplitExplain( dbclSlave1, findConf);
   checkExplain( actExplains1, expExplains1 );
   var actExplains2 = getSplitExplain( dbclSlave2, findConf);
   checkExplain( actExplains2, expExplains2 );
                                                                  
   println("check result after analyze success!");
                                                              	
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

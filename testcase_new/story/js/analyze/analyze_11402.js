/************************************
*@Description: split cl更新统计信息  
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11402
**************************************/
function main()
{
   if(commIsStandalone(db))
   {
      println("skip standalone environment");
      return;
   }
                                                                                             	
   if(2 > commGetGroupsNum(db))
   {
      println("group nums less than 2");
      return;
   }
                                                                                               	
   var csName = COMMCSNAME + "11402";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                                                                	
   commCreateCS( db, csName, false, "" );
                                                                                                  		
   //create cl
   var groups = commGetGroups(db);
   var srcGroupName = groups[0][0].GroupName;
   var destGroupName = groups[1][0].GroupName;
	                                                                                           
   var clOption = {ShardingKey:{a:1}, ShardingType:"hash", Group : srcGroupName};
   var clName = COMMCLNAME + "11402";
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true );
	                                                                                               
   //get master/slave datanode
   var db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "m"} );
   var dbclPrimary = db1.getCS(csName).getCL(clName);
   
   db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "s"} );
   var dbclSlave = db1.getCS(csName).getCL(clName);
                                                                                               	
   //create index
   commCreateIndex( dbcl, "b", {b : 1}, false )
	 
   //insert
   var insertNums = 3000;
   var sameValues = 9000;
   insertDiffDatas( dbcl, insertNums );
   insertSameDatas( dbcl, insertNums, sameValues );
                                                                                                 	                                                                                            
   //check before invoke analyze
   checkStat( db, csName, clName, "$shard", false, false );
   checkStat( db, csName, clName, "b", false, false );

   //check the query explain of master/slave nodes 
   var findConf1 = {a : 9000};
   var findConf2 = {b : 9000};
   var expExplains1 = [{ScanType:"ixscan", IndexName:"$shard", ReturnNum:insertNums}];
   var expExplains2 = [{ScanType:"ixscan", IndexName:"b", ReturnNum:insertNums}];
                                                                    
   var actExplains1 = getCommonExplain( dbclPrimary, findConf1);
   var actExplains2 = getCommonExplain( dbclPrimary, findConf2);
   checkExplain( actExplains1, expExplains1 );
   checkExplain( actExplains2, expExplains2 );
   
   var actExplains1 = getCommonExplain( dbclSlave, findConf1);
   var actExplains2 = getCommonExplain( dbclSlave, findConf2);
   checkExplain( actExplains1, expExplains1 );
   checkExplain( actExplains2, expExplains2 );
	
   println("check result before analyze success!");
	
   //invoke analyze
   var options = {Collection: csName + "." + clName};
   analyze( db, options );
                                                                                               
   //check after analyze
   checkStat( db, csName, clName, "$shard", true, true );
   checkStat( db, csName, clName, "b", true, true );
                                                                
   //check the query explain of master/slave nodes 
   var findConf1 = {a : 9000};
   var findConf2 = {b : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                                                                    
   var actExplains1 = getCommonExplain( dbclPrimary, findConf1);
   var actExplains2 = getCommonExplain( dbclPrimary, findConf2);
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );
   
   var actExplains1 = getCommonExplain( dbclSlave, findConf1);
   var actExplains2 = getCommonExplain( dbclSlave, findConf2);
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );
   
   println("check analyze result success before split!");
                                                                                  	
   //split cl
   splitCL(dbcl, srcGroupName, destGroupName);

   var findConf1 = {a : 9000};
   var findConf2 = {b : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                                                                    
   var actExplains1 = getCommonExplain( dbclPrimary, findConf1);
   var actExplains2 = getCommonExplain( dbclPrimary, findConf2);
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );
   
   var actExplains1 = getCommonExplain( dbclSlave, findConf1);
   var actExplains2 = getCommonExplain( dbclSlave, findConf2);
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );
   
   println("check analyze result success after split before analyze!");
	
   //check split after analyze
   var options = {Collection: csName + "." + clName};
   analyze( db, options );
   
   checkStat( db, csName, clName, "$shard", true, true );
   checkStat( db, csName, clName, "b", true, true );
     
   var findConf1 = {a : 9000};
   var findConf2 = {b : 9000};
   var expExplains1 = [{ScanType:"tbscan", IndexName:"", GroupName:srcGroupName, ReturnNum:insertNums}];
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", GroupName:srcGroupName, ReturnNum:insertNums},
                       {ScanType:"ixscan", IndexName:"b", GroupName:destGroupName, ReturnNum:0}];
                                                           
   var actExplains1 = getSplitExplain( dbclPrimary, findConf1);
   var actExplains2 = getSplitExplain( dbclPrimary, findConf2);
   checkExplain( actExplains1, expExplains1 );
   checkExplain( actExplains2, expExplains2 );
   
   var actExplains1 = getSplitExplain( dbclSlave, findConf1);
   var actExplains2 = getSplitExplain( dbclSlave, findConf2);
   checkExplain( actExplains1, expExplains1 );
   checkExplain( actExplains2, expExplains2 );
                                      
   println("check analyze result success after split after analyze!");
	
   db1.close();
   commDropCS( db, csName, true, "drop CS in the end" );
}

function splitCL( dbcl, srcGroup, destGroup )
{
   try
   {
      dbcl.split( srcGroup, destGroup, 50 );
   }
   catch(e)
   {
      throw buildException("split CL", e, "split", "split success", e);
   }
}
main();

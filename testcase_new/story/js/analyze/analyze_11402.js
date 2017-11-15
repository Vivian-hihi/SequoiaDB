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
   commCreateIndex( dbcl, "ac", {a : 1 , c : 1}, false )
	 
   //insert
   var insertNums = 3000;
   var sameValues = 9000;
   insertDiffDatas( dbcl, insertNums );
   insertSameDatas( dbcl, insertNums, sameValues );
                                                                                                 	                                                                                            
   //check before invoke analyze
   checkStat( db, csName, clName, "ac", false, false );
                                                                                                   	
   //check the query explain of master/slave nodes 
   var findConf = {a : 9000, c : "test9000"};
   var expExplains = [{ScanType:"ixscan", IndexName:"ac", ReturnNum:insertNums}];
                                                                                         
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
   println("check result before analyze success!");
	
   //invoke analyze
   var options = {Collection: csName + "." + clName};
   analyze( db, options );
                                                                                               
   //check after analyze
   checkStat( db, csName, clName, "ac", true, true );
                                                                
   //check the query explain of master/slave nodes 
   var findConf = {a : 9000, c : "test9000"};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                                                                                      
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                                                                              
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                                                  	
   //split cl
   splitCL(dbcl, srcGroupName, destGroupName);
	
   //check after split
   println("after split!");
   var findConf = {a : 9000, c : "test9000"};
   var expExplains = [{ScanType:"tbscan", IndexName:"",GroupName: srcGroupName, ReturnNum:insertNums}];
                                                                                        	
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
   println("check result after analyze success!");
	
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

/************************************
*@Description:   指定shard索引生成默认统计信息并修手工改统计信息再清空
*@author:      liuxiaoxuan
*@createdate:  2017.11.13
*@testlinkCase: seqDB-11628
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
                                    
   var csName = COMMCSNAME + "11628";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                              	
   commCreateCS( db, csName, false, "" );
                                                           		
   //create cl	
   var clOption = {ShardingKey:{a:1}, ShardingType:"hash"};
   var clName = COMMCLNAME + "11628";
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true );
                                                                         	
   //get master/slave datanode
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db.getCS(csName).getCL(clName);
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db.getCS(csName).getCL(clName);
                                                                   	
   //insert datas
   var insertNums = 3000;
   var sameValues = 9000;
   insertDiffDatas( dbcl, insertNums );
   insertSameDatas( dbcl, insertNums, sameValues );
                                                             	
   //split cl
   ClSplitOneTimes( csName, clName, 50 );
                         
   //analyze 
   var cl_full_name = csName + "." + clName;
   var options = { Collection : cl_full_name };
   analyze( db, options );
                         
   //check before analyze success
   checkStat( db, csName, clName, "$shard", true, true );
                                                         	
   var srcGroupName = getSrcGroup( csName, clName );
                                                       	
   //check the query explain before analyze
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", 
                       GroupName:srcGroupName, ReturnNum:insertNums}];
                                                                     
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                                           
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                                     	
   println("check result before default analyze !");
                                                         	
   //invoke analyze
   var options = { Mode : 3, Collection : cl_full_name, Index: "$shard"};
   analyze( db, options );
                                                                           	
   //check after analyze success
   checkStat( db, csName, clName, "$shard", true, false );
                                                                 	
   //check the query explain after analyze
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$shard", 
                       GroupName:srcGroupName, ReturnNum:insertNums}];
                                     
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
          
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                         
   println("check result after default analyze !");
                                                 	
   // modify SYSSTAT info
   var mcvValues = [{a: 0},{a: 1},{a:9000}];
   var fracs = [500,500,9000];
   updateIndexStateInfo( db, csName, clName, "$shard", mcvValues, fracs );
                                                              	
   // reload analyze again
   var options = { Mode : 4, Collection : cl_full_name};
   analyze( db, options );
                                                 	
   //check the query explain after analyze
   checkStat( db, csName, clName, "$shard", true, true );
                                        
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", 
                       GroupName:srcGroupName, ReturnNum:insertNums}];
   
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                 	
   println("check result after reload analyze success!");
                                              	
   //truncate invalidate
   var options = { Mode : 5, Collection : cl_full_name };
   analyze( db, options );
                              	
   //check analyze after truncate invalidate
   checkStat( db, csName, clName, "$shard", true, true );
                           
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", 
                       GroupName:srcGroupName, ReturnNum:insertNums}];
                            		
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                	
   println("check result after truncate invalidate!");
   
   // modify SYSSTAT info again
   var mcvValues = [{a: 0},{a: 1},{a:9000}];
   var fracs = [500,500,500];
   updateIndexStateInfo( db, csName, clName, "$shard", mcvValues, fracs );                                                    
                                                 	
   //check the query explain after analyze
   checkStat( db, csName, clName, "$shard", true, true );
                                        
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", 
                       GroupName:srcGroupName, ReturnNum:insertNums}];
   
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                 	
   println("check result after second modify SYS info success!");
   
   //truncate invalidate
   var options = { Mode : 5, Collection : cl_full_name };
   analyze( db, options );
                              	
   //check analyze after truncate invalidate
   checkStat( db, csName, clName, "$shard", true, true );
                           
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$shard", 
                       GroupName:srcGroupName, ReturnNum:insertNums}];
                            		
   var actExplains = getSplitExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                
   var actExplains = getSplitExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                	
   println("check result after second truncate invalidate!");
                                             
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

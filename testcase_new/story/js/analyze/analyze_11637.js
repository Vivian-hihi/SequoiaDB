/************************************
*@Description:  指定2个参数组合收集统计信息  
*@author:      liuxiaoxuan
*@createdate:  2017.11.11
*@testlinkCase: seqDB-11637
**************************************/
function main()
{	
   if (commIsStandalone(db))
   {
      println("skip standalone environment");
      return ;
   }
                                         	
   var csName = COMMCSNAME + "11637";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                            	
   commCreateCS( db, csName, false, "" );
                                            		
   //create cl	
   var clName = COMMCLNAME + "11637";
   var dbcl = commCreateCL( db, csName, clName );
                                 	
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
                                                    	
   //create index
   commCreateIndex( dbcl, "a", {a : 1}, false );
                                                 	
   //get Group and Node info
   var groupName = commGetCLGroups( db, csName + "." + clName );
   var groupDetail = commGetGroups( db, false, groupName[0] );
                                 	
   var groupId = groupDetail[0][0].GroupID;
   var priNodeId = groupDetail[0][0].PrimaryNode;
                                             	
   //check before analyze success
   checkStat( db, csName, clName, "a", false, false );
                                          	
   //check the query explain before analyze
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
                                                              
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                                            
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                        	
   println("check result before analyze success!");
                                                     	
   //invoke analyze
   var options = [{ CollectionSpace : csName, GroupID : groupId },
                  { CollectionSpace : csName, NodeID : priNodeId },
                  { Collection : csName + "." + clName, GroupName : groupName[0]},
                  { Collection : csName + "." + clName, NodeID : priNodeId},
                  { GroupID : groupId, NodeID : priNodeId }];
                                                               
   for(var i in options)
   {
      analyze( db, options[i] );
   }
                                 	
   //check after analyze success
   checkStat( db, csName, clName, "a", true, true );
                                                 	
   //check the query explain after analyze
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                   	
   println("check result after analyze success!");
                                         	
   //check invalid analyze, cs+cl, cs+index, index+group, index+node
   var options = [{ CollectionSpace : csName, Collection : csName + "." + clName},
                  { CollectionSpace : csName, Index : "a"},
                  { Index : "a", GroupID : groupId },
                  { Index : "a", GroupName : groupName[0] },
                  { Index : "a", NodeID : priNodeId }];
                                                   
   for(var i in options)
   {
      checkAnalyzeInvalidResult( options[i] );
   }
                                                            	
   commDropCS( db, csName, true, "drop CS in the end" );
}

function checkAnalyzeInvalidResult( options )
{  
   try
   {
      db.analyze( options );
      throw "NEED ANALYZE FAILED";
   }
   catch(e)
   {
      if( -6 !== e )
      {
         throw buildException("check analyze", e, "analyze", "analyze success", e);
      }
   }
}

main();

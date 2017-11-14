/************************************
*@Description: loadCS/unloadCS后更新统计信息 
*@author:      liuxiaoxuan
*@createdate:  2017.11.11
*@testlinkCase: seqDB-11649 
**************************************/
function main()
{	
   var csName = COMMCSNAME + "11649";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                             	
   commCreateCS( db, csName, false, "" );
                                             		
   //create cl	
   var clName = COMMCLNAME + "11649";
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
   var options = { CollectionSpace : csName };
   analyze( db, options );
                              	
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
                                            	
   //unload cs
   var dataDB = getDataDB(csName, clName);
   dataDB.unloadCS(csName);
                               	
   //check after unload
   checkStat( db, csName, clName, "a", true, true );
                                    	
   println("check unload result success!");
                                       	
   //load cs
   dataDB.loadCS(csName);
                          	
   //check after load
   checkStat( db, csName, clName, "a", true, true );
                                   	
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                                                
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                       	
   println("check load result success!");
                                   	
   commDropCS( db, csName, true, "drop CS in the end" );
}

function getDataDB( csName, clName )
{  
   if (commIsStandalone(db))
   {  
      return db;
   }
	
   var groupName = commGetCLGroups( db, csName + "." + clName );
   var groupDetail = commGetGroups( db, false, groupName[0] );
                     
   var priNodePos = groupDetail[0][0].PrimaryPos;
                      	
   var hostName = groupDetail[0][priNodePos].HostName;
   var svcName = groupDetail[0][priNodePos].svcname;
   return new Sdb(hostName, svcName);
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

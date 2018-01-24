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
   
   var clFullName = csName + "." + clName;
                                                  	
   //get master/slave datanode          
   var db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "m"} );
   var dbclPrimary = db1.getCS(csName).getCL(clName);
   
   db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "s"} );
   var dbclSlave = db1.getCS(csName).getCL(clName);
                                        	
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
                     
   //check out snapshot access plans
	var accessFindOption = { Collection: clFullName };
   var actAccessPlans = getCommonAccessPlans(db, accessFindOption);
   var expAccessPlans = [];
                     
   checkSnapShotAccessPlans(clFullName, expAccessPlans, actAccessPlans);
                     
   //check the query explain after analyze
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                                 
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                           
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   //query
   query(dbclPrimary, findConf, null, null, insertNums);
   query(dbclSlave, findConf, null, null, insertNums);
   
   //check out snapshot access plans
	var accessFindOption = { Collection: clFullName };
   var actAccessPlans = getCommonAccessPlans(db, accessFindOption);
   var expAccessPlans = [{ScanType:"tbscan", IndexName:""},
	                      {ScanType:"tbscan", IndexName:""}];
                     
   checkSnapShotAccessPlans(clFullName, expAccessPlans, actAccessPlans);
                                             	
   println("check result after analyze success!");
                                            	
   //unload cs
   var primaryDB = getPrimaryNodeDB(csName, clName);
   primaryDB.unloadCS(csName);
                               	
   //check after unload
   checkStat( db, csName, clName, "a", true, true );
   
   //check out snapshot access plans
	var accessFindOption = { Collection: clFullName };
   var actAccessPlans = getCommonAccessPlans(db, accessFindOption);
   var expAccessPlans = [{ScanType:"tbscan", IndexName:""} ];
	                                
   checkSnapShotAccessPlans(clFullName, expAccessPlans, actAccessPlans);
                                    	
   println("check unload result success!");
                                       	
   //load cs
   primaryDB.loadCS(csName);
                          	
   //check after load
   checkStat( db, csName, clName, "a", true, true );
   
   //check out snapshot access plans
	var accessFindOption = { Collection: clFullName };
   var actAccessPlans = getCommonAccessPlans(db, accessFindOption);
   var expAccessPlans = [{ScanType:"tbscan", IndexName:""} ];
	                                
   checkSnapShotAccessPlans(clFullName, expAccessPlans, actAccessPlans);
                                   	
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                                                
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   //query
   query(dbclPrimary, findConf, null, null, insertNums);
   query(dbclSlave, findConf, null, null, insertNums);
   
   //check out snapshot access plans
	var accessFindOption = { Collection: clFullName };
   var actAccessPlans = getCommonAccessPlans(db, accessFindOption);
   var expAccessPlans = [{ScanType:"tbscan", IndexName:""},
	                      {ScanType:"tbscan", IndexName:""}];
                     
   checkSnapShotAccessPlans(clFullName, expAccessPlans, actAccessPlans);
                                       	
   println("check load result success!");
                                   	
   db1.close();                                    
   commDropCS( db, csName, true, "drop CS in the end" );
}

function getPrimaryNodeDB( csName, clName )
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

main();

/************************************
*@Description: 指定group收集统计信息  
*@author:      liuxiaoxuan
*@createdate:  2017.11.10
*@testlinkCase: seqDB-11620
**************************************/
function main()
{	
   if(commIsStandalone(db))
   {
      println("skip standalone environment");
      return;
   } 
                                               	
   var csName = COMMCSNAME + "11620";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                               	
   var csOption = { PageSize: 4096 };
   commCreateCS( db, csName, false, "", csOption );
                                                           		
   //create cl	
   var groups = commGetGroups(db);
   var groupName = groups[0][0].GroupName;
                                                      	
   var clOption = { Group : groupName };
   var clName = COMMCLNAME + "11620";
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true );
                             
   var clFullName = csName + "." + clName;
                             
   //get master/slave datanode
   var db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "m"} );
   var dbclPrimary = db1.getCS(csName).getCL(clName);
   
   db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "s"} );
   var dbclSlave = db1.getCS(csName).getCL(clName);
                                                                                  	
   //create index
   commCreateIndex( dbcl, "a", {a : 1}, false );
                                                    	
   //insert
   var insertNums = 3000;
   var sameValues = 9000;
   insertDiffDatas( dbcl, insertNums );
   insertSameDatas( dbcl, insertNums, sameValues );
                                                        	
   //check before invoke analyze
   checkStat( db, csName, clName, "a", false, false );
                                                       	 
   //check the query explain of master/slave nodes 
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
                                                                           
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
   var expAccessPlans = [{ScanType:"ixscan", IndexName:"a"},
	                      {ScanType:"ixscan", IndexName:"a"}];  

   checkSnapShotAccessPlans(clFullName, expAccessPlans, actAccessPlans); 
                        
   println("check result before analyze success!");
                                                              	
   //invoke analyze
   var options = {GroupName : groupName};
   analyze( db, options );
                                                             	
   //check after analyze
   checkStat( db, csName, clName, "a", true, true );
                                                          
   //check out snapshot access plans
	var accessFindOption = { Collection: clFullName };
   var actAccessPlans = getCommonAccessPlans(db, accessFindOption);
   var expAccessPlans = [];   
   checkSnapShotAccessPlans(clFullName, expAccessPlans, actAccessPlans); 
                                                          
   //check the query explain of master/slave nodes 
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
                                            
   //analyze invalid groups
   var options1 = {GroupName : "SYSCoord"};
   checkAnalyzeInvalidGroup(options1);
                                                                                 	
   var options2 = {GroupName : "NotExistGroup"};
   checkAnalyzeInvalidGroup(options2);
                                                                                  	
   //check catalog
   var options3 = {GroupName : "SYSCatalogGroup"};
   checkAnalyzeCataGroup( options3 );
                                                          	
   println("check result after analyze success!");
   
   //query
   query(dbclPrimary, findConf, null, null, insertNums);
   query(dbclSlave, findConf, null, null, insertNums);
   
   //check out snapshot access plans
	var accessFindOption = { Collection: clFullName };
   var actAccessPlans = getCommonAccessPlans(db, accessFindOption);
   var expAccessPlans = [{ScanType:"tbscan", IndexName:""},
	                      {ScanType:"tbscan", IndexName:""}];  

   checkSnapShotAccessPlans(clFullName, expAccessPlans, actAccessPlans); 
   
   db1.close();
   commDropCS( db, csName, true, "drop CS in the end" );
}

function checkAnalyzeInvalidGroup( options )
{
   try
   {
      db.analyze( options );
      throw "NEED ANALYZE FAILED";
   }
   catch ( e )
   {
      if( -264 !== e && -154 !== e)
      {
         throw buildException( "check analyze", e, "check analyze", "success", "fail" );
      }
   }
}

function checkAnalyzeCataGroup( options )
{
   try
   {
      db.analyze( options );
      
      //get and connect to master node
      var cataRG = db.getCatalogRG();
      var priNode = cataRG.getMaster();
      var cataDB = priNode.connect();
      
      //check analyze stat info
      var sysStatCLName = "SYSSTAT.SYSCOLLECTIONSTAT";
      var sysStatIndexName = "SYSSTAT.SYSINDEXSTAT";
     
      var count = 0;
      var cursor = cataDB.listCollections();
      while(cursor.next())
      { 
         var name = cursor.current().toObj().Name;
         if(sysStatCLName == name || sysStatIndexName == name)
         {
            count++;
         }
      }
  
      if(count > 0)
      {
          throw 'CHECK CATAGROUP FAIL';
      }

   }
   catch ( e )
   {
      throw buildException( "check analyze", e, "check analyze", "success", "fail" );
   }
}
main();

/************************************
*@Description:  指定location其他参数收集统计信息   
*@author:      liuxiaoxuan
*@createdate:  2017.11.15
*@testlinkCase: seqDB-11759
**************************************/
function main()
{	
   if (commIsStandalone(db))
   {
      println("skip standalone environment");
      return ;
   }
   
   var csName = COMMCSNAME + "11759";
   commDropCS( db, csName, true, "drop CS in the beginning" );
	
   commCreateCS( db, csName, false, "" );
		
   //create cl	
   var clName = COMMCLNAME + "11759";
   var dbcl = commCreateCL( db, csName, clName );
	
   //insert datas
   var insertNums = 3000;
   var sameValues = 9000;
                                  	
   insertDiffDatas( dbcl, insertNums );
   insertSameDatas( dbcl, insertNums, sameValues );
   
   //create index
   commCreateIndex( dbcl, "a", {a : 1}, false );
   
   //get master/slave datanode
   var db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "m"} );
   var dbclPrimary = db1.getCS(csName).getCL(clName);
   db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "s"} );
   var dbclSlave = db1.getCS(csName).getCL(clName);
   
   //check before analyze         
   checkStat( db, csName, clName, "a", false, false );
                                               	
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
                                                           
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                             
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                      	
   println("check result before correct analyze success!");   
                                                            
   //get Group and Node info
   var groupName = commGetCLGroups( db, csName + "." + clName );
   var groupDetail = commGetGroups( db, false, groupName );    

   var nodesInGroup = groupDetail[0];
   var groupId = nodesInGroup[0].GroupID;
   
   var primaryPos = nodesInGroup[0].PrimaryPos ; 
   var slavePos = parseInt( primaryPos % (nodesInGroup.length - 1) ) + 1;
      
   var priNodeId = nodesInGroup[primaryPos].NodeID;
   var slaveNodeId = nodesInGroup[slavePos].NodeID;
   
   var priHostname = nodesInGroup[primaryPos].HostName;
   var priSvcname = nodesInGroup[primaryPos].svcname;
   
   var slaveHostname = nodesInGroup[slavePos].HostName;
   var slaveSvcname = nodesInGroup[slavePos].svcname;
                                                                                       
   //check correct analyze
   var options = [{ GroupID: groupId, GroupName: groupName},
                  { NodeID: priNodeId },
                  { HostName: priHostname, svcname: priSvcname }];
                                                            
   for(var i in options)
   {
      analyze( db, options[i] );
   }
   
   //check analyze 
   checkStat( db, csName, clName, "a", true, true );
                                               	
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                                                           
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                             
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                      	
   println("check result after correct analyze success!"); 
   
   //check error analyze, location : slave node
   var options = [{ NodeID: slaveNodeId },
                  { HostName: slaveHostname, svcname: slaveSvcname }];
                                                            
   for(var i in options)
   {
      checkAnalyzeResult( options[i] );
   }
   
   println("check error analyze success!");
   
   db1.close();
   commDropCS( db, csName, true, "drop CS in the end" );
}

function checkAnalyzeResult( options )
{  
   try
   {
      db.analyze( options );
      throw "NEED ANALYZE FAILED";
   }
   catch(e)
   {
      if( -264 !== e )
      {
         throw buildException("check analyze", e, "analyze", "analyze success", e);
      }
   }
}

main();

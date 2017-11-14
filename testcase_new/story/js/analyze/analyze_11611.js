/************************************
*@Description: 指定普通cl收集统计信息  
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11611
**************************************/
function main()
{
   var csName = COMMCSNAME + "11611";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   //create CLs
   var clName1 = COMMCLNAME + "11611_1";
   var dbcl1 = commCreateCL( db, csName, clName1 );
                                                      	
   var clName2 = COMMCLNAME + "11611_2";
   var dbcl2 = commCreateCL( db, csName, clName2 );
                                                       
   var clName3 = COMMCLNAME + "11611_3";
   var dbcl3 = commCreateCL( db, csName, clName3 );
                                                             	
   var clName4 = COMMCLNAME + "11611_4";
   var dbcl4 = commCreateCL( db, csName, clName4 );
                                                       	
   //get master/slave datanode
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary1 = db.getCS(csName).getCL(clName1);
   var dbclPrimary2 = db.getCS(csName).getCL(clName2);
   var dbclPrimary3 = db.getCS(csName).getCL(clName3);
   var dbclPrimary4 = db.getCS(csName).getCL(clName4);
                                                           	
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave1 = db.getCS(csName).getCL(clName1);
   var dbclSlave2 = db.getCS(csName).getCL(clName2);
   var dbclSlave3 = db.getCS(csName).getCL(clName3);
   var dbclSlave4 = db.getCS(csName).getCL(clName4);
                                                              	
   //create index
   commCreateIndex( dbcl3, "a", {a : 1}, false );
   commCreateIndex( dbcl4, "a", {a : 1}, false );
                                                      	
   //insert datas
   var insertNums = 3000;
   var sameValues = 9000;
                                                            	
   // include datas , but no index
   insertDiffDatas( dbcl1, insertNums );
   insertSameDatas( dbcl1, insertNums, sameValues );
   // include datas and index
   insertDiffDatas( dbcl3, insertNums );
   insertSameDatas( dbcl3, insertNums, sameValues );
                                                         	
   //check before invoke analyze
   checkStat( db, csName, clName1, "", false, false );
   checkStat( db, csName, clName2, "", false, false );
   checkStat( db, csName, clName3, "a", false, false );
   checkStat( db, csName, clName4, "a", false, false );
                                                              	
   //check the query explain of master/slave nodes 
   var findConf = {a : 9000};
                                                          	
   var expExplains1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:0}];
   var expExplains3 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   var expExplains4 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
                                                                               
   var actExplains1 = getCommonExplain( dbclPrimary1, findConf);
   checkExplain( actExplains1, expExplains1 );
   var actExplains2 = getCommonExplain( dbclPrimary2, findConf);
   checkExplain( actExplains2, expExplains2 );
   var actExplains3 = getCommonExplain( dbclPrimary3, findConf);
   checkExplain( actExplains3, expExplains3 );
   var actExplains4 = getCommonExplain( dbclPrimary4, findConf);
   checkExplain( actExplains4, expExplains4 );
                                                        
   var actExplains1 = getCommonExplain( dbclSlave1, findConf);
   checkExplain( actExplains1, expExplains1 );
   var actExplains2 = getCommonExplain( dbclSlave2, findConf);
   checkExplain( actExplains2, expExplains2 );
   var actExplains3 = getCommonExplain( dbclSlave3, findConf);
   checkExplain( actExplains3, expExplains3 );
   var actExplains4 = getCommonExplain( dbclSlave4, findConf);
   checkExplain( actExplains4, expExplains4 );
                                                               	
   println("check result before analyze success!");
                                                           
   //invoke analyze
   var options = [{Collection: csName + "." + clName1},
                  {Collection: csName + "." + clName2},
                  {Collection: csName + "." + clName3},
                  {Collection: csName + "." + clName4}];
                                                           						
   for(var i in options)
   {
      analyze( db, options[i] );
   }
                                        
   //check after analyze
   checkStat( db, csName, clName1, "", true, false );
   checkStat( db, csName, clName2, "", false, false );
   checkStat( db, csName, clName3, "a", true, true );
   checkStat( db, csName, clName4, "c", false, false );
                                                        
   //check the query explain of master/slave nodes 
   var findConf = {a : 9000};
                                                        	
   var expExplains1 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   var expExplains2 = [{ScanType:"tbscan", IndexName:"", ReturnNum:0}];
   var expExplains3 = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
   var expExplains4 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:0}];
                                                                             
   var actExplains1 = getCommonExplain( dbclPrimary1, findConf);
   checkExplain( actExplains1, expExplains1 );
   var actExplains2 = getCommonExplain( dbclPrimary2, findConf);
   checkExplain( actExplains2, expExplains2 );
   var actExplains3 = getCommonExplain( dbclPrimary3, findConf);
   checkExplain( actExplains3, expExplains3 );
   var actExplains4 = getCommonExplain( dbclPrimary4, findConf);
   checkExplain( actExplains4, expExplains4 );
                                                                       
   var actExplains1 = getCommonExplain( dbclSlave1, findConf);
   checkExplain( actExplains1, expExplains1 );
   var actExplains2 = getCommonExplain( dbclSlave2, findConf);
   checkExplain( actExplains2, expExplains2 );
   var actExplains3 = getCommonExplain( dbclSlave3, findConf);
   checkExplain( actExplains3, expExplains3 );
   var actExplains4 = getCommonExplain( dbclSlave4, findConf);
   checkExplain( actExplains4, expExplains4 );
                                                       	
   //analyze table below SYSSTAT 
   var options1 = {Collection: "SYSSTAT.SYSCOLLECTIONSTAT"};
   //checkAnalyzeInvalidResult(options1);
                                                              	
   var options2 = {Collection: "SYSSTAT.SYSINDEXSTAT"};
   //checkAnalyzeInvalidResult(options2);
                                                         	
   println("check result after analyze success!");
                                                       	
   commDropCS( db, csName, true, "drop CS in the end" );
}

function checkAnalyzeInvalidResult( options )
{
   try
   {
      db.analyze( options );
      throw "NEED ANALYZE FAILED";
   }
   catch ( e )
   {
      if( -23 !== e )
      {
         throw buildException( "check analyze", e, "check analyze", "success", "fail" );
      }
   }
}
main();

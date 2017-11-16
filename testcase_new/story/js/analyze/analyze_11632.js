/************************************
*@Description:  指定cs将统计信息重新加载至缓存再清空 
*@author:      liuxiaoxuan
*@createdate:  2017.11.13
*@testlinkCase: seqDB-11632
**************************************/
function main()
{	
   var csName = COMMCSNAME + "11632";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                         	
   commCreateCS( db, csName, false, "" );
                                               		
   //create cl	
   var clName = COMMCLNAME + "11632";
   var dbcl = commCreateCL( db, csName, clName );
                                                   	
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
    
    //analyze
   var cl_full_name = csName + "." + clName;
   var options = { Collection : cl_full_name };
   analyze( db, options );
    
   //check before analyze 
   checkStat( db, csName, clName, "a", true, true );
                          	
   //check the query explain before analyze
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                  
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                       
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                      	
   println("check result before default analyze !");
                           	
   //invoke analyze
   
   var options = { Mode : 3, Collection : cl_full_name};
   analyze( db, options );
                            	
   //check after analyze success
   checkStat( db, csName, clName, "a", true, false );
                                                    	
   //check the query explain after analyze
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
                                                  
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                             
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                 	
   println("check result after default analyze !");
                                                       	
   // modify SYSSTAT info
   var mcvValues = [{a: 0},{a: 1},{a:9000}];
   var fracs = [500,500,9000];
   updateIndexStateInfo( db, csName, clName, "a", mcvValues, fracs );
                                                                 
   // reload analyze again
   var options = { Mode : 4, CollectionSpace : csName };
   analyze( db, options );
                                              	
   //check after reload analyze success
   checkStat( db, csName, clName, "a", true, true );
                                                      	
   //check the query explain after analyze
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                                  
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                                
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                          	
   println("check result after reload analyze success!");
                                                 	
   //truncate invalidate
   var options = { Mode : 5, CollectionSpace : csName };
   analyze( db, options );
                                    	
	//check analyze after truncate invalidate
   checkStat( db, csName, clName, "a", true, true );
   
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                                           	                          	
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                  
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                              	
   println("check result after truncate invalidate!");
   
   // modify SYSSTAT info again
   var mcvValues = [{a: 0},{a: 1},{a:9000}];
   var fracs = [500,500,500];
   updateIndexStateInfo( db, csName, clName, "a", mcvValues, fracs );                                                            
                                              	
   //check after reload analyze success
   checkStat( db, csName, clName, "a", true, true );
                                                      	
   //check the query explain after analyze
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];
                                  
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                                
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                          	
   println("check result after modify analyze info again success!");
                                                 	
   //truncate invalidate
   var options = { Mode : 5, CollectionSpace : csName };
   analyze( db, options );
                                    	
	//check analyze after truncate invalidate
   checkStat( db, csName, clName, "a", true, true );
   
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
                                           	                          	
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                  
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                              	
   println("check result after truncate invalidate again!");
            
   db1.close();            
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

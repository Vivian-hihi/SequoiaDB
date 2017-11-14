/************************************
*@Description:  指定唯一索引生成默认统计信息并手工修改统计信息再清空
*@author:      liuxiaoxuan
*@createdate:  2017.11.13
*@testlinkCase: seqDB-11627
**************************************/
function main()
{	
   var csName = COMMCSNAME + "11627";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                              	
   commCreateCS( db, csName, false, "" );
                                                         		
   //create cl	
   var clName = COMMCLNAME + "11627";
   var dbcl = commCreateCL( db, csName, clName );
                                                      	
   //get master/slave datanode
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db.getCS(csName).getCL(clName);
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db.getCS(csName).getCL(clName);
                                                          	
   //insert datas
   var insertNums = 5000;
   insertDiffDatas( dbcl, insertNums );
                                                  	
   //create unique index
   var isUnique = true;
   commCreateIndex( dbcl, "a", {a : 1}, isUnique );
   
   //analyze   
   var cl_full_name = csName + "." + clName;
   var options = { Collection :  cl_full_name};
   analyze( db, options );
                                                      
   //check before analyze success
   checkStat( db, csName, clName, "a", true, true );
                                                       	
   //check the query explain before analyze
   var findConf = {a : 1000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:1}];
                                                                
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                                              
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                              	
   println("check result before default analyze !");
                                                                	
   //invoke analyze
   var options = { Mode : 3, Collection :  cl_full_name, Index: "a"};
   analyze( db, options );
                                                                         	
   //check after analyze success
   checkStat( db, csName, clName, "a", true, false );
                                                                         	
   //check the query explain after analyze
   var findConf = {a : 1000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:1}];
                                                                        
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                                                  
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                                 	
   println("check result after default analyze !");
                                                                 	
   // modify SYSSTAT info
   var mcvValues = [{a: 0},{a: 1},{a:1000}];
   var fracs = [500,500,9000];
   updateIndexStateInfo( db, csName, clName, "a", mcvValues, fracs );
                                                                           	
   // reload analyze 
   var options = { Mode : 4, Collection : cl_full_name };
   analyze( db, options );
                                                            	
   //check after reload analyze success
   checkStat( db, csName, clName, "a", true, true );
                                                          	
   var findConf = {a : 1000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:1}];
                                                                         
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                                                 
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                              	
   println("check result after reload analyze success!");
                                                           	
   //truncate invalidate
   var options = { Mode : 5, Collection : cl_full_name };
   analyze( db, options );
                                                               	
   checkStat( db, csName, clName, "a", true, true );
                                                      	
   var findConf = {a : 1000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:1}];
                                                                         	
   checkStat( db, csName, clName, "a", true, true );
                                                           	
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                       	
   println("check result after truncate invalidate!");
   
   //modify SYS info again
   var mcvValues = [{a: 0},{a: 1},{a:1000}];
   var fracs = [500,500,500];
   updateIndexStateInfo( db, csName, clName, "a", mcvValues, fracs );
                                 	
   //check after reload analyze info
   checkStat( db, csName, clName, "a", true, true );
                                               	
   var findConf = {a : 1000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:1}];
                                             
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                                 
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                 	
   println("check result after second modify SYS info!");
   
   //truncate invalidate again
   var options = { Mode : 5, Collection : cl_full_name };
   analyze( db, options );
                              	
   //check analyze after truncate invalidate
   checkStat( db, csName, clName, "a", true, true );
                           
   var findConf = {a : 1000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:1}];
                        
                            		
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
                                
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
                                                	
   println("check result after second truncate invalidate!");
                                                             	
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

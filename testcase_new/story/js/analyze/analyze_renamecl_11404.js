/************************************
*@Description: renameCl更新统计信息、清空缓存功能验证 (只支持standalone模式)
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11404
**************************************/
function main()
{
   if(!commIsStandalone(db))
   {
      println("not standalone enviroment");
      return;
   }
                                                             	
   var csName = COMMCSNAME + "11404";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                              	
   commCreateCS( db, csName, false, "" );
                                  	
   //create CLs
   var clName1 = COMMCLNAME + "11404_1";
   var dbcl1 = commCreateCL( db, csName, clName1 );

   var clName2 = COMMCLNAME + "11404_2";
   var dbcl2 = commCreateCL( db, csName, clName2 );
                                                        
   commCreateIndex( dbcl1, "a", {a : 1}, false );
   commCreateIndex( dbcl2, "b", {b : 1}, false );
                          
   var insertNums = 3000;
   var sameValues = 9000;
   insertDiffDatas( dbcl1, insertNums );
   insertSameDatas( dbcl1, insertNums, sameValues );
   insertDiffDatas( dbcl2, insertNums );
   insertSameDatas( dbcl2, insertNums, sameValues );
	
   //check before invoke analyze
   checkStat( db, csName, clName1, "a", false, false );
   checkStat( db, csName, clName2, "b", false, false );
              
   var findConf1 = {a : 9000};
   var findConf2 = {b : 9000};

   var actExplains1 = getCommonExplain( dbcl1, findConf1);
   var actExplains2 = getCommonExplain( dbcl2, findConf2);
   var expExplains1 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   var expExplains2 = [{ScanType:"ixscan", IndexName:"b", ReturnNum:insertNums}];

   //check explain
   checkExplain( actExplains1, expExplains1 );
   checkExplain( actExplains2, expExplains2 );

   //check snapshot access plan
   var actAccessPlan1 = db.snapshot(11, {Collection : csName + "." + clName1}).toArray();
   var actAccessPlan2 = db.snapshot(11, {Collection : csName + "." + clName2}).toArray();

   var expectAccessPlan1 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                             "AccessCount": 1}];
   var expectAccessPlan2 = [{"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                             "AccessCount": 1}];
	
   checkSnapShotAccessPlans( expectAccessPlan1, actAccessPlan1 );
   checkSnapShotAccessPlans( expectAccessPlan2, actAccessPlan2 );
                                                                 	
   println("check result before analyze success!");
                      
   //invoke analyze
   var options = {CollectionSpace: csName};
   analyze( db, options );
                                                            
   //check after analyze
   checkStat( db, csName, clName1, "a", true, true );
   checkStat( db, csName, clName2, "b", true, true );

   //check access plan after analyze before query
   var actAccessPlan1 = db.snapshot(11, {Collection : csName + "." + clName1}).toArray();
   var actAccessPlan2 = db.snapshot(11, {Collection : csName + "." + clName2}).toArray();

   var expectAccessPlan1 = [];
   var expectAccessPlan2 = [];
	
   checkSnapShotAccessPlans( expectAccessPlan1, actAccessPlan1 );
   checkSnapShotAccessPlans( expectAccessPlan2, actAccessPlan2 );
                                                                  
   var findConf1 = {a : 9000};
   var findConf2 = {b : 9000};

   var actExplains1 = getCommonExplain( dbcl1, findConf1);
   var actExplains2 = getCommonExplain( dbcl2, findConf2);
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];

   //check explain
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );

   //check snapshot access plan after query
   var actAccessPlan1 = db.snapshot(11, {Collection : csName + "." + clName1}).toArray();
   var actAccessPlan2 = db.snapshot(11, {Collection : csName + "." + clName2}).toArray();

   var expectAccessPlan1 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                             "AccessCount": 1}];
   var expectAccessPlan2 = [{"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                             "AccessCount": 1}];
	
   checkSnapShotAccessPlans( expectAccessPlan1, actAccessPlan1 );
   checkSnapShotAccessPlans( expectAccessPlan2, actAccessPlan2 );
                                            	
   println("check result after analyze success!");
                                                       	
   //rename CL1
   var oldClName = clName1;
   var newClName = "newClName";
   renameCL(csName, oldClName, newClName);
                                                                                	
   //check newCL's anaylze info
   checkAnalyzeStatInfo(csName, newClName);
   checkStat( db, csName, newClName, "a", true, true );
   checkStat( db, csName, clName2, "b", true, true );
                                                                 	
   //check result after renameCL
   var newCL = db.getCS(csName).getCL(newClName);
   var findConf1 = {a : 9000};
   var findConf2 = {b : 9000};

   var actExplains1 = getCommonExplain( newCL, findConf1);
   var actExplains2 = getCommonExplain( dbcl2, findConf2);
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];

   //check explain
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );

   //check snapshot access plan
   var actAccessPlan1 = db.snapshot(11, {Collection : csName + "." + newClName}).toArray();
   var actAccessPlan2 = db.snapshot(11, {Collection : csName + "." + clName2}).toArray();

   var expectAccessPlan1 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                             "AccessCount": 1}];
   var expectAccessPlan2 = [{"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                             "AccessCount": 2}];
	
   checkSnapShotAccessPlans( expectAccessPlan1, actAccessPlan1 );
   checkSnapShotAccessPlans( expectAccessPlan2, actAccessPlan2 );
                                             	
   println("check analyze result after rename CL success!");
   
   //check and create oldCL
   dbcl = commCreateCL( db, csName, clName1 );
           
   //create index
   commCreateIndex( dbcl, "a", {a : 1}, false );
   
   //insert datas
   var insertNums = 3000;
   var sameValues = 9000;
   insertDiffDatas( dbcl, insertNums );
   insertSameDatas( dbcl, insertNums, sameValues );
   
   //check result,no left cache
   var findConf = {a : 9000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   var actExplains = getCommonExplain( dbcl, findConf);
   checkExplain( actExplains, expExplains );
                                               
   println("check result success after create old CL!");
                                               
   commDropCS( db, csName, true, "drop CS in the end" );
}

function renameCL( csName, oldClName, newClName )
{
   try
   {
      var cs = db.getCS(csName);
      cs.renameCL( oldClName, newClName );
   }
   catch(e)
   {
      throw buildException("renameCL", e, "rename cl", "rename success", e);
   }
}

function checkAnalyzeStatInfo( csName, clName )
{
   try
   { 
      var matcher = {CollectionSpace: csName, Collection: clName}
      var rec = db.SYSSTAT.SYSCOLLECTIONSTAT.find(matcher).toArray();
            
      if(0 === rec.length)
      {
         throw "CHECK RENAMECL FAILED";
      }
      
   }
   catch(e)
   {
      throw buildException("check analyze info", e, "check", "success", e);
   }
}
main();

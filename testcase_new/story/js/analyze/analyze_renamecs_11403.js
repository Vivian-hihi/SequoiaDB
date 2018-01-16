/************************************
*@Description: renameCS更新统计信息、清空缓存功能验证(只支持standalone模式)
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11403
**************************************/
function main()
{
   if(!commIsStandalone(db))
   {
      println("not standalone enviroment");
      return;
   }
                                                                                 	
   var csName = COMMCSNAME + "11403";
   var csName_new = "newCsName"
   commDropCS( db, csName, true, "drop CS in the beginning" );
   commDropCS( db, csName_new, true, "drop CS in the beginning" );
                                                                                	
   commCreateCS( db, csName, false, "" );
                                                                          	
   //create CLs
   var clName1 = COMMCLNAME + "11403_1";
   var dbcl1 = commCreateCL( db, csName, clName1 );

   var clName2 = COMMCLNAME + "11403_2";
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
                                                             	
   //check the query explain before analyze
   var findConf1 = {a : 9000};
   var findConf2 = {b : 9000};

   var actExplains1 = getCommonExplain( dbcl1, findConf1);
   var actExplains2 = getCommonExplain( dbcl2, findConf2);
   var expExplains1 = [{ScanType:"ixscan", IndexName:"a", ReturnNum:insertNums}];
   var expExplains2 = [{ScanType:"ixscan", IndexName:"b", ReturnNum:insertNums}];

   //check explain
   checkExplain( actExplains1, expExplains1 );
   checkExplain( actExplains2, expExplains2 );

   //query no explain
   querySameWithOutExplain( dbcl1, findConf1 );
   querySameWithOutExplain( dbcl2, findConf2 );

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
   
   //check the query explain after analyze
   var findConf1 = {a : 9000};
   var findConf2 = {b : 9000};

   var actExplains1 = getCommonExplain( dbcl1, findConf1);
   var actExplains2 = getCommonExplain( dbcl2, findConf2);
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];

   //check explain
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );

   //query no explain
   querySameWithOutExplain( dbcl1, findConf1 );
   querySameWithOutExplain( dbcl2, findConf2 );

   //check snapshot access plan
   var actAccessPlan1 = db.snapshot(11, {Collection : csName + "." + clName1}).toArray();
   var actAccessPlan2 = db.snapshot(11, {Collection : csName + "." + clName2}).toArray();

   var expectAccessPlan1 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                             "AccessCount": 1}];
   var expectAccessPlan2 = [{"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                             "AccessCount": 1}];
	
   
   checkSnapShotAccessPlans( expectAccessPlan1, actAccessPlan1 );
   checkSnapShotAccessPlans( expectAccessPlan2, actAccessPlan2 );
                                                                             	
   println("check result after analyze success!");
                                                                                  	
   //rename CS
   var oldCsName = csName;
   var newCsName = csName_new;
   renameCS(oldCsName, newCsName);
                                                	 
   //check newCL's anaylze info
   checkStat( db, newCsName, clName1, "a", true, true );
   checkStat( db, newCsName, clName2, "b", true, true );

   //check access plans after renameCS
   var actAccessPlan1 = db.snapshot(11, {Collection : newCsName + "." + clName1}).toArray();
   var actAccessPlan2 = db.snapshot(11, {Collection : newCsName + "." + clName2}).toArray();

   var expectAccessPlan1 = [];
   var expectAccessPlan2 = [];
	
   checkSnapShotAccessPlans( expectAccessPlan1, actAccessPlan1 );
   checkSnapShotAccessPlans( expectAccessPlan2, actAccessPlan2 );
                                                         	
   //check result with new CS
   var findConf1 = {a : 9000};
   var findConf2 = {b : 9000};
   var newCL1 = db.getCS(newCsName).getCL(clName1);
   var newCL2 = db.getCS(newCsName).getCL(clName2);

   var actExplains1 = getCommonExplain( newCL1, findConf1);
   var actExplains2 = getCommonExplain( newCL2, findConf2);
   var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNums}];

   //check explain
   checkExplain( actExplains1, expExplains );
   checkExplain( actExplains2, expExplains );

   //query no explain
   querySameWithOutExplain( newCL1, findConf1 );
   querySameWithOutExplain( newCL2, findConf2 );

   //check snapshot access plan
   var actAccessPlan1 = db.snapshot(11, {Collection : newCsName + "." + clName1}).toArray();
   var actAccessPlan2 = db.snapshot(11, {Collection : newCsName + "." + clName2}).toArray();

   var expectAccessPlan1 = [{"Query": {"$and": [{"a": {"$et": {"$param": 0,"$ctype": 10}}}]},
                             "AccessCount": 1}];
   var expectAccessPlan2 = [{"Query": {"$and": [{"b": {"$et": {"$param": 0,"$ctype": 10}}}]},
                             "AccessCount": 1}];
	
   checkSnapShotAccessPlans( expectAccessPlan1, actAccessPlan1 );
   checkSnapShotAccessPlans( expectAccessPlan2, actAccessPlan2 );
                                                 	
   println("check result after rename success!");
                                                      
   commDropCS( db, csName, true, "drop CS in the end" );
   commDropCS( db, csName_new, true, "drop CS in the end" );
}

function renameCS( oldCsName, newCsName )
{
   try
   {
      db.renameCS( oldCsName, newCsName );
   }
   catch(e)
   {
      throw buildException("renameCS", e, "rename cs", "rename success", e);
   }
}

function querySameWithOutExplain(dbcl, findConf, sortConf, hintConf)
{
   if ( typeof(findConf) == "undefined" ) { findConf = null; }
   if ( typeof(sortConf) == "undefined" ) { sortConf = null; }
   if ( typeof(hintConf) == "undefined" ) { hintConf = null; }
   
   //执行查询
   var rc = dbcl.find(findConf).sort(sortConf).hint(hintConf);
   while(rc.next())
   {
   }
}

main();

/************************************
*@Description: 主子表中带limit/skip进行全文检索
*@author:      liuxiaoxuan
*@createdate:  2018.11.26
*@testlinkCase: seqDB-15137
**************************************/
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }
   
   var groups = commGetGroups( db );
   if(groups.length < 2 )
   {
      println("less than two groups");
      return;
   }

   var mainCLName = COMMCLNAME + "_ES_15137_maincl";
   var subCLName1 = COMMCLNAME + "_ES_15137_subcl_1";
   var subCLName2 = COMMCLNAME + "_ES_15137_subcl_2";
   commDropCL(db, COMMCSNAME, mainCLName, true, true);
   commDropCL(db, COMMCSNAME, subCLName1, true, true);
   commDropCL(db, COMMCSNAME, subCLName2, true, true);
   
   var mainCL = commCreateCLByOption( db, COMMCSNAME, mainCLName, {ShardingKey: {a : 1}, IsMainCL: true});
   var subCL1 = commCreateCL( db, COMMCSNAME, subCLName1 );
   var subCL2 = commCreateCLByOption( db, COMMCSNAME, subCLName2, {ShardingKey: {b : 1}, Group: groups[0][0]["GroupName"]});
   subCL2.split(groups[0][0]["GroupName"], groups[1][0]["GroupName"], 50);

   // attach cl
   mainCL.attachCL(COMMCSNAME + "." + subCLName1, {LowBound: {a: "test_15137"}, UpBound: {a: "test_15138" } });
   mainCL.attachCL(COMMCSNAME + "." + subCLName2, {LowBound: {a: "zzz_15137"}, UpBound: {a: "zzz_15138" } });
   var textIndexName = "textIndex15137";
   commCreateIndex( mainCL, textIndexName, {a: "text", b : "text"});
  
   // insert
   var objs = new Array();
   for (var i = 0; i < 20000 ; i++){
      objs.push({a: "test_15137 " + i, b: "testb_" + i });
   }
   for (var i = 0; i < 10000 ; i++){
      objs.push({a: "zzz_15137 " + i, b: "zzzb_" + i });
   }
   insertRecords(mainCL, objs);
   
   if(30000 != mainCL.count())
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }
   checkMainCLFullSyncToES(COMMCSNAME, mainCLName, textIndexName, 30000);
   
     // return datas from one group
//   // skip + limit <= 1w without sort, result is random
//   var dbOpr = new DBOperator();
//   var limit = 100;
//   var skip =  2000;
//   var actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match" : {"a" : "test_15137"}}}}}, null, null, null, limit, skip);
//   var expResult = dbOpr.findFromCL(mainCL, {"a": {"$lt": "z"}}, null, null, null, limit, skip);
//   expResult.sort(compare("a"));
//   actResult.sort(compare("a"));
//   println("expResult: " + JSON.stringify(expResult) + "\nactResult: " + JSON.stringify(actResult));
//   checkResult(expResult, actResult);
//   println("---check skip+limit <= 1w success when return datas from one group---");
//
//   // skip + limit > 1w
//   limit = 6000; // limit < 1w
//   skip =  5000; // skip < 1w
//   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match" : {"a" : "test_15137"}}}}}, null, null, null, limit, skip);
//   expResult = dbOpr.findFromCL(mainCL, {"a": {"$lt": "z"}}, null, null, null, limit, skip);
//   expResult.sort(compare("a"));
//   actResult.sort(compare("a"));
//   checkResult(expResult, actResult);
//
//   limit = 10000; // limit >= 1w
//   skip =  10; // skip < 1w
//   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match" : {"a" : "test_15137"}}}}}, null, null, null, limit, skip);
//   expResult = dbOpr.findFromCL(mainCL, {"a": {"$lt": "z"}}, null, null, null, limit, skip);
//   expResult.sort(compare("a"));
//   actResult.sort(compare("a"));
//   checkResult(expResult, actResult);
//
//   limit = 10; // limit < 1w
//   skip =  10000; // skip >= 1w
//   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match" : {"a" : "test_15137"}}}}}, null, null, null, limit, skip);
//   expResult = dbOpr.findFromCL(mainCL, {"a": {"$lt": "z"}}, null, null, null, limit, skip);
//   expResult.sort(compare("a"));
//   actResult.sort(compare("a"));
//   checkResult(expResult, actResult);
//
//   limit = 10000; // limit >= 1w
//   skip =  10000; // skip >= 1w
//   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match" : {"a" : "test_15137"}}}}}, null, null, null, limit, skip);
//   expResult = dbOpr.findFromCL(mainCL, {"a": {"$lt": "z"}}, null, null, null, limit, skip);
//   expResult.sort(compare("a"));
//   actResult.sort(compare("a"));
//   checkResult(expResult, actResult);
//   println("---check skip+limit > 1w success when return datas from one group---");

   // return datas from one group
   // sort and skip + limit <= 1w
   var limit = 5000;
   var skip =  1000;
   var actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match" : {"a" : "test_15137"}}}}}, null, {_id : 1}, null, limit, skip);
   var expResult = dbOpr.findFromCL(mainCL, {"a": {"$lt": "z"}}, null, {_id : 1}, null, limit, skip);
   checkResult(expResult, actResult);
   println("---check skip+limit <= 1w with sort success when return datas from one group---");

   // sort and skip + limit > 1w
   limit = 6000; // limit < 1w
   skip =  5000; // skip < 1w
   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match" : {"a" : "test_15137"}}}}}, null, {_id : 1}, null, limit, skip);
   expResult = dbOpr.findFromCL(mainCL, {"a": {"$lt": "z"}}, null, {_id : 1}, null, limit, skip);
   checkResult(expResult, actResult);

   limit = 10000; // limit >= 1w
   skip =  10; // skip < 1w
   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match" : {"a" : "test_15137"}}}}}, null, {_id : 1}, null, limit, skip);
   expResult = dbOpr.findFromCL(mainCL, {"a": {"$lt": "z"}}, null, {_id : 1}, null, limit, skip);
   checkResult(expResult, actResult);

   limit = 10; // limit < 1w
   skip =  10000; // skip >= 1w
   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match" : {"a" : "test_15137"}}}}}, null, {_id : 1}, null, limit, skip);
   expResult = dbOpr.findFromCL(mainCL, {"a": {"$lt": "z"}}, null, {_id : 1}, null, limit, skip);
   checkResult(expResult, actResult);
   println("---check skip+limit > 1w with sort success when return datas from one group---");

   // return datas from more groups
//   // skip + limit <= 1w without sort, result is random
//   limit = 5000;
//   skip =  1000;
//   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, null, null, limit, skip);
//   expResult = dbOpr.findFromCL(mainCL, null, null, null, null, limit, skip);
//   expResult.sort(compare("a"));
//   actResult.sort(compare("a"));
//   checkResult(expResult, actResult);
//   println("---check skip+limit <= 1w success when return datas from more groups---");
//
//   // skip + limit > 1w
//   limit = 6000; // limit < 1w
//   skip =  5000; // skip < 1w
//   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, null, null, limit, skip);
//   expResult = dbOpr.findFromCL(mainCL, null, null, null, null, limit, skip);
//   expResult.sort(compare("a"));
//   actResult.sort(compare("a"));
//   checkResult(expResult, actResult);
//
//   limit = 10000; // limit >= 1w
//   skip =  10; // skip < 1w
//   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, null, null, limit, skip);
//   expResult = dbOpr.findFromCL(mainCL, null, null, null, null, limit, skip);
//   expResult.sort(compare("a"));
//   actResult.sort(compare("a"));
//   checkResult(expResult, actResult);
//
//   limit = 10; // limit < 1w
//   skip =  10000; // skip >= 1w
//   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, null, null, limit, skip);
//   expResult = dbOpr.findFromCL(mainCL, null, null, null, null, limit, skip);
//   expResult.sort(compare("a"));
//   actResult.sort(compare("a"));
//   checkResult(expResult, actResult);
//
//   limit = 10000; // limit >= 1w
//   skip =  10000; // skip >= 1w
//   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, null, null, limit, skip);
//   expResult = dbOpr.findFromCL(mainCL, null, null, null, null, limit, skip);
//   expResult.sort(compare("a"));
//   actResult.sort(compare("a"));
//   checkResult(expResult, actResult);
//   println("---check skip+limit > 1w success when return datas from more groups---");

   // return datas from more groups
   // sort and skip + limit <= 1w
   limit = 5000;
   skip =  1000;
   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, {_id : 1}, null, limit, skip);
   expResult = dbOpr.findFromCL(mainCL, null, null, {_id : 1}, null, limit, skip);
   checkResult(expResult, actResult);
   println("---check skip+limit <= 1w with sort success when return datas from more groups---");

   // sort and skip + limit > 1w
   limit = 6000; // limit < 1w
   skip =  5000; // skip < 1w
   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, {_id : 1}, null, limit, skip);
   expResult = dbOpr.findFromCL(mainCL, null, null, {_id : 1}, null, limit, skip);
   checkResult(expResult, actResult);

   limit = 10000; // limit >= 1w
   skip =  10; // skip < 1w
   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, {_id : 1}, null, limit, skip);
   expResult = dbOpr.findFromCL(mainCL, null, null, {_id : 1}, null, limit, skip);
   checkResult(expResult, actResult);

   limit = 10; // limit < 1w
   skip =  10000; // skip >= 1w
   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, {_id : 1}, null, limit, skip);
   expResult = dbOpr.findFromCL(mainCL, null, null, {_id : 1}, null, limit, skip);
   checkResult(expResult, actResult);

   limit = 10000; // limit >= 1w
   skip =  10000; // skip >= 1w
   actResult = dbOpr.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, {_id : 1}, null, limit, skip);
   expResult = dbOpr.findFromCL(mainCL, null, null, {_id : 1}, null, limit, skip);
   checkResult(expResult, actResult);
   println("---check skip+limit > 1w with sort success when return datas from more groups---");

   commDropCL(db, COMMCSNAME, subCLName1, true, true);
   commDropCL(db, COMMCSNAME, subCLName2, true, true);
   commDropCL(db, COMMCSNAME, mainCLName, true, true);
}

main()

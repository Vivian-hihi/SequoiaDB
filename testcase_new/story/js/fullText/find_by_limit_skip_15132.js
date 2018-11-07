/***************************************************************************
@Description :seqDB-15132 :range分区表中带limit/skip进行全文检索 
@Modify list :
              2018-10-29  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }

   var clName = COMMCLNAME + "_ES_15132";
   commDropCL(db, COMMCSNAME, clName, true, true);

   //创建range分区表，并创建全文索引
   var groups = commGetGroups( db );
   if(groups.length < 2 ){
      throw buildException(commGetGroups, "Only one group", "dbcl get groups' name", "two or more groups", "less than two groups");
   }
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, {ShardingType : "range", ShardingKey : {a : 1}, Group : groups[0][0]["GroupName"]} );
   commCreateIndex( dbcl, "fullIndex", {a : "text"});
   
   //插入包含全文索引字段的记录
   var records = new Array();
   for (var i = 0; i < 25000 ; i++){
      var record = {a : "a"};
      records.push(record);
   }
   for (var i = 0; i < 5000 ; i++){
      var record = {a : "f"};
      records.push(record);
   }
   insertRecords(dbcl, records);
   
   if(30000 != dbcl.count())
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }
   
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex", 30000);
   dbcl.split(groups[0][0]["GroupName"], groups[1][0]["GroupName"], {a : "c"}, {a : "g"});
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex", 30000);
   
   //skip/limit组合查询结果覆盖单组返回
   println("======skip/limit======");
   var dbOperator = new DBOperator();
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, null, null, 4000, 1000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, null, null, 4000, 1000);
   checkCount(expResult, actResult);
   println("===skip + limit < 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, null, null, 5000, 5000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, null, null, 5000, 5000);
   checkCount(expResult, actResult);
   println("===skip + limit = 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, null, null, 6000, 6000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, null, null, 6000, 6000);
   checkCount(expResult, actResult);
   println("===skip + limit > 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, null, null, 4000, 11000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, null, null, 4000, 11000);
   checkCount(expResult, actResult);
   println("===skip > 10000 && limit < 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, null, null, 12000, 13000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, null, null, 12000, 13000);
   checkCount(expResult, actResult);
   println("===skip > 10000 && limit > 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, null, null, 15000, 5000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, null, null, 15000, 5000);
   checkCount(expResult, actResult);
   println("===skip < 10000 && limit > 10000 success===");
   
   //skip/limit组合查询结果覆盖多组返回
   var dbOperator = new DBOperator();
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, null, null, 4000, 1000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, null, null, 4000, 1000);
   checkCount(expResult, actResult);
   println("===skip + limit < 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, null, null, 5000, 5000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, null, null, 5000, 5000);
   checkCount(expResult, actResult);
   println("===skip + limit = 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, null, null, 9000, 9000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, null, null, 9000, 9000);
   checkCount(expResult, actResult);
   println("===skip + limit > 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, null, null, 8000, 15000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, null, null, 8000, 15000);
   checkCount(expResult, actResult);
   println("===skip > 10000 && limit < 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, null, null, 12000, 18000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1}, null, 12000, 18000);
   checkCount(expResult, actResult);
   println("===skip > 10000 && limit > 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, null, null, 15000, 9000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, null, null, 15000, 9000);
   checkCount(expResult, actResult);
   println("===skip < 10000 && limit > 10000 success===");
      
   //skip/limit/sort组合查询结果覆盖单组返回
   println("======skip/limit/sort======");
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, {"_id" : 1}, null, 4000, 1000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, {"_id" : 1}, null, 4000, 1000);
   checkResult(expResult, actResult);
   println("===skip + limit < 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, {"_id" : 1}, null, 5000, 5000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, {"_id" : 1}, null, 5000, 5000);
   checkResult(expResult, actResult);
   println("===skip + limit = 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, {"_id" : 1}, null, 6000, 6000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, {"_id" : 1}, null, 6000, 6000);
   checkResult(expResult, actResult);
   println("===skip + limit > 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, {"_id" : 1}, null, 4000, 11000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, {"_id" : 1}, null, 4000, 11000);
   checkResult(expResult, actResult);
   println("===skip > 10000 && limit < 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, {"_id" : 1}, null, 12000, 13000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, {"_id" : 1}, null, 12000, 13000);
   checkResult(expResult, actResult);
   println("===skip > 10000 && limit > 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match" :{"a" : "a"}}}}}, null, {"_id" : 1}, null, 15000, 5000);
   var expResult = dbOperator.findFromCL(dbcl, {"a" : "a"}, null, {"_id" : 1}, null, 15000, 5000);
   checkResult(expResult, actResult);
   println("===skip < 10000 && limit > 10000 success===");
   
   //skip/limit/sort组合查询结果覆盖多组返回
   var dbOperator = new DBOperator();
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1}, null, 4000, 1000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1}, null, 4000, 1000);
   checkResult(expResult, actResult);
   println("===skip + limit < 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1}, null, 5000, 5000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1}, null, 5000, 5000);
   checkResult(expResult, actResult);
   println("===skip + limit = 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1}, null, 9000, 9000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1}, null, 9000, 9000);
   checkResult(expResult, actResult);
   println("===skip + limit > 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1}, null, 8000, 15000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1}, null, 8000, 15000);
   checkResult(expResult, actResult);
   println("===skip > 10000 && limit < 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1}, null, 12000, 18000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1}, null, 12000, 18000);
   checkResult(expResult, actResult);
   println("===skip > 10000 && limit > 10000 success===");
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1}, null, 15000, 9000);
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1}, null, 15000, 9000);
   checkResult(expResult, actResult);
   println("===skip < 10000 && limit > 10000 success===");

   commDropCL(db, COMMCSNAME, clName, true, true);
}

function checkCount(expResult, actResult){
   if (actResult.length != expResult.length){
      throw buildException("checkCount()", "different length between actResult and expResult", "equal length", expResult.length, actResult.length);
   }
}

main()
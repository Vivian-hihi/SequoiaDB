/************************************
*@Description: record include part of text index field, insert/update/delete
*@author:      zhaoyu
*@createdate:  2018.10.11
**************************************/
function main()
{
   var clName = COMMCLNAME + "_ES_11996";
   var clFullName = COMMCSNAME + "." + clName
   var indexName = "abc";
   
   commDropCL( db, COMMCSNAME, clName);
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   commCreateIndex( dbcl, indexName, {a:"text",b:"text",c:"text"});
   dbcl.insert({a:"text",d:"text"});
   
   var esOperator = new ESOperator();
   var dbOperator = new DBOperator();
   var eSIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, indexName);
   checkFullSyncToES(COMMCSNAME, clName, indexName, 1);
   
   var expectRecords = dbOperator.findFromCL(dbcl, {a:{$type:2,$et:"string"}});
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}});
   checkResult(expectRecords, actRecords);
   println("---check insert success---");
   
   dbcl.update({$set:{a:"update"}});
   checkFullSyncToES(COMMCSNAME, clName, indexName, 1);
   var expectRecords = dbOperator.findFromCL(dbcl, {a:{$type:2,$et:"string"}});
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}});
   checkResult(expectRecords, actRecords);
   println("---check update success---");
   
   dbcl.remove();
   checkFullSyncToES(COMMCSNAME, clName, indexName, 0);
   var expectRecords = dbOperator.findFromCL(dbcl);
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}});
   checkResult(expectRecords, actRecords);
   println("---check remove success---");
   
   commDropCL( db, COMMCSNAME, clName);
}
main()
/************************************
*@Description: create fullText index 16 keys£¬insert/update/delete
*@author:      zhaoyu
*@createdate:  2018.10.11
**************************************/
function main()
{
   var clName = COMMCLNAME + "_ES_12005_2";
   var clFullName = COMMCSNAME + "." + clName
   var indexName = "a";
   var doc = [{No:1,a1:"text",a2:"text",a3:"text",a4:"text",a5:"text",a6:"text",a7:"text",a8:"text",a9:"text",a10:"text",a11:"text",a12:"text",a13:"text",a14:"text",a15:"text",a16:"text"}];
   
   commDropCL( db, COMMCSNAME, clName);
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   commCreateIndex( dbcl, indexName, {a1:"text",a2:"text",a3:"text",a4:"text",a5:"text",a6:"text",a7:"text",a8:"text",a9:"text",a10:"text",a11:"text",a12:"text",a13:"text",a14:"text",a15:"text",a16:"text"});
   dbcl.insert(doc);
   
   var esOperator = new ESOperator();
   var dbOperator = new DBOperator();
   var eSIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, indexName);
   checkFullSyncToES(COMMCSNAME, clName, indexName, 1);
   
   var expectRecords = dbOperator.findFromCL(dbcl);
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}});
   checkResult(expectRecords, actRecords);
   println("---check insert success---");
   
   dbcl.update({$set:{a1:"update"}},{a1:{$exists:1}});
   checkFullSyncToES(COMMCSNAME, clName, indexName, 1);
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match:{a1:"update"}}}}});
   var expectRecords = dbOperator.findFromCL(dbcl, {a1:"update"});
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
main();
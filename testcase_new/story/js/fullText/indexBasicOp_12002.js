/************************************
*@Description: fullText index and common index on the same key£¬insert/update/delete/query
*@author:      zhaoyu
*@createdate:  2018.10.11
**************************************/
function main()
{
   if( commIsStandalone( db ) )
   {
      println( "Deploy mode is standalone!" );
      return;
   }
   
   var clName = COMMCLNAME + "_ES_12002";
   var clFullName = COMMCSNAME + "." + clName
   var textIndexName = "fullIndex";
   var commIndexName = "commIndex";
   
   commDropCL( db, COMMCSNAME, clName);
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   commCreateIndex( dbcl, textIndexName, {a:"text"});
   commCreateIndex( dbcl, commIndexName, {a:1});
   dbcl.insert({a:"text"});
   
   var esOperator = new ESOperator();
   var dbOperator = new DBOperator();
   var eSIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, textIndexName);
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 1);
   
   var expectRecords = dbOperator.findFromCL(dbcl, {a:"text"}, null, null, {"":"commIndex"});
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}});
   checkResult(expectRecords, actRecords);
   println("---check insert success---");
   
   dbcl.update({$set:{a:"update"}},{a:"text"});
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 1);
   var expectRecords = dbOperator.findFromCL(dbcl, {a:"update"}, null, null, {"":"commIndex"});
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match:{a:"update"}}}}});
   checkResult(expectRecords, actRecords);
   println("---check update success---");
   
   dbcl.remove();
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 0);
   var expectRecords = dbOperator.findFromCL(dbcl);
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}});
   checkResult(expectRecords, actRecords);
   println("---check remove success---");
   
   commDropCL( db, COMMCSNAME, clName);
}
main()
/***************************************************************************
@Description :seqDB-12078 :集合中的全文索引字段与普通索引字段相同，对该字段进行事务回滚  
@Modify list :
              2018-11-06  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }

   var clName = COMMCLNAME + "_ES_12078";
   var csName = "testCS_ES_12078";
   commDropCS( db, csName );
   
   //创建全文索引及普通索引，索引字段相同
   var dbcl = commCreateCL( db, csName, clName );
   commCreateIndex( dbcl, "fullIndex", {content : "text"});
   commCreateIndex( dbcl, "commIndex", {content : 1});
   
   //insert
   db.transBegin();
   var records = new Array();
   for (var i = 0; i < 10 ; i++){
      var record = {content : "a" + i, age : i + 10};
      records.push(record);
   }
   dbcl.insert(records);
   db.transRollback();
   
   checkFullSyncToES(csName, clName, "fullIndex", 0);
   checkConsistency(csName, clName, 5);
   
   var dbOperator = new DBOperator();
   var expResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {content : ""});
   var esOperator = new ESOperator();
   var esIndexName = dbOperator.getESIndexName(csName, clName, "fullIndex");
   var queryCond = '{"query" : {"exists" : {"field" : "content"}}}';
   var actResult = esOperator.findFromES(esIndexName, queryCond);
   
   actResult.sort(compare("content"));
   expResult.sort(compare("content"));
   println("expResult : " + JSON.stringify(expResult));
   println("actResult : " + JSON.stringify(actResult));
   checkResult(expResult, actResult);
   println("===insert success===");
   
   //update
   db.transBegin();
   dbcl.update({$set : {content : "i can not do it"}}, {content : "a2"});
   db.transRollback();
   
   checkFullSyncToES(csName, clName, "fullIndex", 0);
   checkConsistency(csName, clName, 5);
   var expResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {content : ""});
   var actResult = esOperator.findFromES(esIndexName, queryCond);
   
   actResult.sort(compare("content"));
   expResult.sort(compare("content"));
   checkResult(expResult, actResult);
   println("===update success===");
   
   //delete
   db.transBegin();
   dbcl.remove({content : "a3"});
   db.transRollback();
   
   checkFullSyncToES(csName, clName, "fullIndex", 0);
   checkConsistency(csName, clName, 5);
   var expResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {content : ""});
   var actResult = esOperator.findFromES(esIndexName, queryCond);
   
   actResult.sort(compare("content"));
   expResult.sort(compare("content"));
   checkResult(expResult, actResult);
   println("===delete success===");
   
   //truncate
   db.transBegin();
   dbcl.truncate();
   db.transRollback();
   
   checkFullSyncToES(csName, clName, "fullIndex", 0);
   checkConsistency(csName, clName, 5);
   var expResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {content : ""});
   var actResult = esOperator.findFromES(esIndexName, queryCond);
   
   actResult.sort(compare("content"));
   expResult.sort(compare("content"));
   checkResult(expResult, actResult);
   println("===truncate success===");

   commDropCS( db, csName );
}

main()
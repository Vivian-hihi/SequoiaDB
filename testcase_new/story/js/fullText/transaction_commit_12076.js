/***************************************************************************
@Description :seqDB-12076 :集合中的全文索引字段与普通索引字段相同，对该字段进行事务提交     
@Modify list :
              2018-10-26  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };

   var clName = COMMCLNAME + "_ES_12076";
   var csName = "testCS_ES_12076";
   commDropCL(db, COMMCSNAME, clName, true, true);
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
   db.transCommit();
   
   checkFullSyncToES(csName, clName, "fullIndex", 10);
   checkConsistency(csName, clName, 5);
   
   var dbOperator = new DBOperator();
   var expResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {content : ""}, {"content" : 1});
   var esOperator = new ESOperator();
   var esIndexName = dbOperator.getESIndexName(csName, clName, "fullIndex");
   var queryCond = '{"query" : {"exists" : {"field" : "content"}}}';
   var actResult = esOperator.findFromES(esIndexName, queryCond);
   
   actResult.sort(compare("content"));
   checkResult(expResult, actResult);
   
   //update
   db.transBegin();
   dbcl.update({$set : {content : "i can not do it"}}, {content : "a2"});
   db.transCommit();
   
   checkFullSyncToES(csName, clName, "fullIndex", 10);
   checkConsistency(csName, clName, 5);
   var expResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {content : ""}, {"content" : 1});
   var actResult = esOperator.findFromES(esIndexName, queryCond);
   
   actResult.sort(compare("content"));
   checkResult(expResult, actResult);
   
   //delete
   db.transBegin();
   dbcl.remove({content : "a3"});
   db.transCommit();
   
   checkFullSyncToES(csName, clName, "fullIndex", 9);
   checkConsistency(csName, clName, 5);
   var expResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {content : ""}, {"content" : 1});
   var actResult = esOperator.findFromES(esIndexName, queryCond);
   
   actResult.sort(compare("content"));
   checkResult(expResult, actResult);
   
   //truncate
   db.transBegin();
   dbcl.truncate();
   db.transCommit();
   
   checkFullSyncToES(csName, clName, "fullIndex", 0);
   checkConsistency(csName, clName, 5);
   var expResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {content : ""}, {"content" : 1});
   var actResult = esOperator.findFromES(esIndexName, queryCond);
   
   actResult.sort(compare("content"));
   checkResult(expResult, actResult);

   commDropCL(db, COMMCSNAME, clName, true, true);
   commDropCS( db, csName );
}

main()
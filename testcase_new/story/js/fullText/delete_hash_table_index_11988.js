/***************************************************************************
@Description :seqDB-11988 :hash切分表加入域使用自动切分，创建/删除全文索引 
@Modify list :
              2018-11-02  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }

   var clName = COMMCLNAME + "_ES_11988";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, {ShardingType : "hash", ShardingKey : {a : 1}, AutoSplit : true} );
   
   //插入数据，数据分布覆盖：1个组、多个组上
   var records = new Array();
   for (var i = 0; i < 1 ; i++){
      var record = {a : "a" + i, b : "b" + i};
      records.push(record);
   }
   insertRecords(dbcl, records);
   
   if(1 != dbcl.count())
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }
   
   //数据分布覆盖：1个组，索引字段覆盖：非分区键
   commCreateIndex( dbcl, "fullIndex", {b : "text"});
   commCheckIndex( dbcl, "fullIndex", true );
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex", 1);
   
   var dbOperator = new DBOperator();
   var cappedCL = dbOperator.getCappedCLs( COMMCSNAME, clName, "fullIndex" );
   var cappedCL = cappedCL[0];
   var count = cappedCL.count();
   if (count != 0){
      throw buildException("main()", "cappedCL is not empty", "equal", 0, count);
   }
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   checkConsistency(COMMCSNAME, clName, 5);
   
   commDropIndex( dbcl, "fullIndex" );
   commCheckIndex( dbcl, "fullIndex", false );
   
   try{
      cappedCL.insert({a:"a"});	   
      throw e;
   }
   catch (e){
      if (e != -34){
         throw buildException("main()", "cappedCL is not removed", "equal", -34, e);
      }
   } 
   
   try{
      checkFullSyncToES(COMMCSNAME, clName, "fullIndex", 0);
      throw e;
   }
   catch( e ){
      if (e != -47){
         throw buildException("main()", "es index do not delete", "equal", "delete", "not delete");
      }
   }
   checkConsistency(COMMCSNAME, clName, 5);
   println("================================One Group Not on ShardingKey================================");
   
   //数据分布覆盖：1个组，索引字段覆盖：分区键
   commCreateIndex( dbcl, "fullIndex", {a : "text"});
   commCheckIndex( dbcl, "fullIndex", true );
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex", 1);
   
   var cappedCL = dbOperator.getCappedCLs( COMMCSNAME, clName, "fullIndex" );
   var cappedCL = cappedCL[0];
   var count = cappedCL.count();
   if (count != 0){
      throw buildException("main()", "cappedCL is not empty", "equal", 0, count);
   }
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   checkConsistency(COMMCSNAME, clName, 5);
   
   commDropIndex( dbcl, "fullIndex" );
   commCheckIndex( dbcl, "fullIndex", false );
   
   try{
      cappedCL.insert({a:"a"});	   
      throw e;
   }
   catch (e){
      if (e != -34){
         throw buildException("main()", "cappedCL is not removed", "equal", -34, e);
      }
   } 
   
   try{
      checkFullSyncToES(COMMCSNAME, clName, "fullIndex", 0);
      throw e;
   }
   catch( e ){
      if (e != -47){
         throw buildException("main()", "es index do not delete", "equal", "delete", "not delete");
      }
   }
   checkConsistency(COMMCSNAME, clName, 5);
   println("================================One Group on ShardingKey================================");
   
   var records = new Array();
   for (var i = 0; i < 9999 ; i++){
      var record = {a : "a" + i, b : "b" + i};
      records.push(record);
   }
   insertRecords(dbcl, records);
   
   if(10000 != dbcl.count())
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }
   //数据分布覆盖：多个组，索引字段覆盖：非分区键
   commCreateIndex( dbcl, "fullIndex", {b : "text"});
   commCheckIndex( dbcl, "fullIndex", true );
   
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex", 10000);
   
   var cappedCL = dbOperator.getCappedCLs( COMMCSNAME, clName, "fullIndex" );
   var cappedCL = cappedCL[0];
   var count = cappedCL.count();
   if (count != 0){
      throw buildException("main()", "cappedCL is not empty", "equal", 0, count);
   }
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   checkConsistency(COMMCSNAME, clName, 5);
   
   commDropIndex( dbcl, "fullIndex" );
   commCheckIndex( dbcl, "fullIndex", false );
   
   try{
      cappedCL.insert({a:"a"});	   
      throw e;
   }
   catch (e){
      if (e != -34){
         throw buildException("main()", "cappedCL is not removed", "equal", -34, e);
      }
   } 
   
   try{
      checkFullSyncToES(COMMCSNAME, clName, "fullIndex", 0);
      throw e;
   }
   catch( e ){
      if (e != -47){
         throw buildException("main()", "es index do not delete", "equal", "delete", "not delete");
      }
   }
   checkConsistency(COMMCSNAME, clName, 5);
   println("================================Many Group Not on ShardingKey================================");
   
   //数据分布覆盖：多个组，索引字段覆盖：分区键
   commCreateIndex( dbcl, "fullIndex", {a : "text"});
   commCheckIndex( dbcl, "fullIndex", true );
   
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex", 10000);
   
   var cappedCL = dbOperator.getCappedCLs( COMMCSNAME, clName, "fullIndex" );
   var cappedCL = cappedCL[0];
   var count = cappedCL.count();
   if (count != 0){
      throw buildException("main()", "cappedCL is not empty", "equal", 0, count);
   }
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   checkConsistency(COMMCSNAME, clName, 5);
   
   commDropIndex( dbcl, "fullIndex" );
   commCheckIndex( dbcl, "fullIndex", false );
   
   try{
      cappedCL.insert({a:"a"});	   
      throw e;
   }
   catch (e){
      if (e != -34){
         throw buildException("main()", "cappedCL is not removed", "equal", -34, e);
      }
   } 
   
   try{
      checkFullSyncToES(COMMCSNAME, clName, "fullIndex", 0);
      throw e;
   }
   catch( e ){
      if (e != -47){
         throw buildException("main()", "es index do not delete", "equal", "delete", "not delete");
      }
   }
   checkConsistency(COMMCSNAME, clName, 5);
   println("================================Many Group on ShardingKey================================");
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

main()
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
   
   var groups = commGetGroups( db );
   if(groups.length < 2 ){
      println("Deploy one group");
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
   
   //数据分布覆盖：1个组，索引字段覆盖：非分区键
   commCreateIndex( dbcl, "fullIndex1", {b : "text"});
   commCheckIndex( dbcl, "fullIndex1", true );
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex1", 1);
   
   var dbOperator = new DBOperator();
   var cappedCL = dbOperator.getCappedCLs( COMMCSNAME, clName, "fullIndex1" );
   var cappedCL = cappedCL[0];
   var count = cappedCL.count();
   if (count != 0){
      throw buildException("main()", "cappedCL is not empty", "equal", 0, count);
   }
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   checkConsistency(COMMCSNAME, clName);
   checkInspectResult(COMMCSNAME, clName, 5);
   
   var esIndexNames = dbOperator.getESIndexNames(COMMCSNAME, clName, "fullIndex1");
   commDropIndex( dbcl, "fullIndex1" );
   commCheckIndex( dbcl, "fullIndex1", false );
   checkIndexNotExistInES(esIndexNames);
   checkConsistency(COMMCSNAME, clName);
   checkInspectResult(COMMCSNAME, clName, 5);
   println("================================One Group Not on ShardingKey================================");
   
   //数据分布覆盖：1个组，索引字段覆盖：分区键
   commCreateIndex( dbcl, "fullIndex2", {a : "text"});
   commCheckIndex( dbcl, "fullIndex2", true );
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex2", 1);
   
   var cappedCL = dbOperator.getCappedCLs( COMMCSNAME, clName, "fullIndex2" );
   var cappedCL = cappedCL[0];
   var count = cappedCL.count();
   if (count != 0){
      throw buildException("main()", "cappedCL is not empty", "equal", 0, count);
   }
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   checkConsistency(COMMCSNAME, clName);
   checkInspectResult(COMMCSNAME, clName, 5);
   
   var esIndexNames = dbOperator.getESIndexNames(COMMCSNAME, clName, "fullIndex2");
   commDropIndex( dbcl, "fullIndex2" );
   commCheckIndex( dbcl, "fullIndex2", false );
   checkIndexNotExistInES(esIndexNames);
   checkConsistency(COMMCSNAME, clName);
   checkInspectResult(COMMCSNAME, clName, 5);
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
   commCreateIndex( dbcl, "fullIndex3", {b : "text"});
   commCheckIndex( dbcl, "fullIndex3", true );
   
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex3", 10000);
   
   var cappedCL = dbOperator.getCappedCLs( COMMCSNAME, clName, "fullIndex3" );
   var cappedCL = cappedCL[0];
   var count = cappedCL.count();
   if (count != 0){
      throw buildException("main()", "cappedCL is not empty", "equal", 0, count);
   }
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   checkConsistency(COMMCSNAME, clName);
   checkInspectResult(COMMCSNAME, clName, 5);
   
   var esIndexNames = dbOperator.getESIndexNames(COMMCSNAME, clName, "fullIndex3");
   commDropIndex( dbcl, "fullIndex3" );
   commCheckIndex( dbcl, "fullIndex3", false );
   checkIndexNotExistInES(esIndexNames);
   checkConsistency(COMMCSNAME, clName);
   checkInspectResult(COMMCSNAME, clName, 5);
   println("================================Many Group Not on ShardingKey================================");
   
   //数据分布覆盖：多个组，索引字段覆盖：分区键
   commCreateIndex( dbcl, "fullIndex4", {a : "text"});
   commCheckIndex( dbcl, "fullIndex4", true );
   
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex4", 10000);
   
   var cappedCL = dbOperator.getCappedCLs( COMMCSNAME, clName, "fullIndex4" );
   var cappedCL = cappedCL[0];
   var count = cappedCL.count();
   if (count != 0){
      throw buildException("main()", "cappedCL is not empty", "equal", 0, count);
   }
   
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   checkConsistency(COMMCSNAME, clName);
   checkInspectResult(COMMCSNAME, clName, 5);
   
   var esIndexNames = dbOperator.getESIndexNames(COMMCSNAME, clName, "fullIndex4");
   commDropIndex( dbcl, "fullIndex4" );
   commCheckIndex( dbcl, "fullIndex4", false );
   checkIndexNotExistInES(esIndexNames);
   checkConsistency(COMMCSNAME, clName);
   checkInspectResult(COMMCSNAME, clName, 5);
   println("================================Many Group on ShardingKey================================");
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

main()

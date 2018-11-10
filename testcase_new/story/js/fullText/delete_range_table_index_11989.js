/***************************************************************************
@Description :seqDB-11989 :range切分表中创建/删除全文索引 
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
   
   var clName = COMMCLNAME + "_ES_11989";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, {ShardingType : "range", ShardingKey : {a : 1}, Group : groups[0][0]["GroupName"]} );
   //插入数据，数据分布覆盖：1个组、多个组上
   dbcl.split(groups[0][0]["GroupName"], groups[1][0]["GroupName"], {a : "c"}, {a : "g"});  

   var records = new Array();
   for (var i = 0; i < 60 ; i++){
      var record = {a : "a" + i, b : "b" + i};
      records.push(record);
   }
   
   insertRecords(dbcl, records);
   
   //数据分布覆盖：1个组，索引字段覆盖：非分区键
   commCreateIndex( dbcl, "fullIndex1", {b : "text"});
   commCheckIndex( dbcl, "fullIndex1", true );
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex1", 60);
   
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
   
   var esIndexNames = dbOperator.getESIndexNames(COMMCSNAME, clName, "fullIndex1");
   commDropIndex( dbcl, "fullIndex1" );
   commCheckIndex( dbcl, "fullIndex1", false );
   checkIndexNotExistInES(COMMCSNAME, clName, esIndexNames);
   checkConsistency(COMMCSNAME, clName);
   println("================================One Group Not on ShardingKey================================");
   
   //数据分布覆盖：1个组，索引字段覆盖：分区键
   commCreateIndex( dbcl, "fullIndex2", {a : "text"});
   commCheckIndex( dbcl, "fullIndex2", true );
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex2", 60);
   
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
   
   var esIndexNames = dbOperator.getESIndexNames(COMMCSNAME, clName, "fullIndex2");
   commDropIndex( dbcl, "fullIndex2" );
   commCheckIndex( dbcl, "fullIndex2", false );
   checkIndexNotExistInES(COMMCSNAME, clName, esIndexNames);
   checkConsistency(COMMCSNAME, clName);
   println("================================One Group on ShardingKey================================");
   
   var records = new Array();
   for (var i = 0; i < 40 ; i++){
      var record = {a : "f" + i, b : "g" + i};
      records.push(record);
   }
   dbcl.insert(records);
   
   //数据分布覆盖：多个组，索引字段覆盖：非分区键
   commCreateIndex( dbcl, "fullIndex3", {b : "text"});
   commCheckIndex( dbcl, "fullIndex3", true );
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex3", 100);
   
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
   
   var esIndexNames = dbOperator.getESIndexNames(COMMCSNAME, clName, "fullIndex3");
   commDropIndex( dbcl, "fullIndex3" );
   commCheckIndex( dbcl, "fullIndex3", false );
   checkIndexNotExistInES(COMMCSNAME, clName, esIndexNames);
   checkConsistency(COMMCSNAME, clName);
   println("================================Many Group Not on ShardingKey================================");
   
   //数据分布覆盖：多个组，索引字段覆盖：分区键
   commCreateIndex( dbcl, "fullIndex4", {a : "text"});
   commCheckIndex( dbcl, "fullIndex4", true );
   
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex4", 100);
   
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
   
   var esIndexNames = dbOperator.getESIndexNames(COMMCSNAME, clName, "fullIndex4");
   commDropIndex( dbcl, "fullIndex4" );
   commCheckIndex( dbcl, "fullIndex4", false );
   checkIndexNotExistInES(COMMCSNAME, clName, esIndexNames);
   checkConsistency(COMMCSNAME, clName);
   println("================================Many Group on ShardingKey================================");
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

main()
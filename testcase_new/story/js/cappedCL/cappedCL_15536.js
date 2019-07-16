/***************************************************************************
@Description :seqDB-15536 :固定集合空间上创建主表 
@Modify list :
              2018-11-27  YinZhen  Create
****************************************************************************/
//main();

function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   //在固定集合空间上创建主表
   var clName = COMMCAPPEDCLNAME + "_15536";
   var slaveCLName1 = COMMCAPPEDCLNAME + "slave1_cl_15536";
   var slaveCLName2 = COMMCAPPEDCLNAME + "slave2_cl_15536";
   var mainCL = db.getCS(COMMCAPPEDCSNAME).createCL(clName, {ShardingKey : {a : 1}, ShardingType : "range", IsMainCL : true});
   commDropCL(db, COMMCSNAME, slaveCLName1, true, true);
   commDropCL(db, COMMCSNAME, slaveCLName2, true, true);
   var slaveCL1 = commCreateCL(db, COMMCSNAME, slaveCLName1);
   var slaveCL2 = commCreateCL(db, COMMCSNAME, slaveCLName2);
   mainCL.attachCL(COMMCSNAME + "." + slaveCLName1, {LowBound : {a : 0}, UpBound : {a : 4567}});
   mainCL.attachCL(COMMCSNAME + "." + slaveCLName2, {LowBound : {a : 4567}, UpBound : {a : 10001}});
   
   //createIndex
   var textIndexName = "fullIndex_15536";
   commCreateIndex( mainCL, textIndexName, {b : "text"});
   
   //insert
   var records = new Array();
   var count = 0;
   for (var i = 0; i < 10000 ; i++){
      var randomNum = parseInt(Math.random()*10000 + 1);
      if (randomNum < 4567){
         count++;
      }
      var record = {a : randomNum, b : "b" + i};
      records.push(record);
   }
   mainCL.insert(records);
   checkMainCLFullSyncToES(COMMCAPPEDCSNAME, clName, textIndexName, 10000);
   
   //query
   var dbOperator = new DBOperator();
   var actResult = dbOperator.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(mainCL, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   
   //update
   mainCL.update({$set : {b : "helloworld"}}, {b : "b1"});
   mainCL.insert({a : 4568, b : "b4568"});
   checkMainCLFullSyncToES(COMMCAPPEDCSNAME, clName, textIndexName, 10001);
   var actResult = dbOperator.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(mainCL, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   
   //delete
   mainCL.remove({b : "helloworld"});
   checkMainCLFullSyncToES(COMMCAPPEDCSNAME, clName, textIndexName, 10000);
   var actResult = dbOperator.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(mainCL, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   
   var esIndexNames1 = dbOperator.getESIndexNames(COMMCSNAME, slaveCLName1, textIndexName);
   var esIndexNames2 = dbOperator.getESIndexNames(COMMCSNAME, slaveCLName2, textIndexName);
   commDropCL(db, COMMCSNAME, slaveCLName1, true, true);
   commDropCL(db, COMMCSNAME, slaveCLName2, true, true);
   commDropCL( db, COMMCAPPEDCSNAME, clName, true, true, "drop CL in the end");
   //SEQUOIADBMAINSTREAM-3983
   checkIndexNotExistInES(esIndexNames1);
   checkIndexNotExistInES(esIndexNames2);
}

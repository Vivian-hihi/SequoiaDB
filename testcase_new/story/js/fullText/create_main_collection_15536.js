/***************************************************************************
@Description :seqDB-15536 :固定集合空间上创建主表 
@Modify list :
              2018-11-27  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   //创建固定集合空间，在固定集合空间上创建主表
   var csName = "capped_cs_15536";
   commDropCS( db, csName );
   initCappedCS(csName);
   var clName = COMMCLNAME + "_ES_15536";
   var mainCL = db.getCS(csName).createCL(clName, {ShardingKey : {a : 1}, ShardingType : "range", IsMainCL : true});
   var slaveCLName1 = "slave1_cl_15536";
   commDropCL(db, COMMCSNAME, slaveCLName1, true, true);
   var slaveCL1 = commCreateCL(db, COMMCSNAME, slaveCLName1);
   var slaveCLName2 = "slave2_cl_15536";
   commDropCL(db, COMMCSNAME, slaveCLName2, true, true);
   var slaveCL2 = commCreateCL(db, COMMCSNAME, slaveCLName2);
   mainCL.attachCL(COMMCSNAME + "." + slaveCLName1, {LowBound : {a : 0}, UpBound : {a : 4567}});
   mainCL.attachCL(COMMCSNAME + "." + slaveCLName2, {LowBound : {a : 4567}, UpBound : {a : 10001}});
   
   //createIndex
   commCreateIndex( mainCL, "fullIndex_15536", {b : "text"});
   
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
   insertRecords(mainCL, records);
   
   if(10000 != mainCL.count())
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }
   checkMainCLFullSyncToES(csName, clName, "fullIndex_15536", 10000);
   
   //query
   var dbOperator = new DBOperator();
   var actResult = dbOperator.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(mainCL, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   
   //update
   mainCL.update({$set : {b : "helloworld"}}, {b : "b1"});
   mainCL.insert({a : 4568, b : "b4568"});
   checkMainCLFullSyncToES(csName, clName, "fullIndex_15536", 10001);
   var actResult = dbOperator.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(mainCL, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   
   //delete
   mainCL.remove({b : "helloworld"});
   checkMainCLFullSyncToES(csName, clName, "fullIndex_15536", 10000);
   var actResult = dbOperator.findFromCL(mainCL, {"" : {$Text : {"query" : {"match_all" : {}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(mainCL, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   
   commDropCL(db, COMMCSNAME, slaveCLName1, true, true);
   commDropCL(db, COMMCSNAME, slaveCLName2, true, true);
   commDropCS( db, csName );
}

function initCappedCS( csName )
{
   commDropCS( db, csName, true, "drop CS in the beginning" );
   //create cappedCS
   var options = { Capped : true }
   commCreateCS( db, csName, false, "beginning to create cappedCS", options );
}

main()

/***************************************************************************
@Description :seqDB-12069 :集合空间中包含部分子表，且子表中创建了全文索引，删除集合空间 
@Modify list :
              2018-11-27  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_ES_12069";
   commDropCL(db, COMMCSNAME, clName, true, true);
   var mainCL = commCreateCLByOption( db, COMMCSNAME, clName, {ShardingKey : {a : 1}, ShardingType : "range", IsMainCL : true});
   var csName1 = "slave1_cs_12069";
   commDropCS( db, csName1 );
   var slaveCLName1 = "slave1_cl_12069";
   var slaveCL1 = commCreateCL(db, csName1, slaveCLName1);
   var csName2 = "slave2_cs_12069";
   commDropCS( db, csName2 );
   var slaveCLName2 = "slave2_cl_12069";
   var slaveCL2 = commCreateCL(db, csName2, slaveCLName2);
   mainCL.attachCL(csName1 + "." + slaveCLName1, {LowBound : {a : 0}, UpBound : {a : 4567}});
   mainCL.attachCL(csName2 + "." + slaveCLName2, {LowBound : {a : 4567}, UpBound : {a : 10001}});
   
   //create index
   commCreateIndex( mainCL, "fullIndex_12069", {b : "text"});
   
   //insert records
   var records = new Array();
   var oneSubCLCount = 0;
   for (var i = 0; i < 10000 ; i++){
      var randomNum = parseInt(Math.random()*10000 + 1);
      if (randomNum < 4567){
         oneSubCLCount++;
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
   checkMainCLFullSyncToES(COMMCSNAME, clName, "fullIndex_12069", 10000)
   
   //删除部分子表所在的集合空间
   db.dropCS(csName1);
   checkMainCLFullSyncToES(COMMCSNAME, clName, "fullIndex_12069", 10000 - oneSubCLCount);
   
   //其余子表主备节点数据一致
   checkConsistency(csName2, slaveCLName2);
   checkInspectResult(csName2, slaveCLName2, 5);
   
   var dbOperator = new DBOperator();
   var expResult = dbOperator.findFromCL(slaveCL2, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {b : ""});
   var esIndexNames = dbOperator.getESIndexNames(csName2, slaveCLName2, "fullIndex_12069"); 
   var actResult = new Array();
   var esOperator = new ESOperator();
   for (var i  in esIndexNames){
      var esRecords = esOperator.findFromES(esIndexNames[i], '{"query":{"match_all":{}}, "size":10000}');
      actResult = actResult.concat(esRecords);
   }
   
   expResult.sort(compare("b"));
   actResult.sort(compare("b"));
   checkResult(expResult, actResult);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   commDropCS( db, csName1 );
   commDropCS( db, csName2 );
}

main()
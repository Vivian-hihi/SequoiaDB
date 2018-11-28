/***************************************************************************
@Description :seqDB-12070 : 集合空间中包含主表，且通过主表创建了全文索引，删除集合空间 
@Modify list :
              2018-11-27  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_ES_12070";
   var csName = "main_cs_12070";
   commDropCS( db, csName );
   var mainCL = commCreateCLByOption( db, csName, clName, {ShardingKey : {a : 1}, ShardingType : "range", IsMainCL : true});
   var csName1 = "slave1_cs_12070";
   commDropCS( db, csName1 );
   var slaveCLName1 = "slave1_cl_12070";
   var slaveCL1 = commCreateCL(db, csName1, slaveCLName1);
   var csName2 = "slave2_cs_12070";
   commDropCS( db, csName2 );
   var slaveCLName2 = "slave2_cl_12070";
   var slaveCL2 = commCreateCL(db, csName2, slaveCLName2);
   mainCL.attachCL(csName1 + "." + slaveCLName1, {LowBound : {a : 0}, UpBound : {a : 4567}});
   mainCL.attachCL(csName2 + "." + slaveCLName2, {LowBound : {a : 4567}, UpBound : {a : 10001}});
   
   commCreateIndex( mainCL, "fullIndex_12070", {b : "text"});
   
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
   checkMainCLFullSyncToES(csName, clName, "fullIndex_12070", 10000)
   
   //删除主表所在的集合空间
   db.dropCS(csName);
   checkFullSyncToES(csName1, slaveCLName1, "fullIndex_12070", count);
   checkFullSyncToES(csName2, slaveCLName2, "fullIndex_12070", 10000 - count);
   
   //其余子表主备节点数据一致
   checkConsistency(csName1, slaveCLName1);
   checkInspectResult(csName1, slaveCLName1, 5);
   checkConsistency(csName2, slaveCLName2);
   checkInspectResult(csName2, slaveCLName2, 5);
   
   var dbOperator = new DBOperator();
   var expResult = dbOperator.findFromCL(slaveCL2, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {b : ""});
   var esIndexNames = dbOperator.getESIndexNames(csName2, slaveCLName2, "fullIndex_12070");
   
   var actResult = new Array();
   var esOperator = new ESOperator();
   for (var i  in esIndexNames){
      var esRecords = esOperator.findFromES(esIndexNames[i], '{"query":{"match_all":{}}, "size":10000}');
      actResult = actResult.concat(esRecords);
   }
   expResult.sort(compare("b"));
   actResult.sort(compare("b"));
   checkResult(expResult, actResult);
   
   expResult = dbOperator.findFromCL(slaveCL1, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {b : ""});
   esIndexNames = dbOperator.getESIndexNames(csName1, slaveCLName1, "fullIndex_12070");
   
   var actResult = new Array();
   for (var i  in esIndexNames){
      var esRecords = esOperator.findFromES(esIndexNames[i], '{"query":{"match_all":{}}, "size":10000}');
      actResult = actResult.concat(esRecords);
   }
   expResult.sort(compare("b"));
   actResult.sort(compare("b"));
   checkResult(expResult, actResult);
   
   commDropCS( db, csName );
   commDropCS( db, csName1 );
   commDropCS( db, csName2 );
}

main()
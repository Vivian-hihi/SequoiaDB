/***************************************************************************
@Description :seqDB-12072 :通过主表创建了全文索引，删除主表 
@Modify list :
              2018-11-27  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   //主表及子表在相同集合空间上
   var clName = COMMCLNAME + "_ES_12072";
   commDropCL(db, COMMCSNAME, clName, true, true);
   var mainCL = commCreateCLByOption( db, COMMCSNAME, clName, {ShardingKey : {a : 1}, ShardingType : "range", IsMainCL : true});
   var slaveCLName1 = "slave1_cl_12072";
   commDropCL(db, COMMCSNAME, slaveCLName1, true, true);
   var slaveCL1 = commCreateCL(db, COMMCSNAME, slaveCLName1);
   var slaveCLName2 = "slave2_cl_12072";
   commDropCL(db, COMMCSNAME, slaveCLName2, true, true);
   var slaveCL2 = commCreateCL(db, COMMCSNAME, slaveCLName2);
   mainCL.attachCL(COMMCSNAME + "." + slaveCLName1, {LowBound : {a : 0}, UpBound : {a : 4567}});
   mainCL.attachCL(COMMCSNAME + "." + slaveCLName2, {LowBound : {a : 4567}, UpBound : {a : 10001}});
   
   commCreateIndex( mainCL, "fullIndex_12072", {b : "text"});
   
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
   checkMainCLFullSyncToES(COMMCSNAME, clName, "fullIndex_12072", 10000)
   
   //删除某个子表
   var dbOperator = new DBOperator();
   var esIndexNames = dbOperator.getESIndexNames(COMMCSNAME, slaveCLName1, "fullIndex_12072");
     
   db.getCS(COMMCSNAME).dropCL(slaveCLName1);     
   checkIndexNotExistInES(COMMCSNAME, slaveCLName1, esIndexNames);
   checkFullSyncToES(COMMCSNAME, slaveCLName2, "fullIndex_12072", 10000 - count);
   
   var expResult = dbOperator.findFromCL(slaveCL2, {"" : {$Text : {"query" : {"match_all" : {}}}}}, {b : ""});
   esIndexNames = dbOperator.getESIndexNames(COMMCSNAME, slaveCLName2, "fullIndex_12072"); 
   var actResult = new Array();
   var esOperator = new ESOperator();
   for (var i  in esIndexNames){
      var esRecords = esOperator.findFromES(esIndexNames[i], '{"query":{"match_all":{}}, "size":10000}');
      actResult = actResult.concat(esRecords);
   }
   
   expResult.sort(compare("b"));
   actResult.sort(compare("b"));
   checkResult(expResult, actResult);
   
   checkConsistency(COMMCSNAME, slaveCLName2);
   checkInspectResult(COMMCSNAME, slaveCLName2, 5);
   
   commDropCL(db, COMMCSNAME, slaveCLName1, true, true);
   commDropCL(db, COMMCSNAME, slaveCLName2, true, true);
   commDropCL(db, COMMCSNAME, clName, true, true);
}

main()
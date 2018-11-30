/***************************************************************************
@Description :seqDB-12071 :通过主表创建了全文索引，删除主表 
@Modify list :
              2018-11-27  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_ES_12071";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //主表及子表在相同和不同的集合空间上
   commDropCL(db, COMMCSNAME, clName, true, true);
   var mainCL = commCreateCLByOption( db, COMMCSNAME, clName, {ShardingKey : {a : 1}, ShardingType : "range", IsMainCL : true});
   var slaveCLName1 = "slave1_cl_12071";
   commDropCL(db, COMMCSNAME, slaveCLName1, true, true);
   var slaveCL1 = commCreateCL(db, COMMCSNAME, slaveCLName1);
   var csName = "slave2_cs_12071";
   commDropCS( db, csName );
   var slaveCLName2 = "slave2_cl_12071";
   var slaveCL2 = commCreateCL(db, csName, slaveCLName2);
   mainCL.attachCL(COMMCSNAME + "." + slaveCLName1, {LowBound : {a : 0}, UpBound : {a : 4567}});
   mainCL.attachCL(csName + "." + slaveCLName2, {LowBound : {a : 4567}, UpBound : {a : 10001}});
   
   //create index
   commCreateIndex( mainCL, "fullIndex_12071", {b : "text"});
   
   //insert records
   var records = new Array();
   for (var i = 0; i < 10000 ; i++){
      var randomNum = parseInt(Math.random()*10000 + 1);
      var record = {a : randomNum, b : "b" + i};
      records.push(record);
   }
   insertRecords(mainCL, records);
   
   if(10000 != mainCL.count())
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }
   checkMainCLFullSyncToES(COMMCSNAME, clName, "fullIndex_12071", 10000)
   
   //删除主表
   var dbOperator = new DBOperator();
   var esIndexNames1 = dbOperator.getESIndexNames(COMMCSNAME, slaveCLName1, "fullIndex_12071");
   var esIndexNames2 = dbOperator.getESIndexNames(csName, slaveCLName2, "fullIndex_12071");
   
   db.getCS(COMMCSNAME).dropCL(clName);   
   checkIndexNotExistInES(COMMCSNAME, slaveCLName1, esIndexNames1);
   checkIndexNotExistInES(csName, slaveCLName2, esIndexNames2);
   
   try{
      slaveCL1.insert({a : 1});
      throw "INSERT ERROR"
   }
   catch (e){
      if (-23 != e){
         throw buildException("main()","slaveCL1 should be removed but not","-23 equal e", -23, e);
      }
   }
   try{
      slaveCL2.insert({a : 1});
      throw "INSERT ERROR"
   }
   catch (e){
      if (-23 != e){
         throw buildException("main()","slaveCL2 should be removed but not","-23 equal e", -23, e);
      }
   }
   
   commDropCL(db, COMMCSNAME, slaveCLName1, true, true);
   commDropCL(db, COMMCSNAME, clName, true, true);
   commDropCS( db, csName );
}

main()
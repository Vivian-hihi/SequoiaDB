/***************************************************************************
@Description :seqDB-17979 :编目节点组通过选举切主后，创建全文索引
@Modify list :
              2019-4-26  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }

   var clName = COMMCLNAME + "_ES_17979";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var groups = commGetGroups( db );
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, {'Group':groups[0][0]["GroupName"]});
   
   // 插入数据
   var records = new Array();
   for (var i = 0; i < 10000 ; i++){
      var record = {a : "a" + i, b : "b" + i};
      records.push(record);
   }
   dbcl.insert(records);
   
   // 重新选主
   var rg = db.getRG(groups[0][0]["GroupName"]);
   rg.reelect({Seconds:60});
   
   commCreateIndex( dbcl, "fullIndex_17979", {b : "text"});
   checkFullSyncToES(COMMCSNAME, clName, "fullIndex_17979", 10000);
   var dbOperator = new DBOperator();
   var actResult = dbOperator.findFromCL(dbcl, {"" : {$Text : {"query" : {"match_all" :{}}}}}, null, {"_id" : 1});
   var expResult = dbOperator.findFromCL(dbcl, null, null, {"_id" : 1});
   checkResult(expResult, actResult);
   
   commDropCL( db, COMMCSNAME, clName );
}

main();

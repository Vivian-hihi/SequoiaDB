/***************************************************************************
@Description :seqDB-14395 :在cont使用全文检索 
@Modify list :
              2018-9-30  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "_ES_12395";   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引，并插入包含索引字段的记录，记录总数大于1万条 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   var fullIndex = "fullIndex_ES_12395";
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   
   var dataGenerator = new commDataGenerator();
   var records = dataGenerator.getRecords(30000, "string", ["about", "content"]);
   insertRecords(dbcl, records);
   
   var recordNum = parseInt(dbcl.count());
   if(recordNum !== 30000){
      println("insert has an error : SEQUOIADBMAINSTREAM-3827");
	  return;
   }
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 30000);
   
   //在count命令字中使用全文检索执行查询，覆盖：无记录匹配、部分记录匹配(大于1w条)、记录全匹配(大于1w条)，检查结果
   var actCount = dbcl.find({"" : {$Text : {"query" : {"match" : {"content" : "movie"}}}}}).count();
   checkAllResult(actCount, 0);
   
   var dbOperator = new DBOperator();
   var actCount = dbcl.find({"" : {$Text : {"query" : {"match_all" : {}}}}, content : {$gt : "c"}}).count();
   var expCount = dbOperator.findFromCL(dbcl, {"content" : {$gt : "c"}}, null, null, null).length;
   checkAllResult(actCount, expCount);
   
   var actCount = dbcl.find({"" : {$Text : {"query" : {"match_all" : {}}}}}).count();
   checkAllResult(actCount, 30000);
         
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function checkAllResult(actCount, expCount){
   if(actCount != expCount){
      throw buildException( "count ", null, "dbcl.find().count()", "success", "fail" );
   }
   else{
      println("check records success!")
   }
}

main();

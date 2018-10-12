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
   
   var clName = COMMCLNAME + "12395ES";
   var fullIndex = "fullindex12395es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引，并插入包含索引字段的记录，记录总数大于1万条 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   var records = insertData(dbcl);
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 30000);
   
   //在count命令字中使用全文检索执行查询，覆盖：无记录匹配、部分记录匹配(大于1w条)、记录全匹配(大于1w条)，检查结果
   var actCount = dbcl.find({"" : {$Text : {"query" : {"match" : {"content" : "movie"}}}}}).count();
   checkAllResult(actCount, 0);
   
   var actCount = dbcl.find({"" : {$Text : {"query" : {"match_all" : {}}}}, content : {$gt : "c"}}).count();
   var expCount = dbOperator.findFromCL(dbcl, {"content" : {$gt : "c"}}, null, null, null).length;
   checkAllResult(actCount, expCount);
   
   var actCount = dbcl.find({"" : {$Text : {"query" : {"match_all" : {}}}}}).count();
   checkAllResult(actCount, 30000);
         
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function insertData(dbcl){
   var dataGenerator = new commDataGenerator();
   var masRecords = dataGenerator.getRecords(30000, "string", ["about", "content"]);
   var slaRecords = dataGenerator.getRecords(30000, "string", ["about", "content"]);
   for(var i in masRecords){
      masRecords[i].about = masRecords[i].about + slaRecords[i].about;
      masRecords[i].content = masRecords[i].content + slaRecords[i].content;
   }
   insertRecords(dbcl, masRecords);
   return masRecords;
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

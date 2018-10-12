/***************************************************************************
Description :seqDB-12064 :带sort进行全文检索 
@Modify list :
              2018-9-28  YinZhen  Create
****************************************************************************/
function main(){
	
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };

   var clName = COMMCLNAME + "12064ES";
   var fullIndex = "fullindex12064es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}};
   var sortConf = {about : 1, content : 1};
   var hintConf = {"" : fullIndex};
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引，并插入包含索引字段的记录 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   var records = insertData(dbcl);
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 30000);
   
   //使用全文索引字段进行查询并使用sort,返回的记录数超过1万条，检查结果
   var actRecords = dbOperator.findFromCL(dbcl, findConf, null, sortConf, hintConf);
   var expRecords = dbOperator.findFromCL(dbcl, null, null, sortConf, null);
   
   checkResult(actRecords, expRecords);   
   
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

main();

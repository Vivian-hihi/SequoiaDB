/***************************************************************************
@Description :seqDB-15768 :全文索引字段同时为唯一索引，更新使记录的唯一索引重复 
@Modify list :
              2018-10-09  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "15768ES";
   var fullIndex = "fullindex15768es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var queryCond = '{"query" : {"exists" : {"field" : "content"}}}'; 
   var findConf = {"" : {$Text : {"query" : {"exists" : {"field" : "content"}}}}};
      
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建集合并创建全文索引且同时为唯一索引 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   commCreateIndex(dbcl, "contentIndex", {content : 1}, true);
   
   //插入包含全文索引的记录 
   var records = insertRecords(dbcl);
   
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 2);
   
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   
   var expESRecords = getExpESRecords();
   var expCLRecords = records;
   
   //记录插入成功，原始集合、固定集合及ES端记录正确
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   
   //更新使记录的唯一索引字段与与集合中其它记录的唯一索引重复
   updateRecords(dbcl);
   
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   
   //更新失败，报错-38，检查原始集合、固定集合及主备数据节点数据无差别
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   checkConsistency(COMMCSNAME, clName, 5);
   
   commDropCL(db, COMMCSNAME, clName, true, true);

}

function insertRecords(dbcl){
   var records = new Array();
   records[0] = {about : "about for you", content : "this is my college"};
   records[1] = {about : "how it go on", content : "this is my hometown"};
   dbcl.insert(records);
   return records;
}

function getExpESRecords(){
   var expESRecords = new Array();
   expESRecords.push({about : "about for you", content : "this is my college"});
   expESRecords.push({about : "how it go on", content : "this is my hometown"});
   return expESRecords;
}

function updateRecords(dbcl){
   try{
   dbcl.update({$set : {content : "this is my hometown"}},{content : "this is my college"});
   }
   catch(e){
      if(e != -38){
         throw buildException("insert()", e, "dbcl insert duplicate records on unique index", "success", "fail");
      }
   }
}
function checkRecords( expRecords, actRecords )
{
   var fields = new Array();
   if(expRecords.length > 0){
	   for(var i in expRecords[0]){
		   fields.push(i);
	   }
   }
   var actRec = new Array();
   for(var i in actRecords){
	   var obj = new Object();
	   for(var j in fields){
		   obj[fields[j]] = actRecords[i][fields[j]];
	   }
	   actRec.push(obj);
   }
   if(fields.length > 0){
	  var sortField = fields[0];
	  expRecords.sort(compare(sortField));
	  actRec.sort(compare(sortField));
   }
   checkResult(expRecords, actRec)
}

main();

/***************************************************************************
@Description :seqDB-15762 :集合中已存在记录，创建全文索引字段为唯一索引 
@Modify list :
              2018-10-09  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "15762ES";
   var fullIndex = "fullindex15762es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var queryCond = '{"query" : {"exists" : {"field" : "content"}}}'; 
   var findConf = {"" : {$Text : {"query" : {"exists" : {"field" : "content"}}}}};
      
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建集合并创建全文索引
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   
   //插入两条相同且包含全文索引字段的记录
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
   
   //创建全文索引字段为唯一索引，检查结果
   createUniqueIndex(dbcl);
   
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   
   //唯一索引创建失败，报错-38，检查集合索引、原始集合、固定集合记录无变化使用inspect工具检测主备节点数据一致，ES上记录无变化 
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   checkConsistency(COMMCSNAME, clName, 5);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function insertRecords(dbcl){
   var records = new Array();
   records[0] = {about : "about for you", content : "this is my college"};
   records[1] = {about : "about for you", content : "this is my college"};
   dbcl.insert(records);
   return records;
}

function getExpESRecords(){
   var expESRecords = new Array();
   expESRecords.push({about : "about for you", content : "this is my college"});
   expESRecords.push({about : "about for you", content : "this is my college"});
   return expESRecords;
}

function createUniqueIndex(dbcl){
   try{
      dbcl.createIndex("contentIndex", {content : 1}, true);
   }
   catch(e){
      if(e != -38){
         throw buildException("createIndex()", e, "dbcl exists duplicate records, create unique index", "success", "fail");
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

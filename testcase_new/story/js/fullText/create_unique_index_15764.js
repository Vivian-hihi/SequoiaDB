/***************************************************************************
@Description :seqDB-15764 :集合中已存在记录，创建其它字段为唯一索引 
@Modify list :
              2018-10-09  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "_ES_15764";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建集合并创建全文索引
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   var fullIndex = "fullIndex_ES_15764";
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   
   //插入非全文索引字段重复的记录
   var records = new Array();
   records[0] = {about : "about for you", content : "this is my college", name : "zsan"};
   records[1] = {about : "how it go on", content : "this is my hometown", name : "zsan"};
   dbcl.insert(records);
   
   var dbOperator = new DBOperator();
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 2);
   
   var esOperator = new ESOperator();
   var queryCond = '{"query" : {"exists" : {"field" : "content"}}}'; 
   var findConf = {"" : {$Text : {"query" : {"exists" : {"field" : "content"}}}}};
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   
   var expESRecords = new Array();
   expESRecords.push({about : "about for you", content : "this is my college"});
   expESRecords.push({about : "how it go on", content : "this is my hometown"});
   var expCLRecords = records;
   
   //记录插入成功，检查原始集合、固定集合及ES中已同步记录
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   
   //创建步骤2存在重复记录的字段为唯一索引，检查结果
   createDuplicateIndex(dbcl);
   
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   
   //唯一索引创建失败，报错-38，检查集合索引、原始集合、固定集合及ES的记录无变化，使用inspect工具检测主备数据节点数据无差别
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   checkConsistency(COMMCSNAME, clName, 5);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function createDuplicateIndex(dbcl){
   try{
      dbcl.createIndex("nameIndex", {name : 1}, true);
   }
   catch(e){
      if(e != -38){
         throw buildException("creatIndex()", e, "dbcl create unique index on duplicate field", "success", "fail");
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

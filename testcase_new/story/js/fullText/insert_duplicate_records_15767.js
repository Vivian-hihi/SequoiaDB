/***************************************************************************
@Description :seqDB-15767 :全文索引及唯一索引字段为不同字段，插入唯一索引重复记录 
@Modify list :
              2018-10-09  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "_ES_15767";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建集合并创建全文索引，另创建其他字段为唯一索引 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   var fullIndex = "fullIndex_ES_15767";
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   commCreateIndex(dbcl, "nameIndex", {name : 1}, true);
   
   //插入的记录包含唯一索引及全文索引字段
   var records = new Array();
   records[0] = {about : "about for you", content : "this is my college", name : "zsan"};
   records[1] = {about : "how it go on", content : "this is my hometown", name : "lisi"};
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
   
   //记录插入成功，固定集合记录插入成功，ES同步记录成功
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   
   //重复步骤2 ，检查结果
   insertRecordsAgain(dbcl, records);
   
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   
   //重复插入报错-38，检查原始集合及各数据节点固定集合记录，无记录被删除，固定集合中未新增操作记录，ES 上记录未被删除 。
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   checkConsistency(COMMCSNAME, clName, 5);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function insertRecordsAgain(dbcl, records){
   try{
      dbcl.insert(records);
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

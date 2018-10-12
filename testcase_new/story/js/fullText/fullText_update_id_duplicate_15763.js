/***************************************************************************
@Description :seqDB-15763 :更新使记录的_id索引重复 
@Modify list :
              2018-10-09  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "15763ES";
   var fullIndex = "fullindex15763es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var queryCond = '{"query" : {"exists" : {"field" : "content"}}}'; 
   var findConf = {"" : {$Text : {"query" : {"exists" : {"field" : "content"}}}}};
      
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建集合并创建全文索引
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   
   //插入包含_id索引和全文索引的记录
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
   
   //更新_id字段的值与集合中其它任意一条记录的_id索引重复，检查结果
   updateFieldId(dbcl);
   
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   
   //报错-38，检查原始集合、固定集合及ES记录的_id索引未被更新，使用inspect工具查看主备数据节点数据无差别
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   checkConsistency(COMMCSNAME, clName, 5);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function insertRecords(dbcl){
   var records = new Array();
   records[0] = {_id : 1001, about : "about for you", content : "this is my college"};
   records[1] = {_id : 1002, about : "how it go on", content : "this is my hometown"};
   dbcl.insert(records);
   return records;
}

function getExpESRecords(){
   var expESRecords = new Array();
   expESRecords.push({about : "about for you", content : "this is my college"});
   expESRecords.push({about : "how it go on", content : "this is my hometown"});
   return expESRecords;
}

function updateFieldId(dbcl){
   try{
      dbcl.update({$set : {_id : 1002}},{_id : 1001});
   }
   catch(e){
      if(e != -38){
         throw buildException("update()", e, "dbcl update _id duplicate to another record", "success", "fail");
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

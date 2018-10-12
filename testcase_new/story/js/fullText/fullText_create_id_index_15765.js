/***************************************************************************
@Description :seqDB-15765 :集合中已存在_id字段重复的记录，创建id索引 
@Modify list :
              2018-10-09  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "15765ES";
   var fullIndex = "fullindex15765es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var queryCond = '{"query" : {"exists" : {"field" : "content"}}}'; 
   var findConf = {"" : {$Text : {"query" : {"exists" : {"field" : "content"}}}}};
      
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建集合并创建全文索引(AutoIndexId为false)
   var dbcl = commCreateCLByOption(db, COMMCSNAME, clName, {ReplSize : 0, AutoIndexId : false});
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   
   //插入_id重复的记录
   var records = insertRecords(dbcl);
   
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 1);
   
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   
   var expESRecords = getExpESRecords();
   var expCLRecords = records;
   
   //记录插入成功，原始集合、固定集合及ES端记录正确
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   
   //创建_id索引,检查结果
   createCLIdIndex(dbcl);
   
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   
   //id索引创建失败，报错-38，原始集合、固定集合及ES端记录不变
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function insertRecords(dbcl){
   var records = new Array();
   records[0] = {_id : 1001, about : "about for you", content : "this is my college"};
   records[1] = {_id : 1001, about : "this for you", content : "this is my hometown"};
   dbcl.insert(records);
   return records;
}

function getExpESRecords(){
   var expESRecords = new Array();
   expESRecords.push({about : "this for you", content : "this is my hometown"});
   return expESRecords;
}

function createCLIdIndex(dbcl){
   try{
      dbcl.createIdIndex();
   }
   catch(e){
      if(e != -38){
         throw buildException("createIdIndex()", e, "dbcl exists duplicate _id, create _id index", "success", "fail");
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

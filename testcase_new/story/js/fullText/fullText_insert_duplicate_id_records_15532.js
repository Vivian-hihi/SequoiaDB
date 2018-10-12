/***************************************************************************
@Description :seqDB-15532 :插入_id字段重复的记录 
@Modify list :
              2018-10-09  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "15532ES";
   var fullIndex = "fullindex15532es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var queryCond = '{"query" : {"exists" : {"field" : "content"}}}'; 
      
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建集合并创建全文索引
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   
   //指定_id插入记录，覆盖：包含全文索引字段、不包含全文索引字段，检查结果
   var records = insertRecords(dbcl);
   
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 1);
   
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   
   var expESRecords = getExpESRecords();
   
   //记录插入成功，固定集合及ES端记录正确
   checkRecords( expESRecords,  actESRecords);
   
   //再次执行步骤1，检查结果
   insertRecordsAgain(dbcl, records);
   
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   
   //记录插入失败，固定集合及ES端记录无变化
   checkRecords( expESRecords,  actESRecords);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function insertRecords(dbcl){
   var records = new Array();
   records[0] = {_id : 1001, about : "about for you", content : "this is my college", name : "zsan", age : 18};
   records[1] = {_id : 1002, name : "lisi", age : 20, addr : "JinagXi"};
   dbcl.insert(records);
   return records;
}

function getExpESRecords(){
   var expESRecords = new Array();
   expESRecords.push({about : "about for you", content : "this is my college"});
   return expESRecords;
}

function insertRecordsAgain(dbcl, records){
   try{
      dbcl.insert(records);
   }
   catch(e){
      if(e != -38){
         throw buildException("insert()", e, "insert duplicate _id", "success", "fail");
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

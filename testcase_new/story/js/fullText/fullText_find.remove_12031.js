/***************************************************************************
@Description :seqDB-12031 :使用find.remove删除记录 
@Modify list :
              2018-9-30  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "12031ES";
   var fullIndex = "fullindex12031es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}};
   var selectorConf = {about : "", content : ""};
   var queryCond = '{"query" : {"exists" : {"field" : "about"}}, "size" : 20}';
      
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   
   //插入包含全文索引字段的记录 
   var records = insertRecords(dbcl);
   
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   var cappedCL = dbOperator.getCappedCL(COMMCSNAME, clName, fullIndex);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 10);
   
   //使用find.remove接口删除记录，检查结果 
   var cursor = dbcl.find({about : {"$gt": "about1"}}).remove(); //dbcl.find({about : {"$gt": "about1"}}).remove(); can not remove records
   while(cursor.next()){}

   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 2);
   
   //记录更新成功，固定集合中记录正确，操作类型正确，es中记录最终与原集合一致
   var count = cappedCL.find({Type : 2}).count();
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   
   if (count != 8){
      throw buildException( "Count ", null, "cappedCL.find({Type : 2}).count();", "success", "fail" );
   }
   
   checkRecords( actESRecords, actCLRecords );
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function insertRecords(dbcl){
   var records = new Array();
   for (var i = 0; i < 10; i++){
      records[i] = {about : "about" + i, content : "content" + i};
   }
   dbcl.insert(records);
   return records;
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

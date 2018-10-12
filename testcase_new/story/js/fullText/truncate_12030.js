/***************************************************************************
@Description :seqDB-12030 :使用truncate删除记录 
@Modify list :
              2018-10-08  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "_ES_12030";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   var fullIndex = "fullIndex_ES_12030";
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   
   //插入包含全文索引字段的记录 
   var dataGenerator = new commDataGenerator();
   var records = dataGenerator.getRecords(10, "string", ["about", "content"]);
   dbcl.insert(records);
   
   var dbOperator = new DBOperator();
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   var cappedCL = dbOperator.getCappedCL(COMMCSNAME, clName, fullIndex);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 10);
   
   //使用truncate删除记录，检查结果    
   dbcl.truncate();
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 0);
   
   //记录删除成功，原始集合、固定集合、ES中记录被全文清除，记录数为0
   var esOperator = new ESOperator();
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}}
   var queryCond = '{"query" : {"exists" : {"field" : "about"}}, "size" : 30}';
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   var actCappedCLRecords = dbOperator.findFromCL(cappedCL, null, null, null, null);
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   
   var expRecords = new Array();

   checkRecords( expRecords,  actCLRecords);
   checkRecords( expRecords,  actCappedCLRecords);
   checkRecords( expRecords,  actESRecords);
      
   commDropCL(db, COMMCSNAME, clName, true, true);
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

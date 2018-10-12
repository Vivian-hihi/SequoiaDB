/***************************************************************************
@Description :seqDB-12038 :普通索引字段同时是全文索引字段，在该字段上执行查询 
@Modify list :
              2018-9-29  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "_ES_12038";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引同时在该字段上创建索引
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   var fullIndex = "fullIndex_ES_12038";
   commCreateIndex(dbcl, fullIndex, {content : "text"});
   commCreateIndex(dbcl, "commonIndex", {content : 1});
   
   //插入包含全文索引字段的记录 
   var dataGenerator = new commDataGenerator();
   var records = dataGenerator.getRecords(20, "string", ["about", "content"]);
   dbcl.insert(records);
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 20);
   
   //在索引字段上执行查询，覆盖:普通查询、全文检索，检查结果 
   var dbOperator = new DBOperator();
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}};
   var actRecordsFullSearch = dbOperator.findFromCL(dbcl, findConf, null, null, {"" : fullIndex});
   var actRecordsCommSearch = dbOperator.findFromCL(dbcl, null, null, null, {"" : "commonIndex"});
   
   var expRecords = records;
   
   checkRecords( expRecords, actRecordsFullSearch );
   checkRecords( expRecords, actRecordsCommSearch );
   
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

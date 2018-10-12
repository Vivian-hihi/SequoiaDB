/***************************************************************************
@Description :seqDB-12037 :在全文索引字段上执行普通查询 
@Modify list :
              2018-9-29  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "_ES_12037";      
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引，并插入包含全文索引字段的记录 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   var fullIndex = "fullIndex_ES_12037";
   commCreateIndex(dbcl, fullIndex, {content : "text"});
   var dataGenerator = new commDataGenerator();
   var records = dataGenerator.getRecords(20, "string", ["about", "content"]);
   dbcl.insert(records);
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 20);
   
   //在全文索引字段上执行普通查询，查询时全文索引字段用于：条件、选择、排序，检查结果 
   var dbOperator = new DBOperator();
   var actRecords = dbOperator.findFromCL(dbcl, {content : {$exists : 1}}, {content : ""}, {content : 1}, null);
   
   var expRecords = new Array();
   for (var i in records){
      var obj = new Object();
      obj.content = records[i].content;
      expRecords.push(obj);
   }
   expRecords.sort(compare("content"));
   
   //在原集合上执行查询，查询结果正确
   checkRecords( expRecords, actRecords );
   
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

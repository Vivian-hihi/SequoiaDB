/***************************************************************************
@Description :seqDB-12033 :删除不存在的记录 
@Modify list :
              2018-9-30  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "12033ES";
   var fullIndex = "fullindex12033es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}}
   var queryCond = '{"query" : {"exists" : {"field" : "about"}}, "size" : 30}';
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   
   var dataGenerator = new commDataGenerator();
   var records = dataGenerator.getRecords(20, "string", ["about", "content"]);
   dbcl.insert(records);
   
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 20);
   
   //删除不存在的记录，检查结果 
   dbcl.remove({about : "what about you ", content : "this is my content"});
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 20);
   
   //命令行执行成功，固定集合中不新增记录，es中最终与原集合数据一致
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var expCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   checkRecords( actESRecords,  expCLRecords);
      
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

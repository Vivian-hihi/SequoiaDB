/***************************************************************************
@Description :seqDB-14373 :插入重复的记录 
@Modify list :
              2018-9-30  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "14373ES";
   var fullIndex = "fullindex14373es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}}
   var queryCond = '{"query" : {"exists" : {"field" : "content"}}}';
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   
   //插入多条全文索引字段重复的记录
   insertData(dbcl);
   
   //创建全文索引，再次插入多条全文索引字段重复的记录
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   insertData(dbcl);
   
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 6);
   
   //记录插入成功，固定集合中记录正确，ES中记录正确
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var expCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   checkRecords( actESRecords,  expCLRecords);
      
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function insertData(dbcl){
   var records = new Array();
   for(var i = 0; i < 3; i++){
      records[i] = {about : "my name is zsan", content : "this is my college"};
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

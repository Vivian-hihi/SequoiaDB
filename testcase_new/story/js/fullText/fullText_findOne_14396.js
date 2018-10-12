/***************************************************************************
@Description :seqDB-14396 :在findOne中使用全文检索 
@Modify list :
              2018-9-30  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "14396ES";
   var fullIndex = "fullindex14396es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引及普通索引，并插入包含索引字段的记录 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {about : "text"});
   commCreateIndex(dbcl, "commIndex", {about : 1});
   var records = insertData(dbcl);
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 5);
   
   //在findOne中使用全文检索条件，覆盖：只带全文检索条件、混合查询，检查结果 
   var actOnlyFullRecords = getOnlyFullRecords(dbcl);
   var actMixRecords = getMixRecords(dbcl);
   
   var expRecords = new Array();
   expRecords.push(records[0]);
   
   checkRecords( expRecords,  actOnlyFullRecords);
   checkRecords( expRecords,  actMixRecords);
      
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function insertData(dbcl){
   var records = new Array();
   for (var i = 0; i < 5;i++){
      records[i] = {about : "i am a robot " + i};
   }
   dbcl.insert(records);
   return records;
}

function getOnlyFullRecords(dbcl){
   var records = dbcl.findOne({"" : {$Text : {"query" : {"match_all" : {}}}}});
   var actOnlyFullRecords = new Array();
   while(records.next()){
      actOnlyFullRecords.push(records.current().toObj());
   }
   return actOnlyFullRecords;
}

function getMixRecords(dbcl){
   var records = dbcl.findOne({"" : {$Text : {"query" : {"match_all" : {}}}}}, {about : ""}).sort({about : 1});
   var actMixRecords = new Array();
   while(records.next()){
      actMixRecords.push(records.current().toObj());
   }
   return actMixRecords;
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

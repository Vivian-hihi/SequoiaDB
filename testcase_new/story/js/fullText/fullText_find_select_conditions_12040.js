/***************************************************************************
@Description :seqDB-12040 :带选择条件进行全文检索 
@Modify list :
              2018-9-29  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "12040ES";
   var fullIndex = "fullindex12040es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //在同一字段上创建全文索引及普通索引，并插入包含索引字段的记录 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {content : "text", about : "text"});
   commCreateIndex(dbcl, "commIndex", {content : 1});
   
   var dataGenerator = new commDataGenerator();
   var records = dataGenerator.getRecords(30, "string", ["about", "content", "information"]);
   dbcl.insert(records);
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 30);
   
   //使用全文索引字段进行查询并对字段进行选择，查询覆盖：普通查询、全文检索，选择符进行抽测，选择字段覆盖：_id、部分全文索引字段、全部索引字段、其他字段，检查结果 
   var actRecords = getRecordsFullSearch(fullIndex, dbOperator, dbcl);
   var expRecords = records;
   checkRecords( expRecords, actRecords );
   
   var actRecords = getRecordsCommSearch(fullIndex, dbOperator, dbcl);
   var expRecords = getExpRecords(records);
   checkRecords( expRecords, actRecords );
      
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function getRecordsFullSearch(fullIndex, dbOperator, dbcl){
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}};
   var selectorConf = {_id : {$include : 1}, about : {$include : 1}, content : {$default : "this is content "}, information : ""};
   var hintConf = {"" : fullIndex};
   var actRecordsFullSearch = dbOperator.findFromCL(dbcl, findConf, selectorConf, null, hintConf);
   return actRecordsFullSearch;
}

function getRecordsCommSearch(fullIndex, dbOperator, dbcl){
   var selectorConf = {about : {$include : 1}, information : {$default : "this is information "}};
   var hintConf = {"" : "commIndex"};
   var actRecordsCommSearch = dbOperator.findFromCL(dbcl, null, selectorConf, null, hintConf);
   return actRecordsCommSearch;
}

function getExpRecords(records){
   var expRecordsCommSearch = new Array();
   for (var i in records){
      var obj = new Object();
      obj.about = records[i].about;
      obj.information = records[i].information;
      expRecordsCommSearch.push(obj);
   }
   return expRecordsCommSearch;
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

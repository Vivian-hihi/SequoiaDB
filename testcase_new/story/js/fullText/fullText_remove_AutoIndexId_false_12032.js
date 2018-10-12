/***************************************************************************
@Description :seqDB-12032 :AutoIndexId为false时删除记录 
@Modify list :
              2018-9-30  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "12032ES";
   var fullIndex = "fullindex12032es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}}
   var queryCond = '{"query" : {"exists" : {"field" : "content"}}}';
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建集合指定AutoIndexId为false    
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, {ReplSize : 0, AutoIndexId : false});
   
   //创建全文索引，并插入包含全文索引字段的记录 
   commCreateIndex(dbcl, fullIndex, {content : "text"});
   var records = insertData(dbcl);
   
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 1);
   
   //删除记录，检查结果
   removeRecords(dbcl);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 1);
   
   //记录删除失败，固定集合中记录正确，ES中记录正确
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var expCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   checkRecords( actESRecords,  expCLRecords);
   
   //创建id索引后，再次删除记录，检查结果
   dbcl.createIdIndex();
   dbcl.remove({content : "this is my college"});
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 0);
   
   //记录删除成功，ES中最终无记录
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var expCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   checkRecords( actESRecords,  expCLRecords);
      
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function insertData(dbcl){
   var records = new Array();
   records[0] = {content : "this is my college"};
   dbcl.insert(records);
   return records;
}

function removeRecords(dbcl){
   try{
      dbcl.remove({content : "this is my college"});
   }
   catch(e){
      if(e == "-279"){
         println("can not update/delete records when $id index does not exist");
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

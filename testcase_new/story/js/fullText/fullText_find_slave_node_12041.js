/***************************************************************************
@Description :seqDB-12041 :查询在备节点进行全文检索 
@Modify list :
              2018-9-29  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "12041ES";
   var fullIndex = "fullindex12041es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引，并插入包含索引字段的记录 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   var dataGenerator = new commDataGenerator();
   var records = dataGenerator.getRecords(30, "string", ["about", "content"]);
   dbcl.insert(records);
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 30);
   
   //指定索引字段进行全文检索，指定查询覆盖：走主节点、走备节点，检查结果 
   db.setSessionAttr({PreferedInstance : "M"});
   var masActRecords = getActRecords(fullIndex, dbOperator, dbcl);
   var expRecords = records;
   checkRecords( expRecords, masActRecords );
   
   db.setSessionAttr({PreferedInstance : "S"});
   var slaActRecords = getActRecords(fullIndex, dbOperator, dbcl);
   checkRecords( expRecords, slaActRecords );
      
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function getActRecords(fullIndex, dbOperator, dbcl){
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}};
   var hintConf = {"" : fullIndex};
   var actRecords = dbOperator.findFromCL(dbcl, findConf, null, null, hintConf);
   return actRecords;
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

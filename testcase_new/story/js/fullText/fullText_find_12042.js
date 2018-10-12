/***************************************************************************
@Description :seqDB-12042 :全文检索返回多条记录 
@Modify list :
              2018-9-28  YinZhen  Create
****************************************************************************/
function main(){

   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "12042ES";
   var fullIndex = "fullindex12042es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}};
   var hintConf = {"" : fullIndex};
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引，并插入包含索引字段的记录 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   var records = insertData(dbcl);
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 40000);
   
   //指定索引字段进行全文检索，返回记录数覆盖与ES需要多次交互(超过1w条)，检查结果
   var actRecords = dbOperator.findFromCL(dbcl, findConf, null, null, hintConf);
   var expRecords = records;
   
   checkRecords( expRecords,  actRecords);
         
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function insertData(dbcl){
   var dataGenerator = new commDataGenerator();
   var masRecords = dataGenerator.getRecords(40000, "string", ["about", "content"]);
   var slaRecords = dataGenerator.getRecords(40000, "string", ["about", "content"]);
   for(var i in masRecords){
      masRecords[i].about = masRecords[i].about + slaRecords[i].about;
      masRecords[i].content = masRecords[i].content + slaRecords[i].content;
   }
   insertRecords(dbcl, masRecords);
   return masRecords;
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
   expRecords.sort(compare("about", compare("content")));
   actRec.sort(compare("about", compare("content")));
   checkResult(expRecords, actRec)
}

main();

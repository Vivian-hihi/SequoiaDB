/***************************************************************************
@Description :seqDB-12034 :重复删除同一条记录 
@Modify list :
              2018-9-28  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "_ES_12034";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建全文索引 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   var fullIndex = "fullIndex_ES_12034";
   commCreateIndex(dbcl, fullIndex, {name : "text"});
   
   var records = new Array();
   for (var i = 0; i < 10; i++){
      records[i] = {name : "name" + i};
   }
   dbcl.insert(records);
   
   var dbOperator = new DBOperator();
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   var cappedCL = dbOperator.getCappedCL(COMMCSNAME, clName, fullIndex);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 10);
   
   //重复删除同一条记录，检查结果
   dbcl.remove({name : "name1"});
   dbcl.remove({name : "name1"});
   
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 9);
   
   //重复删除时，命令行执行成功，固定集合中新增一条Type:2的记录，es中最终与原集合数据一致
   var count = cappedCL.find({Type : 2}).count();
   checkAllResult(count);
      
   var esOperator = new ESOperator();
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}}
   var queryCond = '{"query" : {"exists" : {"field" : "name"}}, "size" : 20}';
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var expCLRecords = dbOperator.findFromCL(dbcl, findConf, null, null, null);
   checkRecords( actESRecords,  expCLRecords);
      
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function checkAllResult(count){
   if(count != 1){
      throw buildException( "Count ", null, "cappedCL.find({Type : 2}).count();", "success", "fail" );
   }
   else{
      println("new {Type : 2} record succeeded");
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

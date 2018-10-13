/***************************************************************************
@Description :seqDB-15765 :集合中已存在_id字段重复的记录，创建id索引 
@Modify list :
              2018-10-09  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "_ES_15765";   
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建集合并创建全文索引(AutoIndexId为false)
   var dbcl = commCreateCLByOption(db, COMMCSNAME, clName, {ReplSize : 0, AutoIndexId : false});
   var fullIndex = "fullIndex_ES_15765";
   commCreateIndex(dbcl, fullIndex, {about : "text", content : "text"});
   
   //插入_id重复的记录
   var records = new Array();
   records[0] = {_id : 1001, about : "about for you", content : "this is my college"};
   records[1] = {_id : 1001, about : "this for you", content : "this is my hometown"};
   dbcl.insert(records);
   
   var dbOperator = new DBOperator();
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 1);
   
   var esOperator = new ESOperator();
   var queryCond = '{"query" : {"exists" : {"field" : "content"}}}'; 
   var findConf = {"" : {$Text : {"query" : {"exists" : {"field" : "content"}}}}};
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, {about : "", content : ""}, null, null);
   
   var expESRecords = new Array();
   expESRecords.push({about : "this for you", content : "this is my hometown"});
   var expCLRecords = new Array();
   expCLRecords.push({about : "about for you", content : "this is my college"});
   expCLRecords.push({about : "this for you", content : "this is my hometown"});
   
   //记录插入成功，原始集合、固定集合及ES端记录正确
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   
   //创建_id索引,检查结果
   createCLIdIndex(dbcl);
   
   var actESRecords = esOperator.findFromES(esIndexName, queryCond);
   var actCLRecords = dbOperator.findFromCL(dbcl, findConf, {about : "", content : ""}, null, null);
   
   //id索引创建失败，报错-38，原始集合、固定集合及ES端记录不变
   checkRecords( expESRecords,  actESRecords);
   checkRecords( expCLRecords,  actCLRecords);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function createCLIdIndex(dbcl){
   try{
      dbcl.createIdIndex();
   }
   catch(e){
      if(e != -38){
         throw buildException("createIdIndex()", e, "dbcl exists duplicate _id, create _id index", "success", "fail");
      }
   }
}

function checkRecords( expRecords, actRecords )
{
   expRecords.sort(compare("about"));
   actRecords.sort(compare("about"));
   checkResult(expRecords, actRecords)
}

main();

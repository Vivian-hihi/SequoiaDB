/***************************************************************************
@Description :seqDB-12037 :在全文索引字段上执行普通查询 
@Modify list :
              2018-10-08  YinZhen  Create
****************************************************************************/
function main(){
   
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };
   
   var clName = COMMCLNAME + "12073ES";
   var fullIndex = "fullindex12073es";
   var dbOperator = new DBOperator();
   var esOperator = new ESOperator();
   var findConf = {"" : {$Text : {"query" : {"match_all" : {}}}}};
   var selectorConf = {about : "", content : ""};
   var queryCond = '{"query" : {"exists" : {"field" : "about"}}, "size" : 20}';
      
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建集合，并创建普通索引及全文索引 
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   commCreateIndex(dbcl, fullIndex, {content : "text"});
   commCreateIndex(dbcl, "commIndex", {content : 1});
   
   //插入lob、记录(包含索引及全文索引的记录) 
   var dataGenerator = new commDataGenerator();
   var records = dataGenerator.getRecords(10, "string", ["content"]);
   dbcl.insert(records);
   dbcl.putLob("/opt/sequoiadb/uninstall.dat");
   
   var esIndexName = dbOperator.getESIndexName(COMMCSNAME, clName, fullIndex);
   var cappedCL = dbOperator.getCappedCL(COMMCSNAME, clName, fullIndex);
   checkFullSyncToES(COMMCSNAME, clName, fullIndex, 10);
   
   //删除集合，检查结果
   var commCS = db.getCS(COMMCSNAME);
   commCS.dropCL(clName);
   
   //集合删除成功，lob文件、索引文件、固定集合文件均被删除，主备节点一致，es中最终无该集合中的记录
   checkAllResult(dbcl, esOperator, cappedCL, esIndexName);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

function checkAllResult(dbcl, esOperator, cappedCL){
   try{
      dbcl.find();
      dbcl.listIndexes();
      dbcl.listLobs();
   }
   catch(e){
      if(e != -23){
         throw buildException("find()", e, "check if it exists", "success", "fail");
      }
   }
   try{
      cappedCL.find();
   }
   catch(e){
      if(e != -23){
         println(e);
         throw buildException("getCappedCL()", e, "check if it exists", "success", "fail");
      }
   }
   sleep(1000);
   try{
      var queryCond = '{"query" : {"match_all" : {}}}';
      esOperator.findFromES(esIndexName, queryCond);
   }
   catch(e){
      if(e != "ReferenceError: esIndexName is not defined"){
         println(e);
         throw buildException("findFromES()", e, "check if it exists", "success", "fail");
      }
   }
}

main();

/***************************************************************************
@Description :seqDB-11993 :创建全文索引接口参数校验 
@Modify list :
              2018-10-25  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }

   var clName = COMMCLNAME + "_ES_11993";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   
   //创建索引类型非法的全文索引
   var indexName = "a";
   try{
      dbcl.createIndex(indexName, {content:"int"});
      throw e;
   }
   catch( e ){
      if( e != -6){
         throw buildException("main()", "create illegal index success", "create full index", "fail to create index", "create index success");
      }
   }
   commCheckIndex( dbcl, indexName, false );
   
   //创建非法的复合索引
   try{
      dbcl.createIndex(indexName, {content:"text", about : 1});
      throw e;
   }
   catch( e ){
      if( e != -6){
         throw buildException("mian()", "create illegal composite index", "create full index", "fail to create index", "create index success");
      }
   }
   commCheckIndex( dbcl, indexName, false );
   
   //指定isUnique、enforced、sortBufferSize创建全文索引
   dbcl.createIndex(indexName, {content : "text"}, true, true, 128);
   commCheckIndex( dbcl, indexName, true );
   
   dbcl.insert([{content:"a"},{content:"a"}]);
   var dbOperator = new DBOperator();
   var actResult = dbOperator.findFromCL(dbcl, null, {content : ""});
   var expResult = [{content:"a"},{content:"a"}];
   
   checkResult(expResult, actResult);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main()
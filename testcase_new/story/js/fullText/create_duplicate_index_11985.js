/***************************************************************************
@Description :seqDB-11985 :创建重复的全文索引 
@Modify list :
              2018-10-25  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }

   var clName = COMMCLNAME + "_ES_11985";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   
   //创建索引名已存在的全文索引
   var indexName = "a";
   dbcl.createIndex( indexName, {about : 1});
   commCheckIndex( dbcl, indexName, true );
   try{
      dbcl.createIndex( indexName, {content:"text"});
      throw e;
   }
   catch( e ){
      if( e != -46){
         throw buildException("main()", "create duplicate index success", "create index", "fail to create index", "create index success");
      }
   }
   
   //在已存在全文索引定义的集合中，再次创建全文索引
   dbcl.createIndex( "b", {content:"text"});
   commCheckIndex( dbcl, "b", true );
   try{
      dbcl.createIndex( "c", {content:"text"});
      throw e ;
   }
   catch( e ){
      if( e != -42){
         throw buildException("main()", "create index on same field", "create full index", "fail to create index", "create index success");
      }
   }
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main()
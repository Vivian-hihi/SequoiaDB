/***************************************************************************
@Description :seqDB-11994 :删除不存在的全文索引 
@Modify list :
              2018-10-25  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }

   var clName = COMMCLNAME + "_ES_11994";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   
   //删除存在的全文索引，删除成功
   var indexName = "a";
   commCreateIndex( dbcl, indexName, {content:"text"});
   commCheckIndex( dbcl, indexName, true );
   dbcl.dropIndex( indexName ); 
   
   //删除不存在的全文索引，删除失败
   commCheckIndex( dbcl, indexName, false );
   try{
      dbcl.dropIndex( indexName ); 
      throw "DROPINDEXERR";
   }
   catch( e ){
      if( e != -47){
         throw buildException("main()", "drop not exist index success", "drop full index", "fail to drop", "drop success");
      }
   }
   commCheckIndex( dbcl, indexName, false );
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main()
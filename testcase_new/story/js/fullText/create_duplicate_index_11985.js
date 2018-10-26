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
   };

   var clName = COMMCLNAME + "_ES_11985";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   
   //创建索引名已存在的全文索引
   var indexName = "a";
   commCreateIndex( dbcl, indexName, {content : 1});
   commCheckIndex( dbcl, indexName, true );
   try{
      commCreateIndex( dbcl, indexName, {content:"text"});
   }
   catch( e ){
      if( e != -46){
         throw buildException("commCreateIndex()", e, "create duplicate index ", "success", "fail");
	  }
   }
   
   //在已存在全文索引定义的集合中，再次创建全文索引
   commCreateIndex( dbcl, "b", {content:"text"});
   commCheckIndex( dbcl, "b", true );
   try{
      commCreateIndex( dbcl, "c", {about:"text"});
   }
   catch( e ){
      if( e != -42){
         throw buildException("commCreateIndex()", e, "create full index twice ", "success", "fail");
	  }
   }
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main()
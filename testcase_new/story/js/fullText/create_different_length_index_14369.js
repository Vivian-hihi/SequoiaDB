/***************************************************************************
@Description :seqDB-14369 :创建全文索引，索引名字段长度验证 
@Modify list :
              2018-10-24  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };

   var clName = COMMCLNAME + "_ES_14369";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   
   //索引名长度为1时，全文索引创建成功
   var indexName = "a"
   commCreateIndex( dbcl, indexName, {content:"text"});
   commCheckIndex( dbcl, indexName, true );
   commDropIndex( dbcl, indexName, true );
   
   //固定集合名长度小于127时，全文索引创建成功
   var indexName = "";
   for (var i = 0; i < 51; i++){
      indexName = indexName + "a";	  
   }
   commCreateIndex( dbcl, indexName, {content:"text"});
   commCheckIndex( dbcl, indexName, true );
   commDropIndex( dbcl, indexName, true );
   
   //固定集合名长度等于127时，全文索引创建成功
   var indexName = "";
   for (var i = 0; i < 111; i++){
      indexName = indexName + "a";	  
   }
   commCreateIndex( dbcl, indexName, {content:"text"});
   commCheckIndex( dbcl, indexName, true );
   commDropIndex( dbcl, indexName, true );
   
   //固定集合名长度大于127时，全文索引创建失败
   var indexName = "";
   for (var i = 0; i < 112; i++){
      indexName = indexName + "a";  
   }
   try{
      commCreateIndex( dbcl, indexName, {content:"text"});	   
   }
   catch( e ){
	  if( e != -6){
         throw buildException("commCreateIndex()", e, "create index fali ,the index length : 112 ", "success", "fail");
	  }
   }
   commCheckIndex( dbcl, indexName, false );
   
   //固定集合名长度大于127时，全文索引创建失败
   var indexName = "";
   for (var i = 0; i < 120; i++){
      indexName = indexName + "a";	  
   }
   try{
      commCreateIndex( dbcl, indexName, {content:"text"});	   
   }
   catch( e ){
      if( e != -6){
	     throw buildException("commCreateIndex()", e, "create index fali ,the index length : 120 ", "success", "fail");
	  }
   }
   commCheckIndex( dbcl, indexName, false );
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main()
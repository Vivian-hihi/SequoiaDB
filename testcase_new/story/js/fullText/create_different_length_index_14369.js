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
   }

   var clName = COMMCLNAME + "_ES_14369";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   
   //索引名长度为1时，全文索引创建成功
   var indexName = "a"
   dbcl.createIndex(indexName, {content : "text"});
   commCheckIndex( dbcl, indexName, true );
   println("===create index success===");
   commDropIndex( dbcl, indexName, true );
   
   //固定集合名长度小于127时，全文索引创建成功
   var indexName = "";
   for (var i = 0; i < 20; i++){
      indexName = indexName + "a";	  
   }
   dbcl.createIndex(indexName, {content : "text"});
   commCheckIndex( dbcl, indexName, true );
   println("===create index success===");
   commDropIndex( dbcl, indexName, true );
   
   //固定集合名长度等于127时，全文索引创建成功
   var cursor = db.snapshot(8, {Name : COMMCSNAME + "." + clName});
   var cursor = cursor.next().toObj();
   var cappedCLLength = String(cursor["UniqueID"]).length + 5;
   
   var indexName = "";
   for (var i = 0; i < 127 - cappedCLLength; i++){
      indexName = indexName + "a";	  
   }
   dbcl.createIndex(indexName, {content : "text"});
      
   var dbOperater = new DBOperator();
   var cappedCLName = dbOperater.getCappedCLName( dbcl, indexName );
   
   commCheckIndex( dbcl, indexName, true );
   println("===create index success===");
   commDropIndex( dbcl, indexName, true );
   
   //SEQUOIADBMAINSTREAM-3896
   //固定集合名长度等于128时，全文索引创建失败
   /*
   var indexName = "";
   for (var i = 0; i < 127 -cappedCLLength + 1; i++){
      indexName = indexName + "a";  
   }
   try{
      dbcl.createIndex(indexName, {content : "text"}); 
      throw "CREATEINDEXERR" ;
   }
   catch( e ){
	  if( e != -6){
         throw buildException("mian()", "create more than 127B index", "create index " + indexName, "fail to create index", "create index success");
      }
   }
   commCheckIndex( dbcl, indexName, false );
   println("===create index fail===");
   */
   
   //固定集合名长度大于127时，全文索引创建失败
   var indexName = "";
   for (var i = 0; i < 127 -cappedCLLength + 100; i++){
      indexName = indexName + "a";	  
   }
   try{
      dbcl.createIndex(indexName, {content : "text"});	   
      throw "CREATEINDEXERR" ;
   }
   catch( e ){
      if( e != -6){
         throw buildException("main()", "create more than 127B index", "create index " + indexName, "fail to create index", "create index success");
      }
   }
   commCheckIndex( dbcl, indexName, false );
   println("===create index fail===");
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main()
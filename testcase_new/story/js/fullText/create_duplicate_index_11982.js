/***************************************************************************
@Description :seqDB-11982 :全文索引个数已达上限时创建全文索引 
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
   
   //在已存在全文索引定义的集合中，再次创建全文索引
   dbcl.createIndex( "a", {content:"text"});
   commCheckIndex( dbcl, "a", true );
   try{
      dbcl.createIndex( "b", {about:"text"});
      throw "CREATEINDEXERR";
   }
   catch( e ){
      if( e != -42){
         throw buildException("main()", "create duplicate indexes", "create index", "fail to create index", "create index success");
      }
   }
   
   var indexes = dbcl.listIndexes();
   var arrayIndexes = new Array();
   while (indexes.next()){
      arrayIndexes.push(indexes.current().toObj());
   }
   
   //listIndexes不显示创建失败的集合
   if(arrayIndexes.length != 2){
      throw buildException("main()", "more than 2 indexes", "indexes.length equal to 2", 2, arrayIndexes.length);
   }
   for(var i in arrayIndexes){
      if(arrayIndexes[i]["IndexDef"]["name"] != "$id" && arrayIndexes[i]["IndexDef"]["name"] != "a"){
         throw buildException("main()", "have not exist index", "equal", "$id or a", arrayIndexes[i]["IndexDef"]["name"]);
      }
   }
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main()
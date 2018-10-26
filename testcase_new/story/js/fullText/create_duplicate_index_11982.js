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
   };

   var clName = COMMCLNAME + "_ES_11985";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var dbcl = commCreateCL(db, COMMCSNAME, clName, 0);
   
   //在已存在全文索引定义的集合中，再次创建全文索引
   commCreateIndex( dbcl, "a", {content:"text"});
   commCheckIndex( dbcl, "a", true );
   try{
      commCreateIndex( dbcl, "b", {about:"text"});
   }
   catch( e ){
	  if( e != -42){
	     throw buildException("commCreateIndex()", e, "create full index twice ", "success", "fail");
	  }
   }
   
   var indexes = dbcl.listIndexes();
   var arrayIndexes = new Array();
   while (indexes.next()){
	  arrayIndexes.push(indexes.current().toObj());
   }
   
   //listIndexes不显示创建失败的集合
   if(arrayIndexes.length != 2){
	  throw buildException("listIndexes()", e, "indexes length is wrong ", "success", "fail");
   }
   for(var i in arrayIndexes){
      if(arrayIndexes[i]["IndexDef"]["name"] != "$id" && arrayIndexes[i]["IndexDef"]["name"] != "a"){
		 throw buildException("listIndexes()", "listIndexes", "indexes values is wrong ", "success", "fail");
	  }
   }
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main()
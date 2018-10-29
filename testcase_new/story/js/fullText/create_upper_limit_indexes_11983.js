/***************************************************************************
@Description :seqDB-11983 :索引个数已达上限时创建全文索引   
@Modify list :
              2018-10-26  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };

   var clName = COMMCLNAME + "_ES_11983";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   //创建64个索引
   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   for (var i=0; i < 63; i++){
	   var obj = new Object();
	   obj["a"+i] = 1;
      dbcl.createIndex( "a"+i, obj);
   }
   
   //在已创建64个索引的情况下，创建全文索引
   try{
      dbcl.createIndex( "fullIndex", {content : "text"});
	  throw e ;
   }
   catch( e ){
	  if( e != -42){
	     throw buildException("dbcl.createIndex()", "create Index", "create full index in upper limit indexes collection ", "success", "fail");
	  }
   }
   commCheckIndex( dbcl, "fullIndex", false );
   
   var indexes = dbcl.listIndexes();
   var arrayIndexes = new Array();
   while(indexes.next()){
      var index = indexes.current().toObj();
      arrayIndexes.push(index["IndexDef"]["name"]);
   }
   
   //检查listIndexes是否包含创建失败的索引名
   for(var i in arrayIndexes){
      if (arrayIndexes[i] == "fullIndex"){
	     throw buildException("listIndexes()", "list Indexes", "create full index in upper limit indexes collection ", "success", "fail");	  
	  }
   }
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main()
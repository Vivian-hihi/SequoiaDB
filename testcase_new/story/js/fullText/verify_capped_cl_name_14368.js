/***************************************************************************
@Description :seqDB-14368 :创建全文索引，固定集合名验证 
@Modify list :
              2018-10-25  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };

   var clName = COMMCLNAME + "_ES_14368";
   var csName = "testCS_ES_14368";
   commDropCL( db, COMMCSNAME, clName, true, true);
   commDropCS( db, csName, true );
   
   var dbcl = commCreateCL(db, COMMCSNAME, clName );
   var dbcl_1 = commCreateCL( db, csName, clName );
   var dbcl_2 = commCreateCLByOption( db, csName, clName + "_2", {ShardingKey:{content:1}, ShardingType:"hash"} )
   var dbcl_3 = commCreateCLByOption( db, csName, clName + "_3", {ShardingKey:{content:1}, ShardingType:"range"} )
   var dbcl_4_1 = commCreateCL( db, csName, clName + "_4_1" );
   var dbcl_4_2 = commCreateCL( db, csName, clName + "_4_2" );
   var dbcl_4 = commCreateCLByOption( db, csName, clName + "_4", {ShardingKey:{content:1}, ShardingType:"range", IsMainCL:true} )
   dbcl_4.attachCL(csName + "." + clName + "_4_1", {LowBound : {content : "a"}, UpBound : {content : "f"}});
   dbcl_4.attachCL(csName + "." + clName + "_4_2", {LowBound : {content : "x"}, UpBound : {content : "z"}});
   
   //在不同的集合空间，不同的集合创建全文索引
   var indexName = "a"
   commCreateIndex( dbcl, indexName, {content:"text"});
   commCreateIndex( dbcl_1, indexName, {content:"text"});
   commCreateIndex( dbcl_2, indexName, {content:"text"});
   commCreateIndex( dbcl_3, indexName, {content:"text"});
   commCreateIndex( dbcl_4, indexName, {content:"text"});
   
   //获取固定集合名
   var dbOperator = new DBOperator();
   var cappedArray = new Array();
   var cappedCLName = dbOperator.getCappedCLName( dbcl, indexName );
   cappedArray.push(cappedCLName);
   var cappedCLName_1 = dbOperator.getCappedCLName( dbcl_1, indexName );
   cappedArray.push(cappedCLName_1);
   var cappedCLName_2 = dbOperator.getCappedCLName( dbcl_2, indexName );
   cappedArray.push(cappedCLName_2);
   var cappedCLName_3 = dbOperator.getCappedCLName( dbcl_3, indexName );
   cappedArray.push(cappedCLName_3);
   var cappedCLName_4 = dbOperator.getCappedCLName( dbcl_4, indexName );
   cappedArray.push(cappedCLName_4);
   
   //检查固定集合名均不一致
   for (var i in cappedArray){
      var cpName = cappedArray[i];
      if (cappedArray.indexOf(cpName) != cappedArray.lastIndexOf(cpName)){
	     throw buildException("getCappedCLName()", e, "get the same name capped cl", "success", "fail");
	  }	  
   }
   
   //检查索引属性的ExtDataName和固定集合名一致
   checkExtDataName(dbcl, indexName, cappedCLName);
   checkExtDataName(dbcl_1, indexName, cappedCLName_1);
   checkExtDataName(dbcl_2, indexName, cappedCLName_2);
   checkExtDataName(dbcl_3, indexName, cappedCLName_3);
   checkExtDataName(dbcl_4, indexName, cappedCLName_4);

   //检查ES端的全文索引名字映射关系为固定集合名_组名
   var esOperator = new ESOperator();
   var esIndexName = dbOperator.getESIndexName (COMMCSNAME, clName, indexName);
   esOperator.isExistIndexInES(esIndexName);
   var esIndexName = dbOperator.getESIndexName (csName, clName, indexName);
   esOperator.isExistIndexInES(esIndexName);
   var esIndexName = dbOperator.getESIndexName (csName, clName + "_2", indexName);
   esOperator.isExistIndexInES(esIndexName);
   var esIndexName = dbOperator.getESIndexName (csName, clName + "_3", indexName);
   esOperator.isExistIndexInES(esIndexName);
   var esIndexName = dbOperator.getESIndexName (csName, clName + "_4_1", indexName);
   esOperator.isExistIndexInES(esIndexName);
   var esIndexName = dbOperator.getESIndexName (csName, clName + "_4_2", indexName);
   esOperator.isExistIndexInES(esIndexName);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   commDropCS( db, csName, true );
}

function checkExtDataName(dbcl, indexName, cappedCLName){
   var index = dbcl.getIndex(indexName).toObj();
   var extDataName = index["ExtDataName"];
   if (cappedCLName != extDataName){
      throw buildException("getCappedCLName()", e, "cappedCLName is not equal index extDataName ", "success", "fail");
   }
}

main()
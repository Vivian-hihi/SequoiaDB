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
   }

   var clName = COMMCLNAME + "_ES_14368";
   var csName = "testCS_ES_14368";
   commDropCL( db, COMMCSNAME, clName, true, true);
   commDropCS( db, csName, true );
   
   var dbcl = commCreateCL(db, COMMCSNAME, clName );
   var dbcl1 = commCreateCL( db, csName, clName );
   var dbcl2 = commCreateCLByOption( db, csName, clName + "_2", {ShardingKey:{content:1}, ShardingType:"hash"} )
   var dbcl3 = commCreateCLByOption( db, csName, clName + "_3", {ShardingKey:{content:1}, ShardingType:"range"} )
   var slaveCL1 = commCreateCL( db, csName, clName + "_4_1" );
   var slaveCL2 = commCreateCL( db, csName, clName + "_4_2" );
   var mainCL = commCreateCLByOption( db, csName, clName + "_4", {ShardingKey:{content:1}, ShardingType:"range", IsMainCL:true} )
   mainCL.attachCL(csName + "." + clName + "_4_1", {LowBound : {content : "a"}, UpBound : {content : "f"}});
   mainCL.attachCL(csName + "." + clName + "_4_2", {LowBound : {content : "x"}, UpBound : {content : "z"}});
   
   //在不同的集合空间，不同的集合创建全文索引
   var indexName = "a"
   commCreateIndex( dbcl, indexName, {content:"text"});
   commCreateIndex( dbcl1, indexName, {content:"text"});
   commCreateIndex( dbcl2, indexName, {content:"text"});
   commCreateIndex( dbcl3, indexName, {content:"text"});
   commCreateIndex( mainCL, indexName, {content:"text"});
   
   //获取固定集合名
   var dbOperator = new DBOperator();
   var cappedArray = new Array();
   var cappedCLName = dbOperator.getCappedCLName( dbcl, indexName );
   cappedArray.push(cappedCLName);
   var cappedCLName1 = dbOperator.getCappedCLName( dbcl1, indexName );
   cappedArray.push(cappedCLName1);
   var cappedCLName2 = dbOperator.getCappedCLName( dbcl2, indexName );
   cappedArray.push(cappedCLName2);
   var cappedCLName3 = dbOperator.getCappedCLName( dbcl3, indexName );
   cappedArray.push(cappedCLName3);
   var cappedSlaveCLName1 = dbOperator.getCappedCLName( slaveCL1, indexName );
   cappedArray.push(cappedSlaveCLName1);
   var cappedSlaveCLName2 = dbOperator.getCappedCLName( slaveCL2, indexName );
   cappedArray.push(cappedSlaveCLName2);
   
   //检查固定集合名均不一致
   for (var i in cappedArray){
      var cpName = cappedArray[i];
      if (cappedArray.indexOf(cpName) != cappedArray.lastIndexOf(cpName)){
         throw buildException("main()", "exists duplicate capped cl name", "equal", JSON.stringify(cappedArray), cpName);
      }	  
   }
   
   //检查索引属性的ExtDataName和固定集合名一致
   checkExtDataName(dbcl, indexName, cappedCLName);
   checkExtDataName(dbcl1, indexName, cappedCLName1);
   checkExtDataName(dbcl2, indexName, cappedCLName2);
   checkExtDataName(dbcl3, indexName, cappedCLName3);
   checkExtDataName(slaveCL1, indexName, cappedSlaveCLName1);
   checkExtDataName(slaveCL2, indexName, cappedSlaveCLName2);

   //检查ES端的全文索引名字映射关系为固定集合名_组名
   var esOperator = new ESOperator();
   var groups = commGetCLGroups( db, COMMCSNAME + "." + clName );
   var esIndexName = cappedCLName.toLowerCase() + "_" + groups[0];
   esOperator.isExistIndexInES(esIndexName);
   var groups = commGetCLGroups( db, csName + "." + clName );
   var esIndexName = cappedCLName1.toLowerCase() + "_" + groups[0];
   esOperator.isExistIndexInES(esIndexName);
   var groups = commGetCLGroups( db, csName + "." + clName + "_2" );
   var esIndexName = cappedCLName2.toLowerCase() + "_" + groups[0];
   esOperator.isExistIndexInES(esIndexName);
   var groups = commGetCLGroups( db, csName + "." + clName + "_3" );
   var esIndexName = cappedCLName3.toLowerCase() + "_" + groups[0];
   esOperator.isExistIndexInES(esIndexName);
   var groups = commGetCLGroups( db, csName + "." + clName + "_4_1" );
   var esIndexName = cappedSlaveCLName1.toLowerCase() + "_" + groups[0];
   esOperator.isExistIndexInES(esIndexName);
   var groups = commGetCLGroups( db, csName + "." + clName + "_4_2" );
   var esIndexName = cappedSlaveCLName2.toLowerCase() + "_" + groups[0];
   esOperator.isExistIndexInES(esIndexName);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
   commDropCS( db, csName, true );
}

function checkExtDataName(dbcl, indexName, cappedCLName){
   var index = dbcl.getIndex(indexName).toObj();
   var extDataName = index["ExtDataName"];
   if (cappedCLName != extDataName){
      throw buildException("checkExtDataName()", "index's property ExtDataName is not equal to cappedCLName", "equal", "cappedCLName : " + cappedCLName, "extDataName : " + extDataName);
   }
}

main()
/************************************
*@Description: 插入全文索引字段为1个/多个数组元素的记录，ES从固定集合/原始集合中同步   
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-15537
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   // create CL
   var clName = COMMCLNAME + "_ES_15537";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var dbcl = commCreateCL( db, COMMCSNAME, clName );

   // insert before create text index
   var objs = new Array({id:1, a: ["arr1"]},
                        {id:2, a: [1]},
                        {id:3, a: ["arr2", "arr3"]},
                        {id:4, a: ["arr4", 2]},
                        {id:5, a: ["arr5", "arr6", 3]});
   dbcl.insert(objs);

   var textIndexName = "textIndex_15537";
   dbcl.createIndex(textIndexName, {"a" : "text"});
   
   // check result
   var dbOperator = new DBOperator();
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 4);
   var expectRecords = [{id:1, a: ["arr1"]},
                        {id:3, a: ["arr2", "arr3"]},
                        {id:4, a: ["arr4", 2]},
                        {id:5, a: ["arr5", "arr6", 3]}];
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}}, {id:"",a:""}, {id:1});
   checkResult(expectRecords, actRecords);

   dbcl.insert(objs);
   
   // check result
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 8);
   var expectRecords = [{id:1, a: ["arr1"]}, {id:1, a: ["arr1"]},
                        {id:3, a: ["arr2", "arr3"]}, {id:3, a: ["arr2", "arr3"]},
                        {id:4, a: ["arr4", 2]}, {id:4, a: ["arr4", 2]},
                        {id:5, a: ["arr5", "arr6", 3]}, {id:5, a: ["arr5", "arr6", 3]}];
   var actRecords = dbOperator.findFromCL(dbcl, {"":{"$Text":{query:{match_all:{}}}}}, {id:"",a:""}, {id:1});
   checkResult(expectRecords, actRecords);

   var esIndexNames = dbOpr.getESIndexNames(COMMCSNAME, clName, textIndexName);
   commDropCL(db, COMMCSNAME, clName, true, true);
   //SEQUOIADBMAINSTREAM-3983
   checkIndexNotExistInES(esIndexNames);
}
main();

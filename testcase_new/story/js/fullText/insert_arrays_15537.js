/************************************
*@Description: 插入全文索引字段为1个/多个数组元素的记录，ES从固定集合/原始集合中同步   
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-15537
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var csName = COMMCSNAME + "_ES_15537";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   // create CL
   var clName = COMMCLNAME + "_ES_15537";
   var dbcl = commCreateCL( db, csName, clName );

   // insert before create text index
   var objs = new Array({a: ["arr1"]},
                        {a: [1]},
                        {a: ["arr1", "arr2"]},
                        {a: ["arr1", 1]});
   dbcl.insert(objs);

   var textIndexName = "textIndex";
   dbcl.createIndex(textIndexName, {"a" : "text"});

   checkFullSyncToES(csName, clName, textIndexName, 1);
   
   // check result
   var dbOpr = new DBOperator();
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var expectResult = [{a: ["arr1"]}];
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1}});
   checkResult(expectResult, actResult);

   // insert after create text index
   var objs = new Array({a: ["arr2"]},
                        {a: ["arr11", "arr12"]},
                        {a: ["arr11", 1]});
   dbcl.insert(objs);
   
   // check sync to es
   checkFullSyncToES(csName, clName, textIndexName, 2); 

   // check result
   var expectResult = [{a: ["arr1"]}, {a: ["arr2"]}];
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1}});
   actResult.sort(compare("a"));
   checkResult(expectResult, actResult);

   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

/************************************
*@Description: 更新全文索引字段为多个数组元素的记录 
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-15775
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   // create CL
   var clName = COMMCLNAME + "_ES_15775";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var dbcl = commCreateCL( db, COMMCSNAME, clName );

   // insert before create text index
   var objs = new Array({a: "string1"},
                        {a: ["arr1"]},
                        {a: -1},
                        {a: true},
                        {a: {b : "ab"}},
                        {a: {$date: "2018-10-10"}},
                        {a: ["arr2", "arr3"]},
                        {a: ["arr4", 1]});
   dbcl.insert(objs);

   var textIndexName = "textIndex_15775";
   dbcl.createIndex(textIndexName, {"a" : "text"});

   // check sync to es
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 4);
   
   // check result
   var dbOpr = new DBOperator();
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var expResult = [{a: ["arr1"]},{a: ["arr2", "arr3"]},{a: ["arr4", 1]},{a: "string1"}];
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a": {"$include": 1}});
   actResult.sort(compare("a")); 
   checkResult(expResult, actResult);
/*
   // update to ["",""]
   dbcl.update({"$set" : {a : ["updated 1", "udpated 2"]}}, {a : {"$exists" : 1}});
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 8);
   
   // check result
   var expResult = dbOpr.findFromCL(dbcl);
   var actResult = dbOpr.findFromCL(dbcl, findCond);
   checkResult(expResult, actResult);
*/
   var esIndexNames = dbOpr.getESIndexNames(COMMCSNAME, clName, textIndexName);
   commDropCL(db, COMMCSNAME, clName, true, true);
   //SEQUOIADBMAINSTREAM-3983
   checkIndexNotExistInES(esIndexNames);
}
main();

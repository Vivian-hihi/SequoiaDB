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
                        {a: ["arr1", "arr2"]},
                        {a: ["arr1", 1]});
   dbcl.insert(objs);

   var textIndexName = "textIndex";
   dbcl.createIndex(textIndexName, {"a" : "text"});

   // check sync to es
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 2);
   
   // check result
   var dbOpr = new DBOperator();
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var expResult = [{a: ["arr1"]}, {a: "string1"}];
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a": {"$include": 1}});
   actResult.sort(compare("a"));
   checkResult(expResult, actResult);

   // update to ["",""]
   dbcl.update({"$set" : {a : ["updated 1", "udpated 2"]}}, {a : {"$exists" : 1}});
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 0);
   
   // check result
   var expResult = [];
   var actResult = dbOpr.findFromCL(dbcl, findCond);
   checkResult(expResult, actResult);

   commDropCL(db, COMMCSNAME, clName, true, true);
}
main();

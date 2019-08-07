/************************************
*@Description: 将全文索引字段为数组类型更新为string类型 
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-15776
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   // create CL
   var clName = COMMCLNAME + "_ES_15776";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var dbcl = commCreateCL( db, COMMCSNAME, clName );

   // insert before create text index
   var objs = new Array({a: "string1"},
                        {a: ["arr1"]},
                        {a: 0},
                        {a: true},
                        {a: {b : "ab"}},
                        {a: {$date: "2018-10-10"}},
                        {a: ["arr2", "arr3"]},
                        {a: ["arr4", 1]});
   dbcl.insert(objs);

   var textIndexName = "textIndex_15776";
   dbcl.createIndex(textIndexName, {"a" : "text"});

   // check sync to es
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 3);
   
   // check result
   var dbOpr = new DBOperator();
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var expResult = [{a: ["arr1"]},{a: ["arr2", "arr3"]},{a: "string1"}];
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a": {"$include": 1}});
   actResult.sort(compare("a"));
   checkResult(expResult, actResult);

   // update to string
   dbcl.update({"$set" : {a : "updated string"}}, {a : {"$exists" : 1}});
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 8);
   
   // check result
   var expResult = dbOpr.findFromCL(dbcl, null);
   var actResult = dbOpr.findFromCL(dbcl, findCond);
   checkResult(expResult, actResult); 

   var esIndexNames = dbOpr.getESIndexNames(COMMCSNAME, clName, textIndexName);
   commDropCL(db, COMMCSNAME, clName, true, true); 
   //SEQUOIADBMAINSTREAM-3983
   checkIndexNotExistInES(esIndexNames);
}
main();

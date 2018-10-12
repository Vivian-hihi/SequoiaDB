/************************************
*@Description: 将全文索引字段为数组类型更新为string类型 
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-15776
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var csName = COMMCSNAME + "15776";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   // create CL
   var clName = COMMCLNAME + "15776";
   var dbcl = commCreateCL( db, csName, clName );

   // insert before create text index
   var objs = new Array({a: "string1"},
                        {a: ["arr1"]},
                        {a: 0},
                        {a: true},
                        {a: {b : "ab"}},
                        {a: {$date: "2018-10-10"}},
                        {a: ["arr1", "arr2"]},
                        {a: ["arr1", 1]});
   dbcl.insert(objs);

   var textIndexName = "textIndex";
   dbcl.createIndex(textIndexName, {"a" : "text"});

   // check sync to es
   checkFullSyncToES(csName, clName, textIndexName, 2);
   
   // check result
   var dbOpr = new DBOperator();
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var expResult = [{a: ["arr1"]}, {a: "string1"}];
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a": {"$include": 1}});
   actResult.sort(compare("a"));
   checkResult(expResult, actResult);

   // update to string
   dbcl.update({"$set" : {a : "updated string"}}, {a : {"$exists" : 1}});
   checkFullSyncToES(csName, clName, textIndexName, 8);
   
   // check result
   var expResult = dbOpr.findFromCL(dbcl, null);
   var actResult = dbOpr.findFromCL(dbcl, findCond);
   checkResult(expResult, actResult); 
 
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

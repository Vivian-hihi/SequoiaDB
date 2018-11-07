/************************************
*@Description: 更新全文索引字段为1个数组元素的记录 
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-15774
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   // create CL
   var clName = COMMCLNAME + "_ES_15774";
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

   var dbOpr = new DBOperator();
   
   // check sync to es
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 2);
   
   // check result
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var expResult = [{a: ["arr1"]}, {a: "string1"}];
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1}});
   actResult.sort(compare("a")); 
   checkResult(expResult, actResult);

   // update to [""]
   dbcl.update({"$set" : {a : ["updated"]}}, {a : {"$isnull" : 0}});
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 8); 
   
   // check result
   var expResult = new Array();
   for(var i = 0; i < dbcl.count(); i++)  
   { 
      expResult.push({a : ["updated"]});  
   }
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a": {"$include": 1}});
   checkResult(expResult, actResult);

   commDropCL(db, COMMCSNAME, clName, true, true);
}
main();

/************************************
*@Description: 更新全文索引字段为1个数组元素的记录 
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-15774
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var csName = COMMCSNAME + "15774";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   // create CL
   var clName = COMMCLNAME + "15774";
   var dbcl = commCreateCL( db, csName, clName );

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
   var esIndexName = dbOpr.getESIndexName(csName, clName, textIndexName);
   
   // check sync to es
   checkFullSyncToES(csName, clName, textIndexName, 2);
   
   // check result
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var expResult = [{a: ["arr1"]}, {a: "string1"}];
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1}});
   actResult.sort(compare("a")); 
   checkResult(expResult, actResult);

   // update to [""]
   dbcl.update({"$set" : {a : ["updated"]}}, {a : {"$isnull" : 0}});
   checkFullSyncToES(csName, clName, textIndexName, 8); 
   
   // check result
   var expResult = new Array();
   for(var i = 0; i < dbcl.count(); i++)  
   { 
      expResult.push({a : ["updated"]});  
   }
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a": {"$include": 1}});
   checkResult(expResult, actResult);

   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

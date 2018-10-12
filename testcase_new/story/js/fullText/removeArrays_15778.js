/************************************
*@Description: 删除全文索引字段为1个数组元素/多个数组元素的记录 
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-15778
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var csName = COMMCSNAME + "_ES_15778";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   // create CL
   var clName = COMMCLNAME + "_ES_15778";
   var dbcl = commCreateCL( db, csName, clName );

   // insert before create text index
   var objs = new Array({a: ["arr1"]}, {a: ["arr1", "arr2"]}, {a: ["arr1", 1]});
   dbcl.insert(objs);

   var textIndexName = "textIndex";
   dbcl.createIndex(textIndexName, {"a" : "text"});

   // check sync to es
   checkFullSyncToES(csName, clName, textIndexName, 1);
  
   // check result
   var dbOpr = new DBOperator();
   var expectResult = [{a: ["arr1"]}];
   var actResult = dbOpr.findFromCL(dbcl, {"":{"$Text":{"query":{"match_all":{}}}}}, {"a" : {"$include" : 1}});
   checkResult(expectResult, actResult);

   // delete
   dbcl.remove();
   checkFullSyncToES(csName, clName, textIndexName, 0);
   
   // check result
   var expectResult = dbOpr.findFromCL(dbcl, null);
   var actResult = dbOpr.findFromCL(dbcl, {"":{"$Text":{"query":{"match_all":{}}}}}, {"a" : {"$include" : 1}});
   checkResult(expectResult, actResult);

   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

/************************************
*@Description: 多键全文索引，部分字段为数组更新为非string类型    
*@author:      liuxiaoxuan
*@createdate:  2018.11.6
*@testlinkCase: seqDB-15782
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   // create CL
   var clName = COMMCLNAME + "_ES_15782";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var dbcl = commCreateCL( db, COMMCSNAME, clName );

   // insert before create text index
   dbcl.insert({a: ["arr1"], b: "string1", c: ["arr1", "arr2"], d: 1});

   var textIndexName = "textIndex_15782";
   dbcl.createIndex(textIndexName, {"a" : "text", "b" : "text", "c" : "text", "d" : "text"});
   
   // check sync to es
   var dbOpr = new DBOperator();
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 1);
   
   // check result
   var esIndexNames = dbOpr.getESIndexNames(COMMCSNAME, clName, textIndexName);
   var esOpr = new ESOperator();
   var searchCond = '{"query":{"match_all":{}}}';
   var expectResult = [{a: ["arr1"], b: "string1", c: ["arr1", "arr2"]}];
   var actResult = esOpr.findFromES(esIndexNames[0], searchCond);
   checkResult(expectResult, actResult);
   
   // update all to non-string after create text index
   dbcl.update({$set:{a: -1, b: -1.111111, c: {$date: "2018-01-01"}, d: true}});
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 0);

   // check result
   var actResult = esOpr.findFromES(esIndexNames[0], searchCond);
   checkResult([], actResult);

   commDropCL(db, COMMCSNAME, clName, true, true);
   //SEQUOIADBMAINSTREAM-3983
   checkIndexNotExistInES(esIndexNames);
}
main();

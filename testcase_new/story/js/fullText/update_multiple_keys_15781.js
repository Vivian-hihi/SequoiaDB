/************************************
*@Description:  多键全文索引，部分字段为数组更新为string类型    
*@author:      liuxiaoxuan
*@createdate:  2018.11.6
*@testlinkCase: seqDB-15781
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   commDropCL(db, COMMCSNAME, clName, true, true);                                                           	
   // create CL
   var clName = COMMCLNAME + "_ES_15781";
   var dbcl = commCreateCL( db, COMMCSNAME, clName );

   // insert before create text index
   dbcl.insert({a: ["arr1"], b: "string1", c: ["arr1", "arr2"], d: 1});

   var textIndexName = "textIndex";
   dbcl.createIndex(textIndexName, {"a" : "text", "b" : "text", "c" : "text", "d" : "text"});
   
   // check sync to es
   var dbOpr = new DBOperator();
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 1);
   
   // check result
   var esIndexNames = dbOpr.getESIndexNames(COMMCSNAME, clName, textIndexName);
   var esOpr = new ESOperator();
   var searchCond = '{"query":{"match_all":{}}}';
   var expectResult = [{a: "arr1", b: "string1"}];
   var actResult = esOpr.findFromES(esIndexNames[0], searchCond);
   checkResult(expectResult, actResult);
   
   // update all to string  after create text index
   dbcl.update({$set:{a:"string_a", b:"string_b", c:"string_c", d:"string_d"}});
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 1);

   // check result
   var expectResult = [{a:"string_a", b:"string_b", c:"string_c", d:"string_d"}];
   var actResult = esOpr.findFromES(esIndexNames[0], searchCond);
   checkResult(expectResult, actResult);
  
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main();

/************************************
*@Description: 多键全文索引，部分字段为数组更新为部分字段为数组   
*@author:      liuxiaoxuan
*@createdate:  2018.11.6
*@testlinkCase: seqDB-15783
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var csName = COMMCSNAME + "_ES_15783";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   // create CL
   var clName = COMMCLNAME + "_ES_15783";
   var dbcl = commCreateCL( db, csName, clName );

   // insert before create text index
   dbcl.insert({a: ["arr1"], b: "string1", c: ["arr1", "arr2"], d: 1});

   var textIndexName = "textIndex";
   dbcl.createIndex(textIndexName, {"a" : "text", "b" : "text", "c" : "text", "d" : "text"});
   
   // check sync to es
   var dbOpr = new DBOperator();
   checkFullSyncToES(csName, clName, textIndexName, 1);
   
   // check result
   var esIndexNames = dbOpr.getESIndexNames(csName, clName, textIndexName);
   var esOpr = new ESOperator();
   var searchCond = '{"query":{"match_all":{}}}';
   var expectResult = [{a: "arr1", b: "string1"}];
   var actResult = esOpr.findFromES(esIndexNames[0], searchCond);
   checkResult(expectResult, actResult);
   
   // update keys to arrays after create text index, should fail
   try
   {
      dbcl.update({$set:{a: 1, b: ["string1", "string2"], c: "string_c", d: ["string_d"]}});
      throw buildException("update()", "update", "update keys to arrays", "fail","success");
   }
   catch(e)
   {
      if(-37 != e)
      {
         throw buildException("update()", "update", "update other exception", e, e);
      }
   }
 
   checkFullSyncToES(csName, clName, textIndexName, 1);
 
   // check result
   var actResult = esOpr.findFromES(esIndexNames[0], searchCond);
   checkResult(expectResult, actResult);

   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

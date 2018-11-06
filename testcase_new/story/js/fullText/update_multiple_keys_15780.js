/************************************
*@Description: 多键全文索引，部分字段为数组更新为全部字段为数组   
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-15780
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var csName = COMMCSNAME + "_ES_15780";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   // create CL
   var clName = COMMCLNAME + "_ES_15780";
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
   
   // update all to arrays after create text index, should fail
   try
   {
      dbcl.update({$set:{a:['updatea1', 'updatea2'],b:['updateb1', 'updateb2'],c:['updatec1', 'updatec2'],d:['updated1', 'updated2']}});
      throw buildException("update()", "update", "update to all arrays of keys", "fail","success");
   }
   catch(e)
   {
      if(-37 != e)
      {
         throw buildException("update()", "update", "update other exception", e, e);
      }
   }

   // check result
   var actResult = esOpr.findFromES(esIndexNames[0], searchCond);
   checkResult(expectResult, actResult);

   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

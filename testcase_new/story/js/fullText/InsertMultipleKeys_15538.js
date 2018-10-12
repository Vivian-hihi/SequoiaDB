/************************************
*@Description: 多键全文索引，部分字段为数组类型，ES从固定集合/原始集合同步记录   
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-15538
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var csName = COMMCSNAME + "15538";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   // create CL
   var clName = COMMCLNAME + "15538";
   var dbcl = commCreateCL( db, csName, clName );

   // insert before create text index
   dbcl.insert({a: ["arr1"], b: ["arr1", "arr2"], c: "string1", d: -1});

   var textIndexName = "textIndex";
   dbcl.createIndex(textIndexName, {"a" : "text", "b" : "text", "c" : "text", "d" : "text"});

   var dbOpr = new DBOperator();
   
   // check sync to es
   checkFullSyncToES(csName, clName, textIndexName, 1);
   
   // check result
   var dbOpr = new DBOperator();
   var esIndexName = dbOpr.getESIndexName(csName, clName, textIndexName);
   var searchCond = '{"query":{"match_all":{}}}';
   var expectResult = [{a: "arr1", c: "string1"}];
   var actResult = esOpr.findFromES(esIndexName, searchCond);
   checkResult(expectResult, actResult);
   
   // insert more than one array after create text index, should fail
   try
   {
      dbcl.insert({a: "string2", b: 0, c: ["arr2"], d: ["arr11", "arr12"]});
      throw buildException("insert()", "insert", "insert many arrays of keys", "fail","success");
   }
   catch(e)
   {
      if(-37 != e)
      {
         throw buildException("insert()", "insert", "insert other exception", e, e);
      }
   }

   // check result after insert
   var actResult = esOpr.findFromES(esIndexName, searchCond);
   checkResult(expectResult, actResult);

   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

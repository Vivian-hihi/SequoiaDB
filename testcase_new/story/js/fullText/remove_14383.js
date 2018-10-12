/************************************
*@Description: 带全文索引条件删除记录 
*@author:      liuxiaoxuan
*@createdate:  2018.10.09
*@testlinkCase: seqDB-14383
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var csName = COMMCSNAME + "14383";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   //create CL
   var clName = COMMCLNAME + "14383";
   var dbcl = commCreateCL( db, csName, clName );

   var textIndexName = "textIndex";
   dbcl.createIndex(textIndexName, {"a" : "text"});

   dbcl.insert({"a" : "testa"}); 

   checkFullSyncToES(csName, clName, textIndexName, 1);

   var dbOpr = new DBOperator();
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var expResult = [{"a" : "testa"}];
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1}});
   checkResult(expResult, actResult);
   
   // remove with DSL 
   try
   {
      dbcl.remove({"": {"$Text": {"query": {"match_all":{}}}}}); // should fail
      throw buildException("remove()", "remove with DSL", "remove", "fail","success");
   }
   catch(e)
   {
      if(-6 !== e)  
      {
          throw buildException("remove()", "remove fail exception", "remove", e, e); 
      }
   }
   
   // check result
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1}});
   checkResult(expResult, actResult);

   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

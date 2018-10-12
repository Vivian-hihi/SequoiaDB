/************************************
*@Description: 查询接口参数验证 
*@author:      liuxiaoxuan
*@createdate:  2018.10.09
*@testlinkCase: seqDB-12046
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var csName = COMMCSNAME + "_ES_12046";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   //create CL
   var clName = COMMCLNAME + "_ES_12046";
   var dbcl = commCreateCL( db, csName, clName );

   var textIndexName = "textIndex";
   dbcl.createIndex(textIndexName, {"a" : "text"});

   dbcl.insert({"a" : "testa"}); 

   checkFullSyncToES(csName, clName, textIndexName, 1);

   // check result
   var dbOpr = new DBOperator();
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var expResult = [{"a" : "testa"}];
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1}});
   checkResult(expResult, actResult);
  
   // find with wrong search command
   try
   {
      var rec = dbcl.find({"": {"$text": {"query": {"match_all":{}}}}}); // should fail
      rec.next();
      throw buildException("find()", "find", "find with wrong command", "fail","success");
   }
   catch(e)
   {
      if(-6 !== e)  
      {
          throw buildException("find()", "find", "find other exception", e, e); 
      }

   }
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

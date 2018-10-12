/************************************
*@Description: 带hint删除记录 
*@author:      liuxiaoxuan
*@createdate:  2018.10.09
*@testlinkCase: seqDB-14384
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var csName = COMMCSNAME + "_ES_14384";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   //create CL
   var clName = COMMCLNAME + "_ES_14384";
   var dbcl = commCreateCL( db, csName, clName );

   var textIndexName = "textIndex";
   var commonIndexName = "commonIndex"
   dbcl.createIndex(textIndexName, {"a" : "text"});
   dbcl.createIndex(commonIndexName, {"b" : 1});
  
   // insert
   var objs = new Array();
   for(var i = 0; i < 10000; i++)
   {
      objs.push({a: "test_14384_" + i, b : "testb_" + i });
   }
   insertRecords(dbcl, objs);

   // if insert fail, exit
   if(10000 != dbcl.count())
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }

   checkFullSyncToES(csName, clName, textIndexName, 10000);
   
   // remove with hint textIndex
   dbcl.remove({"a" : "test_14384_0"}, {"" : textIndexName});
   checkFullSyncToES(csName, clName, textIndexName, 9999);

   // check result
   var dbOpr = new DBOperator();
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var actResult = dbOpr.findFromCL(dbcl, findCond);
   var expResult = dbOpr.findFromCL(dbcl, null);
   actResult.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult);

   // remove with hint commonIndex
   dbcl.remove({"a" : {"$gt" : "test_14384_1000"}}, {"" : commonIndexName});
   checkFullSyncToES(csName, clName, textIndexName, 4);

   // check result
   var actResult = dbOpr.findFromCL(dbcl, findCond);
   var expResult = dbOpr.findFromCL(dbcl, null);
   actResult.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult);
   
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

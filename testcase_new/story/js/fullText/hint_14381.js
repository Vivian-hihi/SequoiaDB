/************************************
*@Description: 使用hint更新记录  
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-14381
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }
  
   var csName = COMMCSNAME + "_ES_14381";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   //create CL
   var clName = COMMCLNAME + "_ES_14381";
   var dbcl = commCreateCL( db, csName, clName );

   var textIndexName = "textIndex";
   var commonIndexName = "commonIndex"
   dbcl.createIndex(textIndexName, {"a" : "text"});
   dbcl.createIndex(commonIndexName, {"b" : 1});

   // insert
   var objs = new Array();
   for(var i = 0; i < 10000; i++)
   {
      objs.push({a: "test_14381_" + i, b : "testb_" + i });
   }
   insertRecords(dbcl, objs);

   // if insert fail, exit
   if(10000 != dbcl.count())
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }

   checkFullSyncToES(csName, clName, textIndexName, 10000);
   
   // update with hint textIndex
   dbcl.update({"$set" : {"a" : "update text index 0"}}, {"a" : "test_14381_0"}, {"" : textIndexName});
   checkFullSyncToES(csName, clName, textIndexName, 10000);

   // check result
   var dbOpr = new DBOperator();
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var actResult = dbOpr.findFromCL(dbcl, findCond);
   var expResult = dbOpr.findFromCL(dbcl, null);
   actResult.sort(compare("a", compare("b")));
   expResult.sort(compare("a", compare("b")));
   checkResult(expResult, actResult);

   // update with hint commonIndex
   dbcl.update({"$set" : {"a" : "update text index many"}}, {"a" : {"$gt" : "test_14381_1000"}}, {"" : commonIndexName});
   checkFullSyncToES(csName, clName, textIndexName, 10000);

   // check result
   var actResult = dbOpr.findFromCL(dbcl, findCond);
   var expResult = dbOpr.findFromCL(dbcl, null);
   actResult.sort(compare("a", compare("b")));
   expResult.sort(compare("a", compare("b")));
   checkResult(expResult, actResult);
   
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

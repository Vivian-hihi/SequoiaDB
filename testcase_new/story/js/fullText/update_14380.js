/************************************
*@Description: 更新索引字段   
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-14380
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var csName = COMMCSNAME + "_ES_14380";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   //create CL
   var clName = COMMCLNAME + "_ES_14380";
   var dbcl = commCreateCL( db, csName, clName );

   var textIndexName = "textIndex";
   var commonIndexName = "commonIndex"
   dbcl.createIndex(textIndexName, {"a" : "text"});
   dbcl.createIndex(commonIndexName, {"b" : 1});

   // insert
   var objs = new Array();
   for(var i = 0; i < 10000; i++)
   {
      objs.push({a: "test_14380_" + i, b : "testb_" + i });
   }
   insertRecords(dbcl, objs);

   // if insert fail, exit
   if(10000 != dbcl.count())
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }

   checkFullSyncToES(csName, clName, textIndexName, 10000);
   
   // update textIndex
   dbcl.update({"$set" : {"a" : "update text index 0"}}, {"a" : {"$gt" : "test_14380_1000"}});
   dbcl.update({"$set" : {"a" : "update text index 1"}}, {"b" : {"$et" : "testb_0"}});
   checkFullSyncToES(csName, clName, textIndexName, 10000);

   // check result
   var dbOpr = new DBOperator();
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a" : {"$include" : 1}});
   var expResult = dbOpr.findFromCL(dbcl, null, {"a" : {"$include" : 1}});
   actResult.sort(compare("a", compare("b")));
   expResult.sort(compare("a", compare("b")));
   checkResult(expResult, actResult);  
 
   // update commonIndex
   dbcl.update({"$set" : {"b" : "update common index 0"}}, {"a" : {"$gt" : "test_14380_1000"}});
   dbcl.update({"$set" : {"b" : "update common index 1"}}, {"b" : {"$et" : "testb_0"}});
   checkFullSyncToES(csName, clName, textIndexName, 10000);

   // check result
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a" : {"$include" : 1}});
   var expResult = dbOpr.findFromCL(dbcl, null, {"a" : {"$include" : 1}});
   actResult.sort(compare("a", compare("b")));
   expResult.sort(compare("a", compare("b")));
   checkResult(expResult, actResult);
 
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

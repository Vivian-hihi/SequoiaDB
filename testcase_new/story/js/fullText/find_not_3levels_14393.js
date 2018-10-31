/************************************
*@Description: 全文检索与普通查询的3层not组合验证  
*@author:      liuxiaoxuan
*@createdate:  2018.10.26
*@testlinkCase: seqDB-14393
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   var csName = COMMCSNAME + "_ES_14393";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   //create CL
   var clName = COMMCLNAME + "_ES_14393";
   var dbcl = commCreateCL( db, csName, clName );
   
   var textIndexName = "textIndex";   
   dbcl.createIndex(textIndexName, {"a" : "text"});
   
   // insert
   var objs = new Array();
   for(var i = 0; i < 20000; i++)
   {
      objs.push({a: "test_14393 " + i, b :  i });
   }
   insertRecords(dbcl, objs);

   // if insert fail, exit
   if(20000 != dbcl.count())
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }

   checkFullSyncToES(csName, clName, textIndexName, 20000);
   
   // match 0 record
   var findNoneConf1 = {"$not": [{"$and":[{"$and":[{"b" : {"$gte" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14393"}}}}}]}]},{"a" : {"$isnull":0}}]}; //not-and-and
   var findNoneConf2 = {"$not": [{"$and":[{"$or":[{"b" : {"$gte" : 0}},{"a" : {"$isnull":0}}]}]},{"":{"$Text":{"query":{"match":{"a" : "test_14393"}}}}}]}; //not-and-or
   var findNoneConf3 = {"$not": [{"$and":[{"$not":[{"b" : {"$gte" : 0}},{"a" : {"$isnull":1}}]}]},{"":{"$Text":{"query":{"match":{"a" : "test_14393"}}}}}]}; //not-and-not
   var findNoneConf4 = {"$not": [{"$or":[{"$and":[{"b" : {"$gte" : 0}},{"a" : {"$isnull":0}}]}]},{"":{"$Text":{"query":{"match":{"a" : "test_14393"}}}}}]}; //not-or-and
//   var findNoneConf5 = {"$not": [{"$or":[{"$or":[{"b" : {"$gte" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14393"}}}}}]}]},{"a" : {"$isnull":0}}]}; //not-or-or, bug #SEQUOIADBMAINSTREAM-3389
//   var findNoneConf6 = {"$not":[{"$or":[{"$not":[{"b" : {"$lt" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14393 2"}}}}}]}]},{"a" : {"$exists":1}}]} //not-or-not, bug #SEQUOIADBMAINSTREAM-3389
   var actResult1 = dbOpr.findFromCL(dbcl, findNoneConf1);
   var actResult2 = dbOpr.findFromCL(dbcl, findNoneConf2);
   var actResult3 = dbOpr.findFromCL(dbcl, findNoneConf3);
   var actResult4 = dbOpr.findFromCL(dbcl, findNoneConf4);
//   var actResult5 = dbOpr.findFromCL(dbcl, findNoneConf5);
//   var actResult6 = dbOpr.findFromCL(dbcl, findNoneConf6);
   var expResult = [];
   checkResult(expResult, actResult1);
   println("---match 0 record for $not-$and-$and---");
   checkResult(expResult, actResult2);
   println("---match 0 record for $not-$and-$or---");
   checkResult(expResult, actResult3);
   println("---match 0 record for $not-$and-$not---");
   checkResult(expResult, actResult4);
   println("---match 0 record for $not-$or-$and---");
//   checkResult(expResult, actResult5);
//   checkResult(expResult, actResult6);

   // match some records
   var findSomeConf1 = {"$not":[{"$and":[{"$and":[{"b" : {"$lte" : 10000}},{"":{"$Text":{"query":{"match_phrase":{"a" : "test_14393"}}}}}]}, {b:{"$lt":10000}}]},{"a" : {"$exists":1}}]}; //not-and-and
//   var findSomeConf2 = {"$not":[{"$and":[{"$or":[{"b" : {"$lte" : 10000}},{"":{"$Text":{"query":{"match_phrase":{"a" : "test_14393"}}}}}]}, {b:{"$lt":10000}}]},{"a" : {"$exists":1}}]}; //not-and-or, bug #SEQUOIADBMAINSTREAM-3389
   var findSomeConf3 = {"$not":[{"$and":[{"$not":[{"b" : {"$gte" : 10000}},{"":{"$Text":{"query":{"match_phrase":{"a" : "test_14393"}}}}}]}, {b:{"$lt":10000}}]},{"a" : {"$exists":1}}]}; //not-and-not
//   var findSomeConf4 = {"$not":[{"$or":[{"$and":[{"b" : {"$lt" : 10000}},{"":{"$Text":{"query":{"match_phrase":{"a" : "test_14393"}}}}}]}, {b:{"$lt":10000}}]},{"a" : {"$exists":1}}]}; //not-or-and, bug #SEQUOIADBMAINSTREAM-3389
//   var findSomeConf5 = {"$not":[{"$or":[{"$not":[{"b" : {"$gte" : 10000}},{"":{"$Text":{"query":{"match_phrase":{"a" : "test_14393"}}}}}]}, {b:{"$lt":10000}}]},{"a" : {"$exists":1}}]}; //not-or-not, bug #SEQUOIADBMAINSTREAM-3389
   var actResult1 = dbOpr.findFromCL(dbcl, findSomeConf1, {'a' : ''});
//   var actResult2 = dbOpr.findFromCL(dbcl, findSomeConf2, {'a' : ''});
   var actResult3 = dbOpr.findFromCL(dbcl, findSomeConf3, {'a' : ''});
//   var actResult4 = dbOpr.findFromCL(dbcl, findSomeConf4, {'a' : ''});
//   var actResult5 = dbOpr.findFromCL(dbcl, findSomeConf5, {'a' : ''});
   var expResult = dbOpr.findFromCL(dbcl, {"b": {"$gte" : 10000}}, {'a' : ''});
   actResult1.sort(compare("a"));
//   actResult2.sort(compare("a"));
   actResult3.sort(compare("a"));
//   actResult4.sort(compare("a"));
//   actResult5.sort(compare("a"));
   checkResult(expResult, actResult1);
   println("---match some records for $not-$and-$and---");
//   checkResult(expResult, actResult2);
   checkResult(expResult, actResult3);
   println("---match some records for $not-$and-$not---");
//   checkResult(expResult, actResult4);
//   checkResult(expResult, actResult5);
   

   // match all records
   var findAllConf1 = {"$not":[{"$and":[{"$and":[{"b" : {"$lt" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14393 2"}}}}}]}]},{"a" : {"$exists":1}}]}; //not-and-and
//   var findAllConf2 = {"$not":[{"$and":[{"$or":[{"b" : {"$lt" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14393 2"}}}}}]}]},{"a" : {"$isnull":1}}]}; //not-and-or, bug #SEQUOIADBMAINSTREAM-3389
   var findAllConf3 = {"$not":[{"$and":[{"$not":[{"b" : {"$lt" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14393 2"}}}}}]}]},{"a" : {"$isnull":1}}]}; //not-and-not
//   var findAllConf4 = {"$not":[{"$or":[{"$and":[{"b" : {"$lt" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14393 2"}}}}}]}]},{"a" : {"$isnull":1}}]}; //not-or-and, bug #SEQUOIADBMAINSTREAM-3389
//   var findAllConf5 = {"$not":[{"$or":[{"$or":[{"b" : {"$lt" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14393 2"}}}}}]}]},{"a" : {"$exists":0}}]}; //not-or-or, bug #SEQUOIADBMAINSTREAM-3389
//   var findAllConf6 = {"$not":[{"$or":[{"$not":[{"b" : {"$gte" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14393 2"}}}}}]}]},{"a" : {"$exists":0}}]}; //not-or-not, bug #SEQUOIADBMAINSTREAM-3389
   var actResult1 = dbOpr.findFromCL(dbcl, findAllConf1, {'a' : ''});
//   var actResult2 = dbOpr.findFromCL(dbcl, findAllConf2, {'a' : ''});
   var actResult3 = dbOpr.findFromCL(dbcl, findAllConf3, {'a' : ''});
//   var actResult4 = dbOpr.findFromCL(dbcl, findAllConf4, {'a' : ''});
//   var actResult5 = dbOpr.findFromCL(dbcl, findAllConf5, {'a' : ''});
//   var actResult6 = dbOpr.findFromCL(dbcl, findAllConf6, {'a' : ''});
   var expResult = dbOpr.findFromCL(dbcl, null, {'a' : ''});
   actResult1.sort(compare("a"));
//   actResult2.sort(compare("a"));
   actResult3.sort(compare("a"));
//   actResult4.sort(compare("a"));
//   actResult5.sort(compare("a"));
//   actResult6.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult1); 
   println("---match all records for $not-$and-$and---");
//   checkResult(expResult, actResult2); 
   checkResult(expResult, actResult3); 
   println("---match all records for $not-$and-$not---");
//   checkResult(expResult, actResult4); 
//   checkResult(expResult, actResult5); 
//   checkResult(expResult, actResult6); 
 
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

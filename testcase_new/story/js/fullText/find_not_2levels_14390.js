/*onf1
*@Description: 全文检索与普通查询的2层not组合验证  
*@author:      liuxiaoxuan
*@createdate:  2018.10.28
*@testlinkCase: seqDB-14390
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   } 

   commDropCL(db, COMMCSNAME, clName, true, true); 
                                                             	
   //create CL
   var clName = COMMCLNAME + "_ES_14390";
   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   
   var textIndexName = "textIndex";   
   dbcl.createIndex(textIndexName, {"a" : "text"});
   
   // insert
   var objs = new Array();
   for(var i = 0; i < 20000; i++)
   {
      objs.push({a: "test_14390 " + i, b :  i });
   }
   insertRecords(dbcl, objs);

   // if insert fail, exit
   if(20000 != dbcl.count())
   {
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }

   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 20000);
   
   // match 0 record
   var findNoneConf1 = {"$not":[{"$and":[{"b" : {"$gte" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}]}; // not-and, fulltext search under "$and"
   var findNoneConf2 = {"$not":[{"$and":[{"b" : {"$gte" : 0}},{"b" : {"$lte": 20000}}]}, {"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}; // not-and, fulltext search under "$not"
   //var findNoneConf3 = {"$not":[{"$or":[{"b" : {"$gte" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}]}; // not-or, fulltext search under "$or", bug #SEQUOIADBMAINSTREAM-3389
   var findNoneConf4 = {"$not":[{"$or":[{"b" : {"$gte" : 0}},{"b" : {"$lte": 20000}}]}, {"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}; //not-or, fulltext search under "$not"
   var findNoneConf5 = {"$not" : [{"$not":[{"b" : {"$lt" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}]}; // not-not 
   var actResult1 = dbOpr.findFromCL(dbcl, findNoneConf1);
   var actResult2 = dbOpr.findFromCL(dbcl, findNoneConf2);
   var actResult4 = dbOpr.findFromCL(dbcl, findNoneConf4);
   var actResult5 = dbOpr.findFromCL(dbcl, findNoneConf5);
   var expResult = [];
   checkResult(expResult, actResult1);
   println("---match 0 record for $not-$and---");
   checkResult(expResult, actResult2);
   println("---match 0 record for $not-$and---");
   checkResult(expResult, actResult4);
   println("---match 0 record for $not-$or---");
   checkResult(expResult, actResult5);
   println("---match 0 record for $not-$not---");

   // match some records
   var findSomeConf1 = {"$not":[{"$and":[{"b" : {"$gte" : 10000}},{"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}]}; // not-and, fulltext search under "$and"
   var findSomeConf2 = {"$not":[{"$and":[{"b" : {"$gte" : 10000}},{"b" : {"$lte": 20000}}]}, {"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}; // not-and, fulltext search under "$not"
   var findSomeConf3 = {"$not":[{"$or":[{"b" : {"$lt" : -1}},{"b" : {"$gte": 10000}}]}, {"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}; // not-or, fulltext search under "$not"
   var findSomeConf4 = {"$not" : [{"$not":[{"b" : {"$lt" : 10000}},{"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}]}; // not-not
   var actResult1 = dbOpr.findFromCL(dbcl, findSomeConf1, {'a' : ''});
   var actResult2 = dbOpr.findFromCL(dbcl, findSomeConf2, {'a' : ''});
   var actResult3 = dbOpr.findFromCL(dbcl, findSomeConf3, {'a' : ''});
   var actResult4 = dbOpr.findFromCL(dbcl, findSomeConf4, {'a' : ''});
   var expResult = dbOpr.findFromCL(dbcl, {"b": {"$lt" : 10000}}, {'a' : ''});
   actResult1.sort(compare("a"));
   actResult2.sort(compare("a"));
   actResult3.sort(compare("a"));
   actResult4.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult1);
   println("---match some records for $not-$and---");
   checkResult(expResult, actResult2);
   println("---match some records for $not-$and---");
   checkResult(expResult, actResult3);
   println("---match some records for $not-$or---");
   checkResult(expResult, actResult4);
   println("---match some records for $not-$not---"); 
   // match all records
   var findAllConf1 = {"$not":[{"$and":[{"b" : {"$lte" : -1}},{"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}]}; // not-and, fulltext search under "$and" 
   var findAllConf2 = {"$not":[{"$and":[{"b" : {"$gte" : 20000}},{"b" : {"$lte": -1}}]}, {"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}; // not-and, fulltext search under "$not"
   var findAllConf3 = {"$not":[{"$or":[{"b" : {"$lt" : -1}},{"b" : {"$gte": 20000}}]}, {"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}; // not-or, fulltext search under "$not"
   var findAllConf4 = {"$not" : [{"$not":[{"b" : {"$gt" : -1}},{"":{"$Text":{"query":{"match":{"a" : "test_14390"}}}}}]}]}; // not-not
   var actResult1 = dbOpr.findFromCL(dbcl, findAllConf1, {'a' : ''});
   var actResult2 = dbOpr.findFromCL(dbcl, findAllConf2, {'a' : ''});
   var actResult3 = dbOpr.findFromCL(dbcl, findAllConf3, {'a' : ''});
   var actResult4 = dbOpr.findFromCL(dbcl, findAllConf4, {'a' : ''});
   var expResult = dbOpr.findFromCL(dbcl, null, {'a' : ''});
   actResult1.sort(compare("a"));
   actResult2.sort(compare("a"));
   actResult3.sort(compare("a"));
   actResult4.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult1);
   println("---match all records for $not-$and---");
   checkResult(expResult, actResult2);
   println("---match all records for $not-$and---");
   checkResult(expResult, actResult3);
   println("---match all records for $not-$or---");
   checkResult(expResult, actResult4);
   println("---match all records for $not-$not---"); 

   commDropCL(db, COMMCSNAME, clName, true, true);
}
main();

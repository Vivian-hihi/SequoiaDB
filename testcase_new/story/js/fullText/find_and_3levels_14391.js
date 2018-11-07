/************************************
*@Description: 全文检索与普通查询的3层and组合验证  
*@author:      liuxiaoxuan
*@createdate:  2018.10.28
*@testlinkCase: seqDB-14391
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   commDropCL(db, COMMCSNAME, clName, true, true);
                                                              	
   //create CL
   var clName = COMMCLNAME + "_ES_14391";
   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   
   var textIndexName = "textIndex";   
   dbcl.createIndex(textIndexName, {"a" : "text"});
   
   // insert
   var objs = new Array();
   for(var i = 0; i < 20000; i++)
   {
      objs.push({a: "test_14391 " + i, b :  i });
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
   var findNoneConf1 = {"$and": [{"$and":[{"$and":[{"b" : {"$gte" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14391"}}}}}]}]},{"a" : {"$isnull":1}}]}; //and-and-and
   var findNoneConf2 = {"$and": [{"$and":[{"$or":[{"b" : {"$lt" : 0}},{"a" : {"$isnull":1}}]}]},{"":{"$Text":{"query":{"match":{"a" : "test_14391"}}}}}]}; //and-and-or
   var findNoneConf3 = {"$and": [{"$and":[{"$not":[{"b" : {"$gte" : 0}},{"a" : {"$isnull":0}}]}]},{"":{"$Text":{"query":{"match":{"a" : "test_14391"}}}}}]}; //and-and-not
   var findNoneConf4 = {"$and": [{"$or":[{"$and":[{"b" : {"$gte" : 0}},{"a" : {"$isnull":1}}]}]},{"":{"$Text":{"query":{"match":{"a" : "test_14391"}}}}}]}; //and-or-and
//   var findNoneConf5 = {"$and": [{"$or":[{"$or":[{"b" : {"$gte" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14391"}}}}}]}]},{"a" : {"$isnull":0}}]}; //and-or-or, bug #SEQUOIADBMAINSTREAM-3389
//   var findNoneConf6 = {"$and":[{"$or":[{"$not":[{"b" : {"$lt" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14391 2"}}}}}]}]},{"a" : {"$exists":1}}]} //and-or-not, bug #SEQUOIADBMAINSTREAM-3389
   var actResult1 = dbOpr.findFromCL(dbcl, findNoneConf1);
   var actResult2 = dbOpr.findFromCL(dbcl, findNoneConf2);
   var actResult3 = dbOpr.findFromCL(dbcl, findNoneConf3);
   var actResult4 = dbOpr.findFromCL(dbcl, findNoneConf4);
//   var actResult5 = dbOpr.findFromCL(dbcl, findNoneConf5);
//   var actResult6 = dbOpr.findFromCL(dbcl, findNoneConf6);
   var expResult = [];
   checkResult(expResult, actResult1);
   println("---match 0 record for $and-$and-$and---");
   checkResult(expResult, actResult2);
   println("---match 0 record for $and-$and-$or---");
   checkResult(expResult, actResult3);
   println("---match 0 record for $and-$and-$not---");
   checkResult(expResult, actResult4);
   println("---match 0 record for $and-$or-$and---");
//   checkResult(expResult, actResult5);
//   checkResult(expResult, actResult6);

   // match some records
   var findSomeConf1 = {"$and":[{"$and":[{"$and":[{"b" : {"$lte" : 10000}},{"":{"$Text":{"query":{"match_phrase":{"a" : "test_14391"}}}}}]}, {b:{"$lt":10000}}]},{"a" : {"$exists":1}}]}; //and-and-and
   var findSomeConf2 = {"$and":[{"$and":[{"$not":[{"b" : {"$gte" : 10000}},{"":{"$Text":{"query":{"match_phrase":{"a" : "test_14391"}}}}}]}, {b:{"$lt":10000}}]},{"a" : {"$exists":1}}]}; //and-and-not
   var actResult1 = dbOpr.findFromCL(dbcl, findSomeConf1, {'a' : ''});
   var actResult2 = dbOpr.findFromCL(dbcl, findSomeConf2, {'a' : ''});
   var expResult = dbOpr.findFromCL(dbcl, {"b": {"$lt" : 10000}}, {'a' : ''});
   actResult1.sort(compare("a"));
   actResult2.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult1);
   println("---match some records for $and-$and-$and---");
   checkResult(expResult, actResult2);
   println("---match some records for $and-$and-$not---");
 
   // match all records
   var findAllConf1 = {"$and":[{"$and":[{"$and":[{"b" : {"$gte" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14391 2"}}}}}]}]},{"a" : {"$exists":1}}]}; //and-and-and
   var findAllConf2 = {"$and":[{"$and":[{"$not":[{"b" : {"$lt" : 0}},{"":{"$Text":{"query":{"match":{"a" : "test_14391 2"}}}}}]}]},{"a" : {"$isnull":0}}]}; //and-and-not
   var actResult1 = dbOpr.findFromCL(dbcl, findAllConf1, {'a' : ''});
   var actResult2 = dbOpr.findFromCL(dbcl, findAllConf2, {'a' : ''});
   var expResult = dbOpr.findFromCL(dbcl, null, {'a' : ''});
   actResult1.sort(compare("a"));
   actResult2.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult1);
   println("---match all records for $and-$and-$and---");
   checkResult(expResult, actResult2);
   println("---match all records for $and-$and-$not---");

   commDropCL(db, COMMCSNAME, clName, true, true); 
}
main();

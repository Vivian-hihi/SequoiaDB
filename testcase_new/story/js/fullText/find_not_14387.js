/************************************
*@Description: 全文检索与普通查询的1层not组合验证  
*@author:      liuxiaoxuan
*@createdate:  2018.10.26
*@testlinkCase: seqDB-14387
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   commDropCL(db, COMMCSNAME, clName, true, true);
                                                              	
   //create CL
   var clName = COMMCLNAME + "_ES_14387";
   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   
   var textIndexName = "textIndex";   
   dbcl.createIndex(textIndexName, {"a" : "text"});
   
   // insert
   var objs = new Array();
   for(var i = 0; i < 20000; i++)
   {
      objs.push({a: "test_14387 " + i, b :  i });
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
   var findNoneConf = {"$not": [{"b": {"$gte" : 0}}, {"":{"$Text":{"query":{"match":{"a" : "test_14387"}}}}}]}; 
   var actResult = dbOpr.findFromCL(dbcl, findNoneConf);
   var expResult = [];
   checkResult(expResult, actResult);
   println("---match 0 record---");

   // match some records
   var findSomeConf = {"$not": [{"b": {"$gte" : 10000}}, {"":{"$Text":{"query":{"match":{"a" : "test_14387"}}}}}]};
   var actResult = dbOpr.findFromCL(dbcl, findSomeConf, {'a' : ''});
   var expResult = dbOpr.findFromCL(dbcl, {"b": {"$lt" : 10000}}, {'a' : ''});
   actResult.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult);
   println("---match some records---");
   
   // match all records
   var findAllConf = {"$not": [{"b": {"$et" : 0}}, {"":{"$Text":{"query":{"match_phrase":{"a" : "test_14387 2"}}}}}]};
   var actResult = dbOpr.findFromCL(dbcl, findAllConf, {'a' : ''});
   var expResult = dbOpr.findFromCL(dbcl, null, {'a' : ''});
   actResult.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult); 
   println("---match all records---");

   commDropCL(db, COMMCSNAME, clName, true, true); 
}
main();

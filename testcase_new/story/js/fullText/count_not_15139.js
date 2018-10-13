/************************************
*@Description: count命令字中使用全文检索与普通查询的not组合
*@author:      liuxiaoxuan
*@createdate:  2018.10.08
*@testlinkCase: seqDB-15139
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   var csName = COMMCSNAME + "_ES_15139";
   commDropCS( db, csName, true, "drop CS in the beginning" );
                                                             	
   commCreateCS( db, csName, false, "" );
                                                              	
   //create CL
   var clName = COMMCLNAME + "_ES_15139";
   var dbcl = commCreateCL( db, csName, clName );
   
   var textIndexName = "textIndex";   
   var commIndexName = "commonIndex";   
   dbcl.createIndex(textIndexName, {"a" : "text"});
   dbcl.createIndex(commIndexName, {"b" : 1});
   
   // insert
   var objs = new Array();
   for(var i = 0; i < 20000; i++)
   {
      objs.push({a: "test_15139 " + i, b : "testb_" + i });
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
   var findNoneConf = {"$not": [{"b": {"$gte" : "testb_0"}}, {"":{"$Text":{"query":{"match":{"a" : "test_15139"}}}}}]}; 
   var actCount = dbcl.count(findNoneConf);
   var expectCount = 0;
   checkCount(expectCount, actCount);

   // match some records
   var findSomeConf = {"$not": [{"b": {"$gte" : "testb_9"}}, {"":{"$Text":{"query":{"match":{"a" : "test_15139"}}}}}]};
   var actCount = dbcl.count(findSomeConf);
   var expectCount = 18889;
   checkCount(expectCount, actCount);

   // match all records
   var findAllConf = {"$not": [{"b": {"$et" : "testb_0"}}, {"":{"$Text":{"query":{"match_phrase":{"a" : "test_15139 2"}}}}}]};
   var actCount = dbcl.count(findAllConf);
   var expectCount = 20000;
   checkCount(expectCount, actCount);
  
   commDropCS( db, csName, true, "drop CS in the end" );
}
function checkCount( expectCount, actCount )
{
   if(expectCount != actCount)
   {
      throw buildException("checkCount()", "check count", "check count", expectCount, actCount);
   }
   println("check result success!");
}

main();

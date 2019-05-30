/************************************
*@Description: 正常停止所有编目节点并重启所有节点，创建全文索引
*@author:      liuxiaoxuan
*@createdate:  2019.05.08
*@testlinkCase: seqDB-18267
**************************************/

function main()
{
   if(commIsStandalone(db))  {   return ;   } 

   //create CL
   var clName = COMMCLNAME + "_ES_18267";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   
   // insert
   var objs = new Array();
   for(var i = 0; i < 20000; i++)
   {
      objs.push({a: "test_18267 " + i, b :  i });
   }
   dbcl.insert(objs);
   
   // stop all nodes
   var node1 = db.getRG("SYSCatalogGroup").getSlave(1);
   var node2 = db.getRG("SYSCatalogGroup").getSlave(2);
   var node3 = db.getRG("SYSCatalogGroup").getSlave(3);
   node1.stop();
   node2.stop();
   node3.stop();
   
   // start nodes
   node1.start();
   node2.start();
   node3.start();
   
   // wait for reelect primary node
   sleep(10000);
      
   // create fulltext
   var textIndexName = "textIndex_18267";   
   dbcl.createIndex(textIndexName, {"a" : "text"});
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 20000);
   
   // query
   var findConf = {"$not": [{"b": {"$gte" : 10000}}, {"":{"$Text":{"query":{"match":{"a" : "test_18267"}}}}}]};
   var actResult = dbOpr.findFromCL(dbcl, findConf, {'a' : ''});
   var expResult = dbOpr.findFromCL(dbcl, {"b": {"$lt" : 10000}}, {'a' : ''});
   actResult.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult);
   println("---check result success---");
   
   var esIndexNames = dbOpr.getESIndexNames(COMMCSNAME, clName, textIndexName);
   commDropCL(db, COMMCSNAME, clName, true, true); 
   //SEQUOIADBMAINSTREAM-3983
   checkIndexNotExistInES(esIndexNames);
}
main();

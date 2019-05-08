/************************************
*@Description: 正常停止编目主节点，选出新主后创建全文索引
*@author:      liuxiaoxuan
*@createdate:  2019.05.08
*@testlinkCase: seqDB-18265
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   //create CL
   var clName = COMMCLNAME + "_ES_18265";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   
   // insert
   var objs = new Array();
   for(var i = 0; i < 20000; i++)
   {
      objs.push({a: "test_18265 " + i, b :  i });
   }
   dbcl.insert(objs);
   
   // stop catalog primary node
   var preCataMaster = db.getRG("SYSCatalogGroup").getMaster();
   var preCataMasterNodeName = preCataMaster.getHostName() + ":" + preCataMaster.getServiceName();
   preCataMaster.stop();
   
   // wait for change primary node
   while(true)
   {
      var curCataMaster = db.getRG("SYSCatalogGroup").getMaster();
      var curCataMasterNodeName = curCataMaster.getHostName() + ":" + curCataMaster.getServiceName();
      // when change primary node, break
      if(preCataMasterNodeName != curCataMasterNodeName) 
      {
         break;
      }
   }
   
   // start node
   preCataMaster.start();

   var textIndexName = "textIndex_18265";   
   dbcl.createIndex(textIndexName, {"a" : "text"});
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 20000);
   
   // query
   var findConf = {"$not": [{"b": {"$gte" : 10000}}, {"":{"$Text":{"query":{"match":{"a" : "test_18265"}}}}}]};
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

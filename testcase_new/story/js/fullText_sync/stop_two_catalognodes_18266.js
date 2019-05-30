/************************************
*@Description: 正常停止两个编目节点后再次重启一个节点，并创建全文索引
*@author:      liuxiaoxuan
*@createdate:  2019.05.08
*@testlinkCase: seqDB-18266
**************************************/

function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   //create CL
   var clName = COMMCLNAME + "_ES_18266";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   
   // insert
   var objs = new Array();
   for(var i = 0; i < 20000; i++)
   {
      objs.push({a: "test_18266 " + i, b :  i });
   }
   dbcl.insert(objs);
   
   // stop catalog primary node and one of slave nodes
   var preCataMaster = db.getRG("SYSCatalogGroup").getMaster();
   var preCataSlave = db.getRG("SYSCatalogGroup").getSlave();
   var preCataMasterNodeName = preCataMaster.getHostName() + ":" + preCataMaster.getServiceName();
   var preCataSlaveNodeName = preCataSlave.getHostName() + ":" + preCataSlave.getServiceName();
   preCataMaster.stop();
   preCataSlave.stop();
   
   // start one of nodes
   preCataSlave.start();
   
   // wait for change primary node
   while(true)
   {
      try{
         var curCataMaster = db.getRG("SYSCatalogGroup").getMaster();
         var curCataMasterNodeName = curCataMaster.getHostName() + ":" + curCataMaster.getServiceName();
         // when change primary node, break
         if(preCataMasterNodeName != curCataMasterNodeName) 
         {
            break;
         }
      }catch(e){
         if(-104 == e){
            continue;
         }
      }
   }
   
   // start the rest of node
   preCataMaster.start();

   // create fulltext
   var textIndexName = "textIndex_18266";   
   dbcl.createIndex(textIndexName, {"a" : "text"});
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 20000);
   
   // query
   var findConf = {"$not": [{"b": {"$gte" : 10000}}, {"":{"$Text":{"query":{"match":{"a" : "test_18266"}}}}}]};
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

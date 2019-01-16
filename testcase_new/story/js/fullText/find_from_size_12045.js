/************************************
*@Description: 带from/size进行全文检索  
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-12045
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   //create CL
   var clName = COMMCLNAME + "_ES_12045";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var dbcl = commCreateCL( db, COMMCSNAME, clName );

   var textIndexName = "textIndex_12045";
   dbcl.createIndex(textIndexName, {"a" : "text"});

   // insert
   var objs = new Array();
   for(var i = 0; i < 10001; i++)
   {   
      objs.push({a: "test_12045_" + i});
   }   
   insertRecords(dbcl, objs);

   // if insert fail, exit
   if(10001 != dbcl.count())
   {   
      println("---insert has an err:SEQUOIADBMAINSTREAM-3827");
      return ;
   }   
  
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 10001);
 
   var esOpr = new ESOperator(); 
   var dbOpr = new DBOperator();
   var esIndexNames = dbOpr.getESIndexNames(COMMCSNAME, clName, textIndexName);
 
   // from 
   var findCond = {"":{"$Text":{"query":{"match_all":{}}, "from": 9990}}};
   var searchCond = '{"query":{"match_all":{}}, "from": 9990}'
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a" : {"$include" : 1}});
   var expResult = esOpr.findFromES(esIndexNames[0], searchCond);
   actResult.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult); 
  
   // size
   var findCond = {"":{"$Text":{"query":{"match_all":{}}, "size": 10000}}};
   var searchCond = '{"query":{"match_all":{}}, "size": 10000}'
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a" : {"$include" : 1}});
   var expResult = esOpr.findFromES(esIndexNames[0], searchCond);
   actResult.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult);

   // from+size < 10000
   var findCond = {"":{"$Text":{"query":{"match_all":{}}, "from": 1, "size": 9990}}};
   var searchCond = '{"query":{"match_all":{}}, "from": 1, "size": 9990}'
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a" : {"$include" : 1}});
   var expResult = esOpr.findFromES(esIndexNames[0], searchCond);
   actResult.sort(compare("a"));
   expResult.sort(compare("a"));
   checkResult(expResult, actResult);

   // from+size > 10000, should fail
   try
   {
      var rec = dbcl.find({"":{"$Text":{"query":{"match_all":{}}, "from": 0, "size":10001}}});
      rec.next();
      throw buildException("find()", "find", "find es overrize", "fail","success");
   }
   catch(e)
   {
      if(-10 != e)
      {
         throw buildException("find()", "find", "find other exception", e, e);
      }
   }

   commDropCL(db, COMMCSNAME, clName, true, true); 
}
main();

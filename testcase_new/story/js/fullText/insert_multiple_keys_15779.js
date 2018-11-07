/************************************
*@Description: 多键全文索引，全部字段为数组类型，ES从固定集合/原始集合中同步记录   
*@author:      liuxiaoxuan
*@createdate:  2018.10.10
*@testlinkCase: seqDB-15779
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   // create CL
   var clName = COMMCLNAME + "_ES_15779";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var dbcl = commCreateCL( db, COMMCSNAME, clName );

   // insert before create text index
   dbcl.insert({a: ["arr1", "arr2"], b: ["b1", "b2"], c: ["c1", "c2"]});

   var textIndexName = "textIndex";
   dbcl.createIndex(textIndexName, {"a" : "text", "b" : "text", "c" : "text"});

   // check sync to es
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 0);
   
   // check result
   var dbOpr = new DBOperator();
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var actResult = dbOpr.findFromCL(dbcl, findCond);
   checkResult([], actResult);
   
   // insert more than one array after create text index, should fail
   try
   {
      dbcl.insert({a: ["arr1", "arr2"], b: ["b1", "b2"], c: ["c1", "c2"]});
      throw buildException("insert()", "insert", "insert many arrays of keys", "fail","success");
   }
   catch(e)
   {
      if(-37 != e)
      {
         throw buildException("insert()", "insert", "insert other exception", e, e);
      }
   }

   //check result
   var actResult = dbOpr.findFromCL(dbcl, findCond);
   checkResult([], actResult);

   commDropCL(db, COMMCSNAME, clName, true, true);
}
main();

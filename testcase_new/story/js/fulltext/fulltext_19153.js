/************************************
*@Description: seqDB-19153:多键全文索引，索引字段为数组元素，更新全部全文索引字段为非string类型
*@author:      zhaoyu
*@createdate:  2019.08.14
*@testlinkCase: seqDB-19153
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var clName = COMMCLNAME + "_19153";
   var textIndexName = "textIndex_19153";
   commDropCL(db, COMMCSNAME, clName, true, true);
   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   dbcl.createIndex(textIndexName, {"a.1" : "text", "a.2":"text"});
   var objs = new Array({id:1, a: "string1", b:"string"},
                        {id:2, a: 1, b: 1},
                        {id:7, a:{0:"obj3", 1:"obj4", 2:"obj5"}, b:{0:"obj3", 1:"obj4", 2:"obj5"}});
   dbcl.insert(objs);
   dbcl.update({$set:{a:{0:1, 1:2, 2:3}}});
   
   var dbOpr = new DBOperator();
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 0);
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1}}, {_id:1});
   var expResult = [];
   checkResult(expResult, actResult);
                                       
   var esIndexNames = dbOpr.getESIndexNames(COMMCSNAME, clName, textIndexName);
   commDropCL(db, COMMCSNAME, clName, true, true);
   //SEQUOIADBMAINSTREAM-3983
   checkIndexNotExistInES(esIndexNames);
}
try
{
   main();
}
catch(e)
{
   if ( e.constructor === Error )
   {
      println(e.stack) ;  
   }
   throw e ;
}
;
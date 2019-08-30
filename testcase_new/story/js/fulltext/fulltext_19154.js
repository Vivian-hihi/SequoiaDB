/************************************
*@Description: seqDB-19154:多键全文索引，索引字段为数组元素，更新部分全文索引字段为string
*@author:      zhaoyu
*@createdate:  2019.08.14
*@testlinkCase: seqDB-19154
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var clName = COMMCLNAME + "_19154";
   var textIndexName = "textIndex_19154";
   commDropCL(db, COMMCSNAME, clName, true, true);
   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   dbcl.createIndex(textIndexName, {"a.1" : "text", "a.2":"text"});
   var objs = new Array({id:1, a: "string1", b:"string"},
                        {id:2, a: 1, b: 1},
                        {id:7, a:{0:"obj3", 1:"obj4", 2:"obj5"}, b:{0:"obj3", 1:"obj4", 2:"obj5"}});
   dbcl.insert(objs);
   dbcl.update({$set:{a:{0:"update", 1:"update",}}});
   
   var dbOpr = new DBOperator();
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 3);
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1}, "b":{"$include":1}}, {_id:1});
   var expResult = [{a:{0:"update", 1:"update"}, b:"string"},
                    {a:{0:"update", 1:"update"}, b: 1},
                    {a:{0:"update", 1:"update"}, b:{0:"obj3", 1:"obj4", 2:"obj5"}}];
   checkResult(expResult, actResult);
                                       
   var esIndexNames = dbOpr.getESIndexNames(COMMCSNAME, clName, textIndexName);
   commDropCL(db, COMMCSNAME, clName, true, true);
   //SEQUOIADBMAINSTREAM-3983
   checkIndexNotExistInES(esIndexNames);
}
main();
/************************************
*@Description: seqDB-19145:多键全文索引，索引字段为数组元素，全量/增量同步(不支持在多个数组元素上创建索引)
*@author:      zhaoyu
*@createdate:  2019.08.14
*@testlinkCase: seqDB-19145
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var clName = COMMCLNAME + "_19145";
   var textIndexName = "textIndex_19145";
   commDropCL(db, COMMCSNAME, clName, true, true);
   var dbcl = commCreateCL( db, COMMCSNAME, clName );

   //{id:5, a:[{0:"obj1"}, {1:"obj2"}, {2:"obj3"}], b:[{0:"obj1"}, {1:"obj2"}, {2:"obj3"}]}无法同步到ES
   var objs = new Array({id:1, a: "string1", b:"string"},
                        {id:2, a: 1, b: 1},
                        {id:3, a:["string1", "string2", "string3"], b:["string1", "string2", "string3"]},
                        {id:4, a:[1, 2, 3], b:[1, 2, 3]},
                        {id:5, a:[{0:"obj1"}, {1:"obj2"}, {2:"obj3"}], b:[{0:"obj1"}, {1:"obj2"}, {2:"obj3"}]},
                        {id:6, a:[{0:1}, {1:2}, {2:3}], b:[{0:1}, {1:2}, {2:3}]},
                        {id:7, a:{0:"obj3", 1:"obj4", 2:"obj5"}, b:{0:"obj3", 1:1, 2:"obj5"}});
   dbcl.insert(objs);
   dbcl.createIndex(textIndexName, {"a.1" : "text", "b.2":"text"});
   
   var dbOpr = new DBOperator();
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 1);
   var findCond = {"":{"$Text":{"query":{"match_all":{}}}}};
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1},"b":{"$include":1}}, {_id:1});
   var expResult = [{a:{0:"obj3", 1:"obj4", 2:"obj5"}, b:{0:"obj3", 1:1, 2:"obj5"}}];
   checkResult(expResult, actResult);
   
   //不支持插入多个数组元素为索引的记录，会报-37，暂时屏蔽该条记录
   objs = new Array({id:1, a: "string1", b:"string"},
                    {id:2, a: 1, b: 1},
                    {id:7, a:{0:"obj3", 1:"obj4", 2:"obj5"}, b:{0:"obj3", 1:1, 2:"obj5"}});
   dbcl.insert(objs);
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 2);
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1},"b":{"$include":1}}, {_id:1});
   var expResult = [{a:{0:"obj3", 1:"obj4", 2:"obj5"}, b:{0:"obj3", 1:1, 2:"obj5"}},
                    {a:{0:"obj3", 1:"obj4", 2:"obj5"}, b:{0:"obj3", 1:1, 2:"obj5"}}];
   
   findCond = {"":{$Text:{query:{bool:{must:[{match:{"a.1":"obj4"}},{match:{"b.2":"obj5"}}]}}}}};                 
   var actResult = dbOpr.findFromCL(dbcl, findCond, {"a":{"$include":1},"b":{"$include":1}}, {_id:1});
   var expResult = [{a:{0:"obj3", 1:"obj4", 2:"obj5"}, b:{0:"obj3", 1:1, 2:"obj5"}},
                    {a:{0:"obj3", 1:"obj4", 2:"obj5"}, b:{0:"obj3", 1:1, 2:"obj5"}}];
   
   dbcl.remove();
   checkFullSyncToES(COMMCSNAME, clName, textIndexName, 0);
                                             
   var esIndexNames = dbOpr.getESIndexNames(COMMCSNAME, clName, textIndexName);
   commDropCL(db, COMMCSNAME, clName, true, true);
   //SEQUOIADBMAINSTREAM-3983
   checkIndexNotExistInES(esIndexNames);
}
main();
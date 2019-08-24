/************************************
*@Description: seqDB-19146:单键全文索引，索引字段为数组元素，更新全文索引字段为string类型(更新整个数组)
*@author:      zhaoyu
*@createdate:  2019.08.14
*@testlinkCase: seqDB-19146
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }

   var clName = COMMCLNAME + "_19146_2";
   var textIndexName = "textIndex_19146";
   commDropCL(db, COMMCSNAME, clName, true, true);
   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   dbcl.createIndex(textIndexName, {"a.1" : "text"});
   var objs = new Array({id:1, a: "string1"},
                        {id:2, a: 1},
                        {id:3, a:["string1", "string2", "string3"]},
                        {id:4, a:[1, 2, 3]},
                        {id:5, a:[{0:"obj1"}, {1:"obj2"}, {2:"obj3"}]},
                        {id:6, a:[{0:1}, {1:2}, {2:3}]},
                        {id:7, a:{1:"obj"}});
   dbcl.insert(objs);
   dbcl.update({$set:{a:["update1","update2","update3"]}});
   
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
main();
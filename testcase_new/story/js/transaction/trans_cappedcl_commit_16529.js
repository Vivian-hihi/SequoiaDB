/************************************
*@Description: 开启事务，混合插入/pop操作并提交事务 
*@author:      liuxiaoxuan
*@createdate:  2018.11.08
*@testlinkCase: seqDB-16529
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   // create capped cs
   var csName = COMMCSNAME + "_capped_16529";
   commDropCS(db, csName, true, "drop cs in beginning");
   commCreateCS(db, csName, true, "", {Capped: true});

   // create capped cl
   var clName = COMMCLNAME + "_capped_16529";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var cappedcl = commCreateCLByOption( db, csName, clName, {Capped: true, Size: 1024, AutoIndexId: false} );

   cappedcl.insert([{a : 0}, {a : 1}]);

   // insert and pop some records, Direction: -1
   db.transBegin();
   cappedcl.insert({a : 2});
   cappedcl.pop({LogicalID: 56, Direction: -1});
   cappedcl.insert({a : 3})
   db.transCommit();

   // check result
   var expectResult = [{a : 0}, {a : 3}];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   cappedcl.truncate();
   cappedcl.insert([{a : 0}, {a : 1}]);

   // insert and pop some records, Direction: 1
   db.transBegin();
   cappedcl.insert({a : 2});
   cappedcl.pop({LogicalID: 56, Direction: 1});
   cappedcl.insert({a : 3});
   db.transCommit();

   // check result
   var expectResult = [{a : 2}, {a : 3}];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);
 
   cappedcl.truncate();
   cappedcl.insert([{a : 0}, {a : 1}]);

   // insert and pop all records, Direction: -1
   db.transBegin();
   cappedcl.insert({a : 2});
   cappedcl.pop({LogicalID: 0, Direction: -1});
   cappedcl.insert({a : 3})
   db.transCommit();

   // check result
   var expectResult = [{a : 3}];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   cappedcl.truncate();
   cappedcl.insert([{a : 0}, {a : 1}]);

   // insert and pop all records, Direction: 1
   db.transBegin();
   cappedcl.insert({a : 2});
   cappedcl.pop({LogicalID: 112, Direction: 1});
   cappedcl.insert({a : 3});
   db.transCommit();

   // check result
   var expectResult = [{a : 3}];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   commDropCS(db, csName, true, "drop cs in ending");
}
main();


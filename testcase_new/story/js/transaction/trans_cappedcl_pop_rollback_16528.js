/************************************
*@Description: 开启事务，执行pop操作并回滚事务 
*@author:      liuxiaoxuan
*@createdate:  2018.11.08
*@testlinkCase: seqDB-16528
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   // create capped cs
   var csName = COMMCSNAME + "_capped_16528";
   commDropCS(db, csName, true, "drop cs in beginning");
   commCreateCS(db, csName, true, "", {Capped: true});

   // create capped cl
   var clName = COMMCLNAME + "_capped_16528";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var cappedcl = commCreateCLByOption( db, csName, clName, {Capped: true, Size: 1024, AutoIndexId: false} );

   cappedcl.insert([{a : 0}, {a : 1}, {a : 2}, {a : 3}, {a : 4}]);

   // pop some records, Direction: -1
   db.transBegin();
   cappedcl.pop({LogicalID: 112, Direction: -1});
   db.transRollback();

   // check result
   var expectResult = [{a : 0}, {a : 1}];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   // pop some records, Direction: 1
   db.transBegin();
   cappedcl.pop({LogicalID: 0, Direction: 1});
   db.transRollback();

   // check result
   var expectResult = [{a : 1}];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   cappedcl.truncate();  
   cappedcl.insert([{a : 0}, {a : 1}]);  

   // pop all records, Direction: -1
   db.transBegin();
   cappedcl.pop({LogicalID: 0, Direction: -1});
   db.transRollback();

   // check result
   var expectResult = [];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   cappedcl.truncate();
   cappedcl.insert([{a : 0}, {a : 1}]);                           

   // pop all records, Direction: 1
   db.transBegin();
   cappedcl.pop({LogicalID: 56, Direction: 1});
   db.transRollback();

   // check result
   var expectResult = [];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   commDropCS(db, csName, true, "drop cs in ending");
}
main();


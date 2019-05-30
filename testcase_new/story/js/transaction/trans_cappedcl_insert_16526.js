/************************************
*@Description: 开启事务，执行插入操作并回滚事务 
*@author:      liuxiaoxuan
*@createdate:  2018.11.08
*@testlinkCase: seqDB-16526
**************************************/

function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   // create capped cs
   var csName = COMMCSNAME + "_capped_16526";
   commDropCS(db, csName, true, "drop cs in beginning");
   commCreateCS(db, csName, true, "", {Capped: true});

   // create capped cl
   var clName = COMMCLNAME + "_capped_16526";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var cappedcl = commCreateCLByOption( db, csName, clName, {Capped: true, Size: 1024, AutoIndexId: false} );

   cappedcl.insert([{a : 0}, {a : 1}]);

   db.transBegin();

   // insert
   cappedcl.insert([{a : 2}, {a : 3}, {a : 4}]);

   // rollback
   db.transRollback();

   // check result
   var expectResult = [{a : 0}, {a : 1}, {a : 2}, {a : 3}, {a : 4}];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   commDropCS(db, csName, true, "drop cs in ending");
}
main();


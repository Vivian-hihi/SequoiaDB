/************************************
*@Description: 开启事务，执行remove操作并回滚事务 
*@author:      liuxiaoxuan
*@createdate:  2019.2.13
*@testlinkCase: seqDB-17842
**************************************/

import( "../fullText/commlib.js" ) ;
function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   // create capped cs
   var csName = COMMCSNAME + "_capped_17842";
   commDropCS(db, csName, true, "drop cs in beginning");
   commCreateCS(db, csName, true, "", {Capped: true});

   // create capped cl
   var clName = COMMCLNAME + "_capped_17842";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var cappedcl = commCreateCLByOption( db, csName, clName, {Capped: true, Size: 1, AutoIndexId: false} );

   cappedcl.insert([{a : 0}, {a : 1}, {a : 2}, {a : 3}, {a : 4}]);

   db.transBegin();

   // remove
   cappedcl.remove({a : 4});

   // rollback
   db.transRollback();

   // check result
   var expectResult = [{a : 0}, {a : 1}, {a : 2}, {a : 3}];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   commDropCS(db, csName, true, "drop cs in ending");
}
main();


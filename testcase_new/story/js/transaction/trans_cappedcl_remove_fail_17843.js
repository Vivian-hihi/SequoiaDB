/************************************
*@Description: remove失败的事务验证 
*@author:      liuxiaoxuan
*@createdate:  2019.2.13
*@testlinkCase: seqDB-17843
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   // create capped cs
   var csName = COMMCSNAME + "_capped_17843";
   commDropCS(db, csName, true, "drop cs in beginning");
   commCreateCS(db, csName, true, "", {Capped: true});

   // create capped cl
   var clName = COMMCLNAME + "_capped_17843";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var cappedcl = commCreateCLByOption( db, csName, clName, {Capped: true, Size: 1, Max: 50, AutoIndexId: false} );

   // insert records
   cappedcl.insert([{a : 0}, {a : 1}, {a : 2}, {a : 3}, {a : 4}]);

   // remove fail, commit
   db.transBegin();
   try
   {
      cappedcl.remove();
      throw "remove should be failure!";
   }
   catch(e)
   {
      if(-6 != e)
      {
         throw buildException("remove()", "remove", "remove", -6, e);
      }
   }
   db.transCommit();
 
   // check result
   var expectResult = [{a : 0}, {a : 1}, {a : 2}, {a : 3}, {a : 4}];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   // remove fail, rollback
   db.transBegin();
   try
   {
      cappedcl.remove();
      throw "remove should be failure!";
   }
   catch(e)
   {
      if(-6 != e)
      {
         throw buildException("remove()", "remove", "remove", -6, e);
      }
   }
   db.transRollback();

    // check result
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   commDropCS(db, csName, true, "drop cs in ending");
}
main();


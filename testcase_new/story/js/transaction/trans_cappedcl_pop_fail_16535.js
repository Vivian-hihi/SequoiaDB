/************************************
*@Description: pop失败的事务验证
*@author:      liuxiaoxuan
*@createdate:  2018.11.10
*@testlinkCase: seqDB-16535
**************************************/

function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   // create capped cs
   var csName = COMMCSNAME + "_capped_16535";
   commDropCS(db, csName, true, "drop cs in beginning");
   commCreateCS(db, csName, true, "", {Capped: true});

   // create capped cl
   var clName = COMMCLNAME + "_capped_16535";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var cappedcl = commCreateCLByOption( db, csName, clName, {Capped: true, Size: 1, Max: 100, AutoIndexId: false} );

   // insert records
   cappedcl.insert([{a : 0}, {a : 1}, {a : 2}, {a : 3}]);

   // pop fail with not exist record, commit
   db.transBegin();
   try 
   {   
      // pop success
      cappedcl.pop({LogicalID : 168, Direction : -1});
      println("pop success in transaction");
      // pop fail  
      cappedcl.pop({LogicalID : 10000, Direction: -1});
      throw "pop should be failure!";
   }   
   catch(e)
   {   
      if(-6 != e)
      {   
         throw buildException("pop", "pop", "pop", -6, e); 
      }   
   }   
   db.transCommit();

   // check result
   var expectResult = [{a : 0}, {a : 1}, {a : 2}];
   var actResult = new DBOperator().findFromCL(cappedcl, {}, {a : {$include : 1}}, {a : 1});
   checkResult(expectResult, actResult);

   commDropCS(db, csName, true, "drop cs in ending");
}
main();


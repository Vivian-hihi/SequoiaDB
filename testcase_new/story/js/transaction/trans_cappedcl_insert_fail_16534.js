/************************************
*@Description: 插入失败的事务验证 
*@author:      liuxiaoxuan
*@createdate:  2018.11.10
*@testlinkCase: seqDB-16534
**************************************/
function main()
{
   if(commIsStandalone(db))  {   return ;   }  

   // create capped cs
   var csName = COMMCSNAME + "_capped_16534";
   commDropCS(db, csName, true, "drop cs in beginning");
   commCreateCS(db, csName, true, "", {Capped: true});

   // create capped cl
   var clName = COMMCLNAME + "_capped_16534";
   commDropCL(db, COMMCSNAME, clName, true, true);

   var cappedcl = commCreateCLByOption( db, csName, clName, {Capped: true, Size: 1, Max: 50, AutoIndexId: false} );

   // insert records
   var str = "";
   for(var i = 0; i < 1024; i++)
   {
     str += 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa';
   }

   var objs = new Array();
   for(var i = 0; i < 31 ; i++)
   {
      objs.push({a : str});
   }
   cappedcl.insert(objs);

   // insert fail up to size, commit
   db.transBegin();
   try
   {
      // insert success
      cappedcl.insert({a : 'test1'});
      println("insert success in transaction");
      // insert fail
      cappedcl.insert(objs);
      throw "insert should be failure!";
   }
   catch(e)
   {
      if(-307 != e)
      {
         throw buildException("insert data", "insert", "insert", -307, e);
      }
   }
   db.transCommit();
 
   // check result
   checkCount(32, cappedcl.count());

   // insert fail up to max, commit
   var maxObjs = new Array();
   for(var i = 0; i < 18; i++)
   {
      maxObjs.push({a : 'a'});
   }
  
   db.transBegin();
   try
   {
      // insert success
      cappedcl.insert(maxObjs);
      println("insert success in transaction");
      // insert fail
      cappedcl.insert({a : 'test2'});
      throw "insert should be failure!";
   }
   catch(e)
   {
      if(-307 != e)
      {
         throw buildException("insert data", "insert", "insert", -307, e);
      }
   }
   db.transCommit();

   // check result
   checkCount(50, cappedcl.count());

   cappedcl.truncate();
   cappedcl.insert(objs);
   
   // insert fail up to size, rollback
   db.transBegin();
   try
   {
      // insert success
      cappedcl.insert({a : 'test3'});
      println("insert success in transaction");
      // insert fail
      cappedcl.insert(objs);
      throw "insert should be failure!";
   }
   catch(e)
   {
      if(-307 != e)
      {
         throw buildException("insert data", "insert", "insert", -307, e);
      }
   }
   db.transRollback();

   // check result
   checkCount(32, cappedcl.count());

   // insert fail up to max, rollback
   try
   {
      // insert success
      cappedcl.insert(maxObjs);
      println("insert success in transaction");
      // insert fail
      cappedcl.insert({a : 'test4'});
      throw "insert should be failure!";
   }
   catch(e)
   {
      if(-307 != e)
      {
         throw buildException("insert data", "insert", "insert", -307, e);
      }
   }
   db.transRollback();

   // check result
   checkCount(50, cappedcl.count());

   commDropCS(db, csName, true, "drop cs in ending");
}
function checkCount( expectCount, actCount )
{
   if(expectCount != actCount)
   {
      throw buildException("checkCount()", "check count", "check count", expectCount, actCount);
   }
   println("check result success!");
}
main();


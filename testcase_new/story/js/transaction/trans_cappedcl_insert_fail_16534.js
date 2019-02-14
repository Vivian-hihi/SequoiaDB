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

   // insert fail up to max, commit
   var objs = new Array();
   for(var i = 0; i < 50; i++)
   {
      objs.push({a : 'a'});
   }
  
   db.transBegin();
   try
   {
      // insert success
      cappedcl.insert(objs);
      println("insert success in transaction");
      // insert fail
      cappedcl.insert({a : 'test'});
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


/************************************
*@Description: seqDB-19179:TransTimeout属性支持会话级别 
*@author:      zhaoyu
*@createdate:  2019.08.24
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_19179";
   var record = {a:1};
   var transTimeoutSet = 10;
   commDropCL( db, COMMCSNAME, clName);
   commCreateCL( db, COMMCSNAME, clName);
   
   var db1 = new Sdb(db);
      var cl1 = db1.getCS(COMMCSNAME).getCL(clName);
      db1.transBegin();
      cl1.insert(record);
      
      var db2 = new Sdb(db);
      
      var db3 = new Sdb(db);
      db3.setSessionAttr({TransTimeout: transTimeoutSet});
      var transTimeout = db3.getSessionAttr().toObj().TransTimeout;
      if(transTimeout !== transTimeoutSet)
      {
         println("actual transTimeout:" + transTimeout + ",expect transTimeout: " + transTimeoutSet);
         throw "transTimeout_err";
      }
      db3.transBegin();
      var cl3 = db3.getCS(COMMCSNAME).getCL(clName);
      var date1 = new Date();
      updateTimeOut( cl3 )
      var date2 = new Date();
      if((date2 - date1) > 15*1000)
      {
         println("actual transTimeout begin trans:" + (date2 - date1) + ",expect transTimeout: " + transTimeoutSet);
         throw "transTimeout_err";
      }
      
      
      db2.transBegin();
      var cl2 = db2.getCS(COMMCSNAME).getCL(clName);
      date1 = new Date();
      updateTimeOut( cl2 )
      date2 = new Date();
      if((date2 - date1) > 65*1000)
      {
         println("actual transTimeout begin trans:" + (date2 - date1) + ",expect transTimeout: " + transTimeoutSet);
         throw "transTimeout_err";
      }
      
      var db4 = new Sdb(db);
      db4.transBegin();
      var cl4 = db4.getCS(COMMCSNAME).getCL(clName);
      date1 = new Date();
      updateTimeOut( cl4 )
      date2 = new Date();
      if((date2 - date1) > 65*1000)
      {
         println("actual transTimeout begin trans:" + (date2 - date1) + ",expect transTimeout: " + transTimeoutSet);
         throw "transTimeout_err";
      }
      
      db1.transCommit();
      db2.transCommit();
      db3.transCommit();
      db4.transCommit();
      
      commDropCL( db, COMMCSNAME, clName);
      
      db1.close();
      db2.close();
      db3.close();
      db4.close();
}
main();

function updateTimeOut( cl )
{
   try
      {
         cl.update({$set:{a:2}});
      }catch(e)
      {
         if(e !==-13)
         {
            throw e;
         }
      }
}
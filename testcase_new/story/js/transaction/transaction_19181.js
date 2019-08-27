/************************************
*@Description: seqDB-19181:TransLockWait属性支持会话级别
*@author:      zhaoyu
*@createdate:  2019.08.26
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_19181";
   var record = {a:1};
   var transLockWaitSet = true;
   commDropCL( db, COMMCSNAME, clName);
   commCreateCL( db, COMMCSNAME, clName);
   
   var db1 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
   var cl1 = db1.getCS(COMMCSNAME).getCL(clName);
   db1.transBegin();
   cl1.insert(record);
   
   var db2 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
   
   var db3 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
   db3.setSessionAttr({TransIsolation:1, TransLockWait: transLockWaitSet});
   var transLockWait = db3.getSessionAttr().toObj().TransLockWait;
   if(transLockWait !== transLockWaitSet)
   {
      println("actual transLockWait:" + transLockWait + ",expect transLockWait: " + transLockWaitSet);
      throw "transLockWait_err";
   }
   db3.transBegin();
   var cl3 = db3.getCS(COMMCSNAME).getCL(clName);
   try
   {
      cl3.find().next();
      throw "need_err";
   }catch(e)
   {
      if(e !==-13)
      {
         throw e;
      }
      
   }
   
   db2.transBegin();
   var cl2 = db2.getCS(COMMCSNAME).getCL(clName);
   cursor = cl2.find();
   checkRec( cursor, [record] );
   
   var db4 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
   db4.transBegin();
   var cl4 = db4.getCS(COMMCSNAME).getCL(clName);
   cursor = cl4.find();
   checkRec( cursor, [record] );
   
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
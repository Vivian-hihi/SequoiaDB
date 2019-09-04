/************************************
*@Description: seqDB-19180:TransUseRBS属性支持会话级别
*@author:      zhaoyu
*@createdate:  2019.08.26
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_19180";
   var record1 = {a:1};
   var record2 = {a:2};
   var transUseRBSSet = false;
   commDropCL( db, COMMCSNAME, clName);
   commCreateCL( db, COMMCSNAME, clName);
   
   var db1 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
   var cl1 = db1.getCS(COMMCSNAME).getCL(clName);
   cl1.insert(record1);
   db1.setSessionAttr({TransIsolation:1, TransUseRBS: transUseRBSSet});
   var transUseRBS = db1.getSessionAttr().toObj().TransUseRBS;
   if(transUseRBS !== transUseRBSSet)
   {
      throw new Error("actual transUseRBS:" + transUseRBS + ",expect transUseRBS: " + transUseRBSSet);
   }
   db1.transBegin();
   var cl1 = db1.getCS(COMMCSNAME).getCL(clName);
   cl1.update({$set:record2});
   
   var db2 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
   db2.setSessionAttr({TransIsolation:1, TransUseRBS: transUseRBSSet});
   var transUseRBS = db2.getSessionAttr().toObj().TransUseRBS;
   if(transUseRBS !== transUseRBSSet)
   {
      throw new Error("actual transUseRBS:" + transUseRBS + ",expect transUseRBS: " + transUseRBSSet);
   }
   db2.transBegin();
   var cl2 = db2.getCS(COMMCSNAME).getCL(clName);
   
   try
   {
      cl2.find().next();
      throw new Error("need_err");
   }catch(e)
   {
      if(e !==-13)
      {
         throw new Error(e);
      }
      
   }
   
   db1.transCommit();
   db2.transCommit();
   
   commDropCL( db, COMMCSNAME, clName);
   
   db1.close();
   db2.close();
}
try
{
   main();
}
catch(e)
{
   if ( e.constructor === Error )
   {
      println(e.stack) ;  
   }
   throw e ;
}
;
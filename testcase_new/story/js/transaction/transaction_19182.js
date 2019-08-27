/************************************
*@Description: seqDB-19182:TransAutoCommit属性支持会话级别 
*@author:      zhaoyu
*@createdate:  2019.08.26
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_19182";
   var transAutoCommitSet = true;
   commDropCL( db, COMMCSNAME, clName);
   commCreateCL( db, COMMCSNAME, clName);
   
   var db3 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
   db3.setSessionAttr({TransAutoCommit:transAutoCommitSet});
   var transAutoCommit = db3.getSessionAttr().toObj().TransAutoCommit;
   if(transAutoCommit !== transAutoCommitSet)
   {
      println("actual transAutoCommit:" + transAutoCommit + ",expect transAutoCommit: " + transAutoCommitSet);
      throw "transAutoCommit_err";
   }
   
   commDropCL( db, COMMCSNAME, clName);
   
   db3.close();
}
main();
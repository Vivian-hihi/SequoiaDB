/************************************
*@Description: seqDB-19191:coord及数据节点均开启事务，TransLockWait属性不一致
*@author:      zhaoyu
*@createdate:  2019.08.27
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_19191";
   var records = [];
   for(var i=0;i<100;i++)
   {
      records.push({a:1});
   }
   
   commDropCL( db, COMMCSNAME, clName);
   var optionObj = {ShardingKey:{_id:1},AutoSplit:true};
   commCreateCLByOption( db, COMMCSNAME, clName, optionObj);
   
   try
   {
      db.updateConf({transisolation:1, translockwait:true},{Global:false});
   
      var db1 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
      var cl1 = db1.getCS(COMMCSNAME).getCL(clName);
      db1.transBegin();
      cl1.insert(records);
      
      var db2 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
      db2.transBegin();
      try
      {
         db2.getCS(COMMCSNAME).getCL(clName).find().next();
         throw "need_err";
      }catch(e)
      {
         if(e !== -13)
         {
            println("actual err:" + e + ",expect err:" + "-13");
            throw e;
         } 
      }
      
      db1.transCommit();
      db2.transCommit();
      commDropCL( db, COMMCSNAME, clName);
      
      db1.close();
      db2.close();
   }finally
   {
      db.deleteConf({transisolation:"", translockwait:""});
   }
   
}
main();
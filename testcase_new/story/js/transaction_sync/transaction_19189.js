/************************************
*@Description: seqDB-19189:coord及数据节点均开启事务，TransTimeout属性不一致 
*@author:      zhaoyu
*@createdate:  2019.08.27
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_19189";
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
      db.updateConf({transactiontimeout:10},{Global:false});
   
      var db1 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
      db1.transBegin();
      db1.getCS(COMMCSNAME).getCL(clName).insert(records);
      
      var db2 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
      db2.transBegin();
      var date1 = new Date();
      try
      {
         db2.getCS(COMMCSNAME).getCL(clName).update({$set:{a:2}});
         throw "need_err";
      }catch(e)
      {
         if(e !== -13)
         {
            throw e;
         } 
      }
      var date2 = new Date();
      if((date2 - date1) > 15*1000)
      {
         println("actual transTimeout begin trans:" + (date2 - date1) + ",expect transTimeout: " + transTimeoutSet);
         throw "transTimeout_err";
      }
      
      db1.transCommit();
      var cursor = db1.getCS(COMMCSNAME).getCL(clName).find();
      checkRec( cursor, records );
      
      db2.transCommit();
      
      commDropCL( db, COMMCSNAME, clName);
      
      db1.close();
      db2.close();
   }finally
   {
      db.deleteConf({transactiontimeout:""});
   }
   
}
main();
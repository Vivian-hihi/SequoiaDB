/************************************
*@Description: seqDB-19194:coord及数据节点均开启事务，TransRCCount属性不一致 
*@author:      zhaoyu
*@createdate:  2019.08.27
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_19194";
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
      db.updateConf({transisolation:1},{Global:false});
      db.updateConf({transrccount:false},{Role:"data"});
   
      var db1 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
      var cl1 = db1.getCS(COMMCSNAME).getCL(clName);
      db1.transBegin();
      cl1.insert(records);
      
      var db2 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
      db2.transBegin();
      recordNum= db2.getCS(COMMCSNAME).getCL(clName).count();
      if(parseInt(recordNum) !== 0)
      {
         println("actual recordNum:" + parseInt(recordNum) + ",expect recordNum:" + 0);
         throw "count_err";
      }
      
      db1.transCommit();
      db2.transCommit();
      commDropCL( db, COMMCSNAME, clName);
   }finally
   {
      db.deleteConf({transisolation:"", transrccount:""});
      db1.close();
      db2.close();
   }
   
}
main();
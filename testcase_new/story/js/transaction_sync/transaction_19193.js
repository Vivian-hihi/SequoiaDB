/************************************
*@Description: seqDB-19193:coord及数据节点均开启事务，TransAutoRollback属性不一致 
*@author:      zhaoyu
*@createdate:  2019.08.27
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_19193";
   var record1 = {a:1};
   
   commDropCL( db, COMMCSNAME, clName);
   commCreateCL( db, COMMCSNAME, clName );
   
   
   try
   {
      db.updateConf({transautorollback:false},{Global:false});
   
      var db1 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
      var cl1 = db1.getCS(COMMCSNAME).getCL(clName);
      cl1.createIndex("a", {a:1}, true, true);
      cl1.insert(record1);
      db1.transBegin();
      try
      {
         cl1.insert(record1);
         throw "need_err";
      }catch(e)
      {
         if(e !==-38)
         {
            throw e;
         }
      }
      var record2 = {a:2};
      cl1.insert(record2);
      db1.transRollback();
      
      var db2 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
      var cursor = db2.getCS(COMMCSNAME).getCL(clName).find();
      checkRec( cursor, [record1] );
      
      db1.transCommit();
      db2.transCommit();
      
      commDropCL( db, COMMCSNAME, clName);
     
      db1.close();
      db2.close();
   }finally
   {
      db.deleteConf({transautorollback:""});
   }
}
main();
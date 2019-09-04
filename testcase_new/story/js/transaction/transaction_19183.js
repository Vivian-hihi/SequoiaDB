/************************************
*@Description: seqDB-19183:TransAutoRollback属性支持会话级别
*@author:      zhaoyu
*@createdate:  2019.08.26
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_19183";
   var record1 = {a:1};
   var record2 = {a:2};
   var record3 = {a:3};
   var transAutoRollbackSet = false;
   commDropCL( db, COMMCSNAME, clName);
   commCreateCL( db, COMMCSNAME, clName);
   
   var db1 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
   var cl1 = db1.getCS(COMMCSNAME).getCL(clName);
   cl1.createIndex("a", {a:1}, true, true);
   cl1.insert(record1);
   
   var db2 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
   db2.setSessionAttr({TransAutoRollback:transAutoRollbackSet});
   var transAutoRollback = db2.getSessionAttr().toObj().TransAutoRollback;
   if(transAutoRollback !== transAutoRollbackSet)
   {
      throw new Error("actual transAutoRollback:" + transAutoRollback + ",expect transAutoRollback: " + transAutoRollbackSet);
   }
   db2.transBegin();
   var cl2 = db2.getCS(COMMCSNAME).getCL(clName);
   insertRecordErr( cl2, record1 );
   
   cl2.insert(record2);
   db2.transRollback();
   var cursor = cl2.find();
   checkRec( cursor, [record1] );
   
   db1.transBegin();
   insertRecordErr( cl1, record1 );
   cl1.insert(record2);
   db1.transRollback();
   var cursor = cl1.find();
   checkRec( cursor, [record1, record2] );
   
   var db3 = new Sdb(COORDHOSTNAME, COORDSVCNAME);
   var cl3 = db3.getCS(COMMCSNAME).getCL(clName);
   db3.transBegin();
   insertRecordErr( cl3, record1 );
   cl3.insert(record3);
   db3.transRollback();
   var cursor = cl3.find();
   checkRec( cursor, [record1, record2, record3] );
   
   commDropCL( db, COMMCSNAME, clName);
   
   db1.close();
   db2.close();
   db3.close();
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

function insertRecordErr( cl, record )
{
   try
   {
      cl.insert(record);
      throw new Error("need_err");
   }catch(e)
   {
      if(e !== -38)
      {
         throw new Error(e);
      }
   }
}

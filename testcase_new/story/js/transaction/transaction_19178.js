/************************************
*@Description: seqDB-19178:TransIsolation属性支持会话级别
*@author:      zhaoyu
*@createdate:  2019.08.24
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_19178";
   var records = [];
   for(var i=0;i<100;i++)
   {
      records.push({a:1});
   }
   var transIsolationSet = 1;
   commDropCL( db, COMMCSNAME, clName);
   var optionObj = {ShardingKey:{_id:1},AutoSplit:true};
   commCreateCLByOption( db, COMMCSNAME, clName, optionObj);
   
   var db1 = new Sdb(db);
   var cl1 = db1.getCS(COMMCSNAME).getCL(clName);
   db1.transBegin();
   cl1.insert(records);
   
   var db2 = new Sdb(db);
   
   var db3 = new Sdb(db);
   db3.setSessionAttr({TransIsolation: transIsolationSet});
   var transIsolation = db3.getSessionAttr().toObj().TransIsolation;
   if(transIsolation !== transIsolationSet)
   {
      throw new Error("actual transIsolation:" + transIsolation + ",expect transIsolation: " + transIsolationSet);
   }
   db3.transBegin();
   var cl3 = db3.getCS(COMMCSNAME).getCL(clName);
   var cursor = cl3.find();
   checkRec( cursor, [] );
   
   db2.transBegin();
   var cl2 = db2.getCS(COMMCSNAME).getCL(clName);
   cursor = cl2.find();
   checkRec( cursor, records );
   
   var db4 = new Sdb(db);
   db4.transBegin();
   var cl4 = db4.getCS(COMMCSNAME).getCL(clName);
   cursor = cl4.find();
   checkRec( cursor, records );
   
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
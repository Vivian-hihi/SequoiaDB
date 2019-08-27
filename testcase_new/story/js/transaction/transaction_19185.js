/************************************
*@Description: seqDB-19185:事务中设置事务属性
*@author:      zhaoyu
*@createdate:  2019.08.24
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   var clName = COMMCLNAME + "_19185";
   var record = {a:1};
   commDropCL( db, COMMCSNAME, clName);
   commCreateCL( db, COMMCSNAME, clName);
   
   var db1 = new Sdb(db);
   db1.transBegin();
   var cl1 = db1.getCS(COMMCSNAME).getCL(clName);
   cl1.insert(record);
   db1.setSessionAttr({TransTimeout: 120});
   setSessionAttr(db1, {TransIsolation:1});
   setSessionAttr(db1, {TransUseRBS:false});
   setSessionAttr(db1, {TransLockWait:true});
   setSessionAttr(db1, {TransAutoCommit:true});
   setSessionAttr(db1, {TransAutoRollback:false});
   setSessionAttr(db1, {TransRCCount:false}); 
   var expectOption = {TransTimeout:120, TransIsolation:0, TransUseRBS:true, TransLockWait:false, TransAutoCommit:false, TransAutoRollback:true, TransRCCount: true};
   var sessionOption = db1.getSessionAttr().toObj();
   for(key in sessionOption)
   {
      if(key == "TransIsolation" || key == "TransTimeout" || key == "TransUseRBS" || key == "TransLockWait" || key == "TransAutoCommit" || key == "TransAutoRollback" || key == "TransRCCount")
      {
         if(expectOption[key] !== sessionOption[key])
         {
            println("expect:" + key + ":" + expectOption[key] + ",actual:" + key + ":" + sessionOption[key]);
            throw "transaction_option_err";
         }
      }
      
   }
   db1.transCommit();
   
   commDropCL( db, COMMCSNAME, clName);
   
   db1.close();
}
main();

function setSessionAttr(db1, option)
{
   try
   {
      db1.setSessionAttr(option);
      throw "need_err";
   }catch(e)
   {
      if(e !== -6)
      {
         println("actual err:" + e + ",expect err:" + "-6");
         throw e;
      }  
   }
   
}
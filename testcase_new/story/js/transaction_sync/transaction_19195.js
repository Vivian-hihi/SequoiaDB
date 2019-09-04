/************************************
*@Description: seqDB-19195:修改事务配置不影响使用setSessionAtrr()设置过的会话
*@author:      zhaoyu
*@createdate:  2019.08.27
**************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   }
   
   try
   {
      var sessionAttr = {TransIsolation:1, TransTimeout:30, TransLockWait:true, TransAutoCommit:true, TransAutoRollback:false, TransUseRBS:false, TransRCCount:false};
      db.setSessionAttr(sessionAttr);
      
      var config = {transisolation:2, transactiontimeout:120, translockwait:false, transautocommit:false, transautorollback:true, transuserbs:true, transrccount:true};
      db.updateConf(config)
      var actSessionAttr = db.getSessionAttr().toObj();
      for(key in actSessionAttr)
      {
         if(key == "TransIsolation" || key == "TransTimeout" || key == "TransUseRBS" || key == "TransLockWait" || key == "TransAutoCommit" || key == "TransAutoRollback" || key == "TransRCCount")
         {
            if(sessionAttr[key] !== actSessionAttr[key])
            {
               throw new Error("expect:" + key + ":" + sessionAttr[key] + ",actual:" + key + ":" + actSessionAttr[key]);
            }
         }
         
      }
     
   }finally
   {
      db.deleteConf({transisolation:"", transactiontimeout:"", translockwait:"", transautocommit:"", transautorollback:"", transuserbs:"", transrccount:""});
   }
   
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
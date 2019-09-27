/******************************************************************************
*@Description : seqDB-19696:检查list( SDB_LIST_BACKUPS )列表信息
*               TestLink : seqDB-19696
*@author      : wangkexin
*@Date        : 2019-09-27
******************************************************************************/
main(db) ;
function main()
{
   if(commIsStandalone( db )){
      println("------Deploy is standalone");
      return;
   }
   
   var groupsArray = commGetGroups( db, false, "", true, true, true );
   var hostName = groupsArray[0][1].HostName;
   var rgName = "group19696";
   var backupName = "backup19696";
   
   try
   {
      println("begin to create data group: " + rgName);
      var srcLogPath = createDataGroup( rgName, hostName );

      //backup group19696 data 
      db.backup( { Name: backupName,GroupName:rgName} );
      
      //check list result
      var fields = ["Version", "Name", "ID", "NodeName", "GroupName", "EnsureInc", "BeginLSNOffset", "EndLSNOffset", "TransLSNOffset", "StartTime", "LastLSN", "LastLSNCode", "HasError"];
      var cur = db.list( SDB_LIST_BACKUPS,{"Name" : backupName} );
      while( cur.next() )
      {
         var ret = cur.current().toObj();
         for(var index in fields)
         {
            var field = fields[index];
            if (ret[field] == null)
            {
               println(field + ": " + ret[field]);
            throw buildException("compare field", "", "list( SDB_LIST_BACKUPS, options )", "not null", "null");
            }
         }
      }
   }
   catch ( e )
   {
      println("catch e : " + e);
      //将新建组日志备份到/tmp/ci/rsrvnodelog目录下
      var backupDir = "/tmp/ci/rsrvnodelog/19696";
      File.mkdir(backupDir);
      File.scp( srcLogPath, backupDir + "/sdbdiag.log" );
      throw e;
   }
   finally
   {
      db.removeRG( rgName );
   }
}

function createDataGroup( rgName, hostName )
{
   var dataRG = db.createRG( rgName );
   var srcLogPath = "";
   
   var port = parseInt( RSRVPORTBEGIN )+10;
   var dataPath = RSRVNODEDIR+"data/"+port;
   var checkSucc = false;
   var times = 0;
   var maxRetryTimes = 10;
   do
   {
      try
      {
         dataRG.createNode( hostName, port, dataPath, {diaglevel:5});
         checkSucc = true;
      }
      catch( e )
      {
         //-145 :SDBCM_NODE_EXISTED  -290:SDB_DIR_NOT_EMPTY
         if( e == -145 || e == -290 )
         {
            port = port + 10;
            dataPath = RSRVNODEDIR+"data/"+port;
         }
         else
         {
            throw "create node failed!  port = " + port + " dataPath = " + dataPath + " errorCode: " + e;
         }
         times++;
      }
   }
   while(!checkSucc && times < maxRetryTimes);
   dataRG.start();
   srcLogPath = hostName+":"+CMSVCNAME+"@"+dataPath+"/diaglog/sdbdiag.log";
   return srcLogPath;
}
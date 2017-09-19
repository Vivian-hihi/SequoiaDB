/*******************************************************************************

   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*******************************************************************************/
/*
@description: deploy package
@modify list:
   2017-09-12 JiaWen He  Init

1. Generate plan
   @parameter
      var SYS_STEP = "Generate plan" ;
      var BUS_JSON = {"_id":{"$oid":"59bb48afbe1f79df0fa98778"},"TaskID":8,"Type":7,"TypeDesc":"DEPLOY_PACKAGE","TaskName":"DEPLOY_PACKAGE","CreateTime":{"$timestamp":"2017-09-15-11.27.43.000000"},"EndTime":{"$timestamp":"2017-09-15-11.27.43.000000"},"Status":0,"StatusDesc":"INIT","AgentHost":"ubuntu-jw-02","AgentService":"11790","Info":{"ClusterName":"myCluster1","SdbUser":"sdbadmin","SdbPasswd":"sdbadmin","SdbUserGroup":"sdbadmin_group","InstallPacket":"/opt/sequoiadb/packet/sequoiasql-oltp-2.8.2-x86_64-enterprise-installer.run","PackageName":"sequoiasql-oltp","InstallPath": "/opt/sequoiasqloltp/","Enforced":true,"HostInfo":[{"HostName":"ubuntu-jw-02","IP":"192.168.3.232","AgentService":"11790","SshPort":"22","User":"root","Passwd":"123"}]},"errno":0,"detail":"","Progress":0,"ResultInfo":[{"HostName":"ubuntu-jw-01","Status":0,"StatusDesc":"INIT","errno":0,"detail":"","Flow":[]}]};
   @return
      RET_JSON: the format is: {"Plan":[[{"cmd":"install","TaskID":8,"Info":{"PackageName":"sequoiasql-oltp","InstallPath":"/opt/sequoiasqloltp/","InstallPacket":"/opt/sequoiadb/packet/sequoiasql-oltp-2.8.2-x86_64-enterprise-installer.run","SdbUser":"sdbadmin","SdbPasswd":"sdbadmin","SdbUserGroup":"sdbadmin_group","HostInfo":{"HostName":"ubuntu-jw-01","IP":"192.168.3.231","AgentService":"11790","SshPort":"22","User":"root","Passwd":"123"}},"ResultInfo":{"HostName":"ubuntu-jw-01","Status":0,"StatusDesc":"INIT","errno":0,"detail":"","Flow":[],"Progress":90}}]]}

2. Install package
   @parameter
      var SYS_STEP = "Doit" ;
      var BUS_JSON = {"cmd":"install","TaskID":8,"Info":{"PackageName":"sequoiasql-oltp","InstallPath":"/opt/sequoiasqloltp/","InstallPacket":"/opt/sequoiadb/packet/sequoiasql-oltp-2.8.2-x86_64-enterprise-installer.run","SdbUser":"sdbadmin","SdbPasswd":"sdbadmin","SdbUserGroup":"sdbadmin_group","HostInfo":{"HostName":"ubuntu-jw-02","IP":"192.168.3.232","AgentService":"11790","SshPort":"22","User":"root","Passwd":"123"}},"ResultInfo":{"HostName":"ubuntu-jw-01","Status":0,"StatusDesc":"INIT","errno":0,"detail":"","Flow":[],"Progress":90}};
   @return
      RET_JSON: the format is: {"HostName":"ubuntu-jw-02","Status":4,"StatusDesc":"FINISH","errno":0,"detail":"","Flow":[],"Progress":90}

3. Check result
   @parameter
      var SYS_STEP = "Check result" ;
      var BUS_JSON = {"errno":0,"detail":"","Progress":15,"ResultInfo":[{"HostName":"ubuntu-jw-02","datagroupname":"","svcname":"11840","role":"coord","Status":0,"StatusDesc":"INIT","errno":0,"detail":"","Flow":["Installing coord[ubuntu-jw-02:11840]","Successfully create coord[ubuntu-jw-02:11840]"]},{"HostName":"ubuntu-jw-01","datagroupname":"","svcname":"11840","role":"catalog","Status":0,"StatusDesc":"INIT","errno":0,"detail":"","Flow":[]},{"HostName":"ubuntu-jw-01","datagroupname":"group1","svcname":"11850","role":"data","Status":0,"StatusDesc":"INIT","errno":0,"detail":"","Flow":[]}],"TaskID":25,"Info":{"Config":[{"HostName":"ubuntu-jw-02","datagroupname":"","dbpath":"/opt/sequoiadb/database/coord/11840","svcname":"11840","role":"coord","diaglevel":"3","logfilesz":"64","logfilenum":"20","transactionon":"false","preferedinstance":"A","numpreload":"0","maxprefpool":"200","maxreplsync":"10","logbuffsize":"1024","sortbuf":"512","hjbuf":"128","syncstrategy":"keepnormal","weight":"10","maxsyncjob":"10","syncinterval":"10000","syncrecordnum":"0","syncdeep":"false","archiveon":"false","archivecompresson":"true","archivepath":"","archivetimeout":"600","archiveexpired":"240","archivequota":"10","indexpath":"","bkuppath":"","lobpath":"","lobmetapath":""},{"HostName":"ubuntu-jw-01","datagroupname":"","dbpath":"/opt/sequoiadb/database/catalog/11840","svcname":"11840","role":"catalog","diaglevel":"3","logfilesz":"64","logfilenum":"20","transactionon":"false","preferedinstance":"A","numpreload":"0","maxprefpool":"200","maxreplsync":"10","logbuffsize":"1024","sortbuf":"512","hjbuf":"128","syncstrategy":"keepnormal","weight":"10","maxsyncjob":"10","syncinterval":"10000","syncrecordnum":"0","syncdeep":"false","archiveon":"false","archivecompresson":"true","archivepath":"","archivetimeout":"600","archiveexpired":"240","archivequota":"10","indexpath":"","bkuppath":"","lobpath":"","lobmetapath":""},{"HostName":"ubuntu-jw-01","datagroupname":"group1","dbpath":"/opt/sequoiadb/database/data/11850","svcname":"11850","role":"data","diaglevel":"3","logfilesz":"64","logfilenum":"20","transactionon":"false","preferedinstance":"A","numpreload":"0","maxprefpool":"200","maxreplsync":"10","logbuffsize":"1024","sortbuf":"512","hjbuf":"128","syncstrategy":"keepnormal","weight":"10","maxsyncjob":"10","syncinterval":"10000","syncrecordnum":"0","syncdeep":"false","archiveon":"false","archivecompresson":"true","archivepath":"","archivetimeout":"600","archiveexpired":"240","archivequota":"10","indexpath":"","bkuppath":"","lobpath":"","lobmetapath":""}],"Coord":[{"HostName":"ubuntu-jw-01","svcname":"11810"}],"User":"","Passwd":"","ClusterName":"myCluster1","BusinessType":"sequoiadb","BusinessName":"myModule1","DeployMod":"vertical"}} ;
   @return
      RET_JSON: the format is: {"errno":0,"detail":""}
*/

function main()
{
   var PD_LOGGER = new Logger( "deployPackage.js" ) ;
   var taskID = 0 ;
   var result = {} ;

   PD_LOGGER.logComm( PDEVENT, sprintf( "Begin to deploy package, Step [?]",
                                        SYS_STEP ) ) ;

   var taskInfo = BUS_JSON[FIELD_INFO] ;
   var packageName = taskInfo[FIELD_PACKAGE_NAME] ;

   if ( packageName == FIELD_SEQUOIASQL_OLTP )
   {
      import( '../conf/script/deployPackage/sequoiasql-oltp.js' ) ;
   }
   else
   {
      rc = SDB_SYS ;
      error = new SdbError( rc, sprintf( "Invalid package name [?]",
                                         packageName ) ) ;
      PD_LOGGER.log( PDERROR, error ) ;
      throw error ;
   }

   taskID = BUS_JSON[TaskID] ;

   PD_LOGGER.setTaskId( taskID ) ;

   PD_LOGGER.logTask( PDEVENT, sprintf( "Step [?]", SYS_STEP ) ) ;

   if( SYS_STEP == STEP_GENERATE_PLAN )
   {
      result = GeneratePlan( taskID ) ;
   }
   else if( SYS_STEP == STEP_DOIT )
   {
      if( BUS_JSON[FIELD_CMD] == "install" )
      {
         result = SendPackage( taskID ) ;
         if ( SDB_OK == result[FIELD_ERRNO] )
         {
            result = InstallPackage( taskID ) ;
         }
      }
      else if( BUS_JSON[FIELD_CMD] == "skip" )
      {
         result = Skip() ;
      }
      else if( BUS_JSON[FIELD_CMD] == "error" )
      {
         result = Error() ;
      }
      else
      {
         var error = new SdbError( SDB_INVALIDARG,
                                   sprintf( "Unknow command [?]",
                                            BUS_JSON[FIELD_CMD] ) ) ;
         PD_LOGGER.logTask( PDERROR, error ) ;
         throw error ;
      }
   }
   else if( SYS_STEP == STEP_CHECK_RESULT )
   {
      result = CheckResult( taskID ) ;
   }
   else if( SYS_STEP == STEP_ROLLBACK )
   {
      result = Rollback( taskID ) ;
   }
   else
   {
      var error = new SdbError( SDB_INVALIDARG,
                                sprintf( "Unknow step [?]", SYS_STEP ) ) ;
      PD_LOGGER.logTask( PDERROR, error ) ;
      throw error ;
   }

   return result ;
}

main() ;
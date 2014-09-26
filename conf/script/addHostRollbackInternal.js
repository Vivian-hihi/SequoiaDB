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
@description: remove the installation packet from the host to add
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter
   BUS_JSON: the format is: {"SdbUser":"sdbadmin","SdbPasswd":"sdbadmin","SdbUserGroup":"sdbadmin_group","InstallPacket":"/home/users/tanzhaobo/sequoiadb/bin/../packet/sequoiadb-1.8-linux_x86_64-installer.run","HostInfo":[{"IP":"192.168.20.42","HostName":"susetzb","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"},{"IP":"192.168.20.165","HostName":"rhel64-test8","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"},{"IP":"192.168.20.166","HostName":"rhel64-test9","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"}]}
   SYS_JSON: {}
   ENV_JSON: {}
   OTHER_JSON: {}
@return
   RET_JSON: the format is: {"HostInfo":[{"Rc":0,"detail":"","IP":"192.168.20.42","HasUninstall":true},{"Rc":0,"detail":"","IP":"192.168.20.165","HasUninstall":true}]}
*/


//var BUS_JSON = { "HostInfo": [ { "IP": "192.168.20.42", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb" }, { "IP": "192.168.20.165", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb" } ] } ;

/* *****************************************************************************
@discretion: uninstall db packet in remote host
@author: Tanzhaobo
@parameter
   ssh[object]: ssh object
   osInfo[string]: os type
   path[string]: the path db installed in
@return void
***************************************************************************** */
function uninstallDBPacket ( ssh, osInfo, path )
{
   var cmd = "" ;
   var path = adaptPath( osInfo, path ) ;
   if ( OMA_LINUX == osInfo )
   {
      cmd = path + OMA_PROG_UNINSTALL_L ;
      try
      {
         ssh.exec( "chmod a+x " + cmd ) ;
         ssh.exec( cmd + " --mode unattended " ) ;
      }
      catch ( e )
      {
         setLastErrMsg( "Failed to uninstall db packet in remote" ) ;
         setLastError( SDB_SYS ) ;
         throw SDB_SYS ;
      }
   }
   else
   {
      // DOTO: tanzhaobo
      // windows
   }
}

function main()
{
   var infoArr = BUS_JSON[HostInfo] ;
   var arrLen = infoArr.length ;
   if ( arrLen == 0 )
   {
      setLastErrMsg( "Not specified any hosts to uninstall" ) ;
      throw SDB_INVALIDARG ;
   }
   // get os infomation
   var osInfo = System.type() ;
   for ( var i = 0; i < arrLen; i++ )
   {
      var ssh          = null ;
      var obj          = infoArr[i]
      var ip           = obj[IP] ;
      var user         = obj[User] ;
      var passwd       = obj[Passwd] ;
      var installPath  = obj[InstallPath] ; 
      var retObj       = new addHostRollbackResult() ;
      retObj[IP]       = ip ;
      try
      {
         // ssh
         var ssh = new Ssh( ip, user, passwd ) ;
         // uninstall business packet from remote host
         uninstallDBPacket( ssh, osInfo, installPath ) ;
         retObj[HasUninstall] = true ;
      }
      catch ( e )
      {
         retObj[Rc] = GETLASTERROR( e, false ) ;
         retObj[Detail] = GETLASTERRMSG() ;
      }
      RET_JSON[Result].push( retObj ) ;
   }
//print("RET_JSON is: " + JSON.stringify(RET_JSON) + "\n") ;
   // return the result
   return RET_JSON ;
}

// execute
   main() ;


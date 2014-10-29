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
@description: uninstall remote sdbcm packet
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter
   BUS_JSON: the info for unindtall remote host: { "HostInfo": [ { "IP": "192.168.20.165", "HostName": "rhel64-test8", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb", "SshPort": "22", "AgentPort": "11790" }, { "IP": "192.168.20.166", "HostName": "rhel64-test9", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb", "SshPort": "22", "AgentPort": "11790" } ] }
   SYS_JSON:
   ENV_JSON:
   OTHER_JSON:
@return
   RET_JSON: the uninstall result:  { "HostInfo": [ { "IP": "192.168.20.165", "errno": 0, "detail": "", "HasUninstall": true }, { "IP": "192.168.20.166", "errno": 0, "detail": "", "HasUninstall": true } ] }
*/

// var BUS_JSON = { "HostInfo": [ { "IP": "192.168.20.165", "HostName": "rhel64-test8", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb", "SshPort": "22", "AgentPort": "11790" }, { "IP": "192.168.20.42", "HostName": "susetzb", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb", "SshPort": "22", "AgentPort": "11790" } ] } ;

var RET_JSON = new Object() ;
RET_JSON[HostInfo] = [] ;

/* *****************************************************************************
@discretion: remove the temp directory and files in remote host
@author: Tanzhaobo
@parameter
   ssh[object]: ssh object
   osInfo[string]: os type
@return void
***************************************************************************** */
function uninstallRemoteTmpPacket( ssh, osInfo )
{
   var cmd = "" ;
   if ( OMA_LINUX == osInfo )
   {
      cmd = "rm -rf " + OMA_PATH_TEMP_OMA_DIR_L2 ;
      try
      {
         ssh.exec( cmd ) ;
      }
      catch ( e )
      {
         setLastErrMsg( "Failed to remove director[" + OMA_PATH_TEMP_OMA_DIR_L + "] in host [" + ssh.getPeerIP() + "]" ) ;
         setLastError( SDB_SYS ) ;
         throw SDB_SYS ;
      }
   }
   else
   {
      // TODO:
   }
}

/* *****************************************************************************
@discretion: stop the temporary sdbcm installed in remote host
@author: Tanzhaobo
@parameter
   ssh[object]: ssh object
   osInfo[string]: os type
@return void
***************************************************************************** */
function stopRemoteSdbcmProgram( ssh, osInfo )
{
   var cmd = "" ;

   if ( OMA_LINUX == osInfo )
   {
      cmd += OMA_PATH_TEMP_BIN_DIR_L ;
      cmd += OMA_PROG_SDBCMTOP_L ;
      cmd += " " + OMA_OPTION_SDBCMART_1 ;
      try
      {
         ssh.exec( cmd ) ;
      }
      catch ( e )
      {
         setLastErrMsg( "Failed to stop sdbcm in host[" + ssh.getPeerIP() + "]" ) ;
         setLastError( SDB_SYS ) ;
         throw SDB_SYS ;
      }
      // check wether sdb is stop in target host
      var times = 0 ;
      for ( ; times < OMA_TRY_TIMES; times++ )
      {
         var isRunning = isSdbcmRunning ( ssh, osInfo ) ;
         if ( isRunning )
         {
            sleep( OMA_SLEEP_TIME ) ;
         }
         else
         {
            break ;
         }
      }
      if ( OMA_TRY_TIMES <= times )
      {
         setLastErrMsg( "Time out, failed to stop sdbcm in host[" + ssh.getPeerIP() + "]" ) ;
         throw e ;
      }
   }
   else
   {
      // TODO: tanzhaobo
   }
}

function main()
{
   var infoArr = BUS_JSON[HostInfo] ;
   var arrLen = infoArr.length ;
   if ( arrLen == 0 )
   {
      setLastErrMsg( "Not specified any hosts to uninstall it's sdbcm" ) ;
      setLastError( SDB_INVALIDARG ) ;
      throw SDB_INVALIDARG ;
   }
   // get os infomation
   var osInfo = System.type() ;
   for ( var i = 0; i < arrLen; i++ )
   {
      var ssh        = null ;
      var obj        = infoArr[i]
      var ip         = obj[IP] ;
      var user       = obj[User] ;
      var passwd     = obj[Passwd] ;
      var retObj     = new uninstallTmpCMResult() ;
      retObj[IP]     = ip ;
      try
      {
         // ssh
         var ssh = new Ssh( ip, user, passwd ) ;
         // check wether it is in localhost,
         // we would not stop local sdbcm
         var flag = isInLocalHost( ssh ) ;
         if ( flag )
         {
            retObj[IsOMStop] = false ;
            RET_JSON[HostInfo].push( retObj ) ;
            continue ;
         }
         // stop remote sdbcm program
         stopRemoteSdbcmProgram( ssh, osInfo ) ;
         retObj[IsOMStop] = true ;
         // remove the packet in remote machine
         uninstallRemoteTmpPacket( ssh, osInfo ) ;
      }
      catch ( e )
      {
         retObj[Errno]  = GETLASTERROR( e, true ) ;
         retObj[Detail] = GETLASTERRMSG() ;
      }
      RET_JSON[HostInfo].push( retObj ) ;
   }

   // return the result
   return RET_JSON ;
}

// execute
   main() ;


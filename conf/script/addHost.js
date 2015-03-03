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
@description: add host to cluster( install db packet and start sdbcm )
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter
   BUS_JSON: the format is: {"SdbUser":"sdbadmin","SdbPasswd":"sdbadmin","SdbUserGroup":"sdbadmin_group","InstallPacket":"/opt/sequoiadb/packet/sequoiadb-1.10-linux_x86_64-installer.run","HostInfo":{"IP":"192.168.20.42","HostName":"susetzb","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentService":"11790","InstallPath":"/opt/sequoiadb"} } ;
   SYS_JSON: task id, the format is: { "TaskID":1 } ;
   ENV_JSON: {}
   OTHER_JSON: {}
@return
   RET_JSON: the format is: {"errno":0,"detail":"","IP":"192.168.20.166"}
*/

// global
var FILE_NAME_ADD_HOST = "addHost.js" ;
var RET_JSON           = new addHostResult() ;
var rc                 = SDB_OK ;
var errMsg             = "" ;

var host_ip            = "" ;
var host_name          = "" ;
var task_id            = 0 ;

var remote_precheck_result_file = "" ;
var result_file                 = "" ;

var progs = null ;
var spts  = [ "error.js", "common.js", "define.js", "log.js",
              "func.js", "addHostPreCheck.js" ] ;
if ( SYS_LINUX == SYS_TYPE )
{
   progs = [ "sdb" ] ; 
}
else
{
   // TODO: windows
}

/* *****************************************************************************
@discretion: init
@author: Tanzhaobo
@parameter void
@return void
***************************************************************************** */
function _init()
{
   // 1. get task id
   task_id = getTaskID( SYS_JSON ) ;
  
   // 2. specify log file's name
   try
   {
      host_ip   = BUS_JSON[HostInfo][IP] ;
      host_name = BUS_JSON[HostInfo][HostName] ;
   }
   catch ( e )
   {
      SYSEXPHANDLE( e ) ;
      errMsg = sprintf( "Failed to create js log file for adding host[?]", host_ip ) ;
      PD_LOG( arguments, PDERROR, FILE_NAME_ADD_HOST,
              sprintf( errMsg + ", rc: ?, detail: ?", GETLASTERROR(), GETLASTERRMSG() ) ) ;
      exception_handle( SDB_INVALIDARG, errMsg ) ;
   }
   setTaskLogFileName( task_id ) ;

   // 3. set local and remote pre-check result file name
   if( SYS_LINUX == SYS_TYPE )
   {
      remote_precheck_result_file = OMA_FILE_TEMP_ADD_HOST_CHECK ;
      try
      {
         result_file = adaptPath( System.getEWD() ) + "../conf/log/addHostPreCheckResult" ;
         if ( File.exist( result_file ) )
            File.remove( result_file ) ;
      }
      catch( e )
      {
         SYSEXPHANDLE( e ) ;
         errMsg = sprintf( "Failed to initialize add host pre-check result file in local host" ) ;
         rc = GETLASTERROR() ;
         PD_LOG2( task_id, arguments, PDEVENT, FILE_NAME_ADD_HOST,
                  sprintf( errMsg + ", rc: ?, detail: ?", rc, GETLASTERRMSG() ) ) ;
         exception_handle( rc, errMsg ) ;
      }
   }
   else
   {
      // TODO: windows
   }
   
   PD_LOG2( task_id, arguments, PDEVENT, FILE_NAME_ADD_HOST,
            sprintf("Begin to add host[?]", host_ip) ) ;
}

/* *****************************************************************************
@discretion: final
@author: Tanzhaobo
@parameter void
@return void
***************************************************************************** */
function _final()
{
   try
   {
      // remove add host pre-check result file
      if ( File.exist( result_file ) )
         File.remove( result_file ) ;
   }
   catch( e )
   {
      SYSEXPHANDLE( e ) ;
      PD_LOG2( task_id, arguments, PDWARNING, FILE_NAME_ADD_HOST,
               sprintf( "Failed to remove add host pre-check result file in localhost, rc: ?, detail: ?",
                        GETLASTERROR(), GETLASTERRMSG() ) ) ;
   }
   
   PD_LOG2( task_id, arguments, PDEVENT, FILE_NAME_ADD_HOST,
            sprintf("Finish adding host[?]", host_ip) ) ;
}

/* *****************************************************************************
@discretion: get the name of install packet
@author: Tanzhaobo
@parameter
   packet[string]: the full name of the packet,
                   e.g. /tmp/packet/sequoiadb-1.8-linux_x86_64-installer.run
@return
   packetname[string]: the name of the install packet
***************************************************************************** */
function _getInstallPacketName( packet )
{
   var s = "" ;
   var i = 1 ;
   var packetname = "" ;
   if ( SYS_LINUX == SYS_TYPE )
   {
      s = "/" ;
      i = packet.lastIndexOf( s ) ;
      if ( -1 != i )
         packetname = packet.substring( i+1 ) ;
      else
         packetname = packet ;
   }
   else
   {
      // TODO:
   }
   return packetname ;
}

/* *****************************************************************************
@discretion: get local db packet's md5
@author: Tanzhaobo
@parameter
   install_packet[string]: the full name of the packet,
      e.g. /tmp/packet/sequoiadb-1.8-linux_x86_64-installer.run
@return
   [string]: the md5 of local db install packet
***************************************************************************** */
function _getLocalDBPacketMD5( install_packet )
{
   try
   {
      return Hash.fileMD5( install_packet ) ;
   }
   catch( e )
   {
      SYSEXPHANDLE( e ) ;
      errMsg = "Failed get local db install packet's md5" ;
      PD_LOG2( task_id, arguments, PDSEVERE, FILE_NAME_ADD_HOST,
               sprintf( errMsg + ", rc: ?, detail: ?", GETLASTERROR(), GETLASTERRMSG() ) ) ;
      exception_handle( SDB_SYS, errMsg ) ;
   }
}

/* *****************************************************************************
@discretion: judge whether need to add current host
@author: Tanzhaobo
@parameter
   ssh[object]: Ssh object
   install_packet[string]: local db install packet
   install_path[string]: remote db install path
@return
   [bool]: true for need to install while false for not
***************************************************************************** */
function _needToAdd( ssh, install_packet, install_path )
{
   /*
   1. pre-check
   2. get pre-check result
   3. analysis
   */
   var obj            = null ;
   var remote_md5     = "" ;
   var local_md5      = "" ;
   var isProgramExist = false ;
   var str            = "" ;
   var retMsg         = "" ;
   var js_files       = "" ;
   
   if ( SYS_LINUX == SYS_TYPE )
   {
      // set execute command run by ./sdb
      /*
      /tmp/omatmp/bin/sdb -e 'var install_path = "/opt/sequoiadb"' -f '/tmp/omatmp/conf/script/define.js, /tmp/omatmp/conf/script/error.js, /tmp/omatmp/conf/script/log.js, /tmp/omatmp/conf/script/common.js, /tmp/omatmp/conf/script/func.js, /tmp/omatmp/conf/script/addHostPreCheck.js'
      */
      js_files = "/tmp/omatmp/conf/script/define.js" + ", " ;
      js_files += "/tmp/omatmp/conf/script/error.js" + ", " ;
      js_files += "/tmp/omatmp/conf/script/log.js" + ", " ;
      js_files += "/tmp/omatmp/conf/script/common.js" + ", " ;
      js_files += "/tmp/omatmp/conf/script/func.js" + ", " ;
      js_files += "/tmp/omatmp/conf/script/addHostPreCheck.js" ;
      str = '/tmp/omatmp/bin/sdb ' + ' -e ' + ' " var install_path = ' + '\'' + install_path + '\'' + ' " ' + ' -f ' + ' " ' + js_files + ' " ' ;

   }
   else
   {
      // TODO: windows
   }

   // 1. pre-check before add current host
   try
   {
      ssh.exec( str ) ;
      // record the return msg to log file
      retMsg = ssh.getLastOut() ;
      PD_LOG2( task_id, arguments, PDEVENT, FILE_NAME_ADD_HOST,
               sprintf( "Received message from remote host[?] as below:?===>??<===",
               ssh.getPeerIP(), OMA_NEW_LINE, OMA_NEW_LINE, retMsg ) ) ;
   }
   catch( e )
   {
      SYSEXPHANDLE( e ) ;
      retMsg = ssh.getLastOut() ;
      errMsg = sprintf( "Failed to pre-check before add host[?]", ssh.getPeerIP() ) ;
      PD_LOG2( task_id, arguments, PDERROR, FILE_NAME_ADD_HOST,
               sprintf( errMsg + ", received message from remote host[?] as below:?===>??<===",
               ssh.getPeerIP(), OMA_NEW_LINE, OMA_NEW_LINE, retMsg ) ) ;
      return true ;
   }

   // 2. get pre-check result
   try
   {
      ssh.pull( remote_precheck_result_file, result_file ) ;
   }
   catch( e )
   {
      SYSEXPHANDLE( e ) ;
      errMsg = sprintf( "Add host pre-check's result does not exist in host[?], rc: ?, detail: ?",
                        ssh.getPeerIP(), GETLASTERROR(), GETLASTERRMSG() ) ;
      PD_LOG2( task_id, arguments, PDWARNING, FILE_NAME_ADD_HOST, errMsg ) ;
      return true ;
   }

   // 3. analysis
   // check whether remote install packet's md5 is the same with local one or not
   try
   {
      // TODO: need to rename
      obj = eval( "(" + Oma.getOmaConfigs( result_file )  + ")" ) ;
      remote_md5 = obj[MD5] ;
      isProgramExist = obj[ISPROGRAMEXIST] ;
      if ( "string" != typeof(remote_md5) && 32 != remote_md5.length )
      {
         PD_LOG2( task_id, arguments, PDWARNING,
                  sprintf("Remote install packet's md5[?] is invalid", remote_md5) ) ;
         return true ;
      }
      if ( ( "string" == typeof(isProgramExist ) && "true" != isProgramExist ) ||
           ( "boolean" == typeof(isProgramExist ) && true != isProgramExist ) )
      {
         PD_LOG2( task_id, arguments, PDWARNING, FILE_NAME_ADD_HOST,
                  "Remote installed db programs are incomplete" ) ;
         return true ;
      }
   }
   catch( e )
   {
      SYSEXPHANDLE( e ) ;
      errMsg = sprintf( "Failed to analysis remote host[?]'s install information" +
                        " from file[?] in localhost, rc: ?, detail: ?",
                        ssh.getPeerIP(), result_file, GETLASTERROR(), GETLASTERRMSG() ) ;
      PD_LOG2( task_id, arguments, PDWARNING, FILE_NAME_ADD_HOST, errMsg ) ;
      return true ;
   }
   // get local install packet's md5
   try
   {
      local_md5 = _getLocalDBPacketMD5( install_packet ) ;
   }
   catch( e )
   {
      return true ;
   }
   if ( local_md5 != remote_md5 )
   {
      errMsg = sprintf( "Local db packet's md5: ?, remote db packet's md5: ?, need to install",
                        local_md5, remote_md5 ) ;
      PD_LOG2( task_id, arguments, PDWARNING, FILE_NAME_ADD_HOST, errMsg ) ;
      return true ;
   }

   return false ;
}

/* *****************************************************************************
@discretion: push install packet to remote host
@author: Tanzhaobo
@parameter
   ssh[object]: Ssh object
   packet[string]: the full name of the packet,
                   e.g. /tmp/packet/sequoiadb-1.8-linux_x86_64-installer.run
@return void
***************************************************************************** */
function _pushDBPacket( ssh, packet )
{
   var src  = "" ;
   var dest = "" ;
   var cmd  = "" ;
   var packetName = _getInstallPacketName( packet ) ;
   
   if ( SYS_LINUX == SYS_TYPE )
   {
      try
      {
         // installer.run
         src = packet;
         dest = OMA_PATH_TEMP_PACKET_DIR_L + packetName ;
         ssh.push( src, dest ) ;
         cmd = "chmod a+x " + OMA_PATH_TEMP_PACKET_DIR_L + packetName ;
         ssh.exec( cmd ) ;
      }
      catch ( e )
      {
         SYSEXPHANDLE( e ) ;
         rc = GETLASTERROR() ;
         errMsg = sprintf( "Failed to push db packet to host[?]", ssh.getPeerIP() ) ;
         PD_LOG2( task_id, arguments, PDERROR, FILE_NAME_ADD_HOST,
                  sprintf( errMsg + ", rc: ?, detail: ?", rc, GETLASTERRMSG() ) ) ;
         exception_handle( rc, errMsg ) ;
      }
   }
   else
   {
      // TODO: tanzhaobo
   }
}

/* *****************************************************************************
@discretion: push install packet to remote host
@author: Tanzhaobo
@parameter
   ssh[object]: Ssh object
   sdbuser[string]: the user to be add for running sequoiadb program
   sdbpasswd[string]: the password of sdbuser
   packet[string]: the full name of the packet,
                   e.g. /tmp/packet/sequoiadb-1.8-linux_x86_64-installer.run
   path[string]: the path where the install packet is in local host, we need 
                 to push this packet to remote host
@return void
***************************************************************************** */
function _installDBPacket( ssh, sdbuser, sdbpasswd, packet, path )
{
   var cmd = "" ;
   var option = "" ;
   option += " --mode unattended " + " --prefix " + path ;
   option += " --username " + sdbuser + " --userpasswd " + sdbpasswd ;
   var packetName = _getInstallPacketName( packet ) ; 
   if ( SYS_LINUX == SYS_TYPE )
   {
      cmd = OMA_PATH_TEMP_PACKET_DIR_L + packetName + option ;
      try
      {
         ssh.exec( cmd ) ;
      }
      catch ( e )
      {
         SYSEXPHANDLE( e ) ;
         rc = GETLASTERROR() ;
         errMsg = sprintf( "Failed to install db packet in host[?]", ssh.getPeerIP() ) ;
         PD_LOG2( task_id, arguments, PDERROR, FILE_NAME_ADD_HOST,
                  sprintf( errMsg + ", rc: ?, detail: ?", rc, GETLASTERRMSG() ) ) ;
         exception_handle( rc, errMsg ) ;
      }
   }
   else
   {
      // TODO: tanzhaobo
   }
}

/* *****************************************************************************
@discretion: uninstall db packet in remote host when install failed
@author: Tanzhaobo
@parameter
   ssh[object]: ssh object
   path[string]: the path db installed in
@return void
***************************************************************************** */
function _uninstallDBPacket( ssh, path )
{
   var cmd = "" ;
   var str = adaptPath( path ) ;
   if ( SYS_LINUX == SYS_TYPE )
   {
      // try to stop sdbcm
      try
      {
         cmd = str + OMA_PROG_BIN_SDBCMTOP_L ; 
         ssh.exec( cmd ) ;
      }
      catch ( e )
      {
      }
      // remove db packet
      try
      {
         cmd = str + OMA_PROG_UNINSTALL_L ;
         ssh.exec( "chmod a+x " + cmd ) ;
         ssh.exec( cmd + " --mode unattended " ) ;
      }
      catch ( e )
      {
      }
   }
   else
   {
      // DOTO: tanzhaobo
   }
}

function main()
{
   _init() ;
   
   var sdbUser         = null ;
   var sdbPasswd       = null ;
   var sdbUserGroup    = null ;
   var installPacket   = null ;
   var hostInfo        = null ;

   var ip              = null ;
   var user            = null ;
   var passwd          = null ;
   var sshPort         = null ;
   var agentService    = null ;
   var installPath     = null ;

   var ssh             = null ;
   var hashcode_local  = null ;
   var hashcode_remote = null ;
   var flag            = false ;

   try
   {
      sdbUser          = BUS_JSON[SdbUser] ;
      sdbPasswd        = BUS_JSON[SdbPasswd] ;
      sdbUserGroup     = BUS_JSON[SdbUserGroup] ;
      installPacket    = BUS_JSON[InstallPacket] ;
      hostInfo         = BUS_JSON[HostInfo] ;
  
      ip               = hostInfo[IP] ;
      RET_JSON[IP]     = ip ;
      user             = hostInfo[User] ;
      passwd           = hostInfo[Passwd] ;
      sshPort          = parseInt(hostInfo[SshPort]) ;
      agentService     = hostInfo[AgentPort] ;
      installPath      = hostInfo[InstallPath] ;
   }
   catch ( e )
   {
      SYSEXPHANDLE( e ) ;
      errMsg = "Js receive invalid argument" ;
      rc = GETLASTERROR() ;
      // record error message in log
      PD_LOG2( task_id, arguments, PDSEVERE, FILE_NAME_ADD_HOST,
               sprintf( errMsg + ", rc: ?, detail: ?", rc, GETLASTERRMSG() ) ) ;
      // tell to user error happen
      exception_handle( SDB_INVALIDARG, errMsg ) ;
   }

   // 1. ssh to target host
   try
   {
      ssh = new Ssh( ip, user, passwd, sshPort ) ;
   }
   catch( e )
   {
      SYSEXPHANDLE( e ) ;
      rc = GETLASTERROR() ;
      errMsg = sprintf( "Failed to ssh to host[?]", ip ) ;
      PD_LOG2( task_id, arguments, PDSEVERE, FILE_NAME_ADD_HOST,
               sprintf( errMsg + ", rc: ?, detail: ?", rc, GETLASTERRMSG() ) ) ;
      exception_handle( rc, errMsg ) ;
   }
   
   // 2. judge whether it's in local host, if so, no need to install
   flag = isInLocalHost( ssh ) ;
   if ( flag )
   {
      PD_LOG2( task_id, arguments, PDEVENT, FILE_NAME_ADD_HOST,
               sprintf("It's in localhost[?], no need to install SequoiaDB in local", ip) ) ;
      _final() ;
      return RET_JSON ;
   }
   
   // 3. create temporary directory in remote host
   createTmpDir( ssh ) ;
      
   // 4. push tool programs and js script files to target host for checking
   pushProgAndSpt( ssh, progs, spts ) ;

   // 5. check whether need to add current host or not
   flag = _needToAdd( ssh, installPacket, installPath ) ;
   if ( false == flag ) 
   {
      PD_LOG2( task_id, arguments, PDEVENT, FILE_NAME_ADD_HOST,
               sprintf( "The same kind of SequoiaDB has been installed" +
                        " in target host[?], no need to install", ip) ) ;
      _final() ;
      return RET_JSON ;
   }

   PD_LOG2( task_id, arguments, PDEVENT, FILE_NAME_ADD_HOST,
            sprintf( "Need to install SequoiaDB in target host[?]", ip ) ) ;

   // 6. push db packet to remote host
   _pushDBPacket( ssh, installPacket ) ;
   
   // 7. install db packet
   try
   {
      _installDBPacket( ssh, sdbUser, sdbPasswd, installPacket, installPath ) ;
   }
   catch ( e )
   {
      SYSEXPHANDLE( e ) ;
      errMsg = sprintf( "Failed to install db packet in host[?]", ip ) ;
      rc = GETLASTERROR() ;
      PD_LOG2( task_id, arguments, PDERROR, FILE_NAME_ADD_HOST,
               sprintf( errMsg + ", rc: ?, detail: ?", rc, GETLASTERRMSG() ) ) ;
      // try to remove the packet
      try
      {
         _uninstallDBPacket( ssh, installPath ) ;
      }
      catch( e )
      {}
      exception_handle( rc, errMsg ) ;
   }
   
   PD_LOG2( task_id, arguments, PDEVENT, FILE_NAME_ADD_HOST,
            sprintf( "Success to install SequoiaDB in target host[?]", ip ) ) ;
   
   // 8. remove temporary directory in remote host
   try
   {
      removeTmpDir2( ssh )
   }
   catch( e )
   {
   }
   
   _final() ;
   // return the result
   return RET_JSON ;
}

// execute
   main() ;


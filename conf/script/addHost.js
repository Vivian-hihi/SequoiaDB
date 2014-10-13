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
@description: add host to cluster( install db patcket and start sdbcm )
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter
   BUS_JSON: the format is: {"SdbUser":"sdbadmin","SdbPasswd":"sdbadmin","SdbUserGroup":"sdbadmin_group","InstallPacket":"/home/users/tanzhaobo/sequoiadb/bin/../packet/sequoiadb-1.8-linux_x86_64-installer.run","HostInfo":[{"IP":"192.168.20.42","HostName":"susetzb","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"},{"IP":"192.168.20.165","HostName":"rhel64-test8","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"},{"IP":"192.168.20.166","HostName":"rhel64-test9","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"}]}
   SYS_JSON: {}
   ENV_JSON: {}
   OTHER_JSON: {}
@return
   RET_JSON: the format is: {"errno":0,"detail":"","HostInfo":[{"errno":0,"detail":"","IP":"192.168.20.42","HasInstall":true},{"errno":0,"detail":"","IP":"192.168.20.165","HasInstall":true}]}
*/

//var BUS_JSON = {"SdbUser":"sdbadmin","SdbPasswd":"sdbadmin","SdbUserGroup":"sdbadmin_group","InstallPacket":"/home/users/tanzhaobo/sequoiadb/bin/../packet/sequoiadb-1.8-linux_x86_64-installer.run","HostInfo":[{"IP":"192.168.20.42","HostName":"susetzb","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"},{"IP":"192.168.20.165","HostName":"rhel64-test8","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"},{"IP":"192.168.20.166","HostName":"rhel64-test9","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"}]} ;

//var BUS_JSON = {"SdbUser":"sdbadmin","SdbPasswd":"sdbadmin","SdbUserGroup":"sdbadmin_group","InstallPacket":"/home/users/tanzhaobo/sequoiadb/bin/../packet/sequoiadb-1.8-linux_x86_64-installer.run","HostInfo":[{"IP":"192.168.20.165","HostName":"rhe164-test8","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"},{"IP":"192.168.20.166","HostName":"rhel64-test9","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"}]} ;

//var BUS_JSON = {"SdbUser":"sdbadmin","SdbPasswd":"sdbadmin","SdbUserGroup":"sdbadmin_group","InstallPacket":"/home/users/tanzhaobo/sequoiadb/bin/../packet/sequoiadb-1.8-linux_x86_64-installer.run","HostInfo":[{"IP":"192.168.20.165","HostName":"rhe164-test8","User":"root","Passwd":"sequoiadb","SshPort":"22","AgentPort":"11790","InstallPath":"/opt/sequoiadb"}]} ;

var RET_JSON       = new Object() ;
RET_JSON[Errno]    = SDB_OK ;
RET_JSON[Detail]   = "" ;
RET_JSON[HostInfo] = [] ;

/* *****************************************************************************
@discretion: get the name of install packet
@author: Tanzhaobo
@parameter
   osInfo[string]: os type
   packet[string]: the full name of the packet,
                   e.g. /tmp/packet/sequoiadb-1.8-linux_x86_64-installer.run
@return
   packetname[string]: the name of the install packet
***************************************************************************** */
function getInstallPacketName( osInfo, packet )
{
   var s = "" ;
   var i = 1 ;
   var packetname = "" ;
   if ( OMA_LINUX == osInfo )
      s = "/" ;
   else
      s = "\\" ;
   i = packet.lastIndexOf( s ) ;
   if ( -1 != i )
   {
      packetname = packet.substring( i+1 ) ;
   }
   else
   {
      packetname = packet ;
   }
   return packetname ;
}

/* *****************************************************************************
@discretion: create tmp diretory in /tmp
@author: Tanzhaobo
@parameter
   ssh[object]: Ssh object
   osInfo[string]: os type
@return void
***************************************************************************** */
function createTmpDir( ssh, osInfo )
{
   var cmd = "" ;
   if ( OMA_LINUX == osInfo )
   {
      try
      {
         // mkdir /tmp/omatmp
         cmd = "mkdir -p " + OMA_PATH_TEMP_OMA_DIR_L ;
         ssh.exec( cmd ) ;
         // mkdir  /tmp/omatmp/packet
         cmd = "mkdir -p " + OMA_PATH_TEMP_PACKET_DIR_L ; 
         ssh.exec( cmd ) ;
         // mkdir  /tmp/omatmp/data/vCoord
         cmd = "mkdir -p " + OMA_PATH_VCOORD_PATH_L ;
         ssh.exec( cmd ) ;
         // chmod
         cmd = "chmod -R 777 " + OMA_PATH_TEMP_OMA_DIR_L ;
         ssh.exec( cmd ) ;
      }
      catch( e )
      {
         setLastErrMsg( "Failed to create tmp director" ) ;
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

/* *****************************************************************************
@discretion: push install packet to remote host
@author: Tanzhaobo
@parameter
   ssh[object]: Ssh object
   osInfo[string]: os type
   packet[string]: the full name of the packet,
                   e.g. /tmp/packet/sequoiadb-1.8-linux_x86_64-installer.run
@return void
***************************************************************************** */
function pushInstallationPacket( ssh, osInfo, packet )
{
   var src = "" ;
   var dest = "" ;
   var packetName = getInstallPacketName( osInfo, packet ) ;
   createTmpDir( ssh, osInfo ) ;
   if ( OMA_LINUX == osInfo )
   {
      try
      {
         // installer.run
         src = packet;
         dest = OMA_PATH_TEMP_PACKET_DIR_L + packetName ;
         ssh.push( src, dest ) ;
         var cmd = "chmod a+x " + OMA_PATH_TEMP_PACKET_DIR_L + packetName ;
         ssh.exec( cmd ) ;
      }
      catch ( e )
      {
         setLastErrMsg( "Failed to push db packet to remote" ) ;
         setLastError( SDB_SYS ) ;
         throw SDB_SYS ;
      }
   }
   else
   {
      // TODO: tanzhaobo
      // push packet in windows
   }
}

/* *****************************************************************************
@discretion: push install packet to remote host
@author: Tanzhaobo
@parameter
   ssh[object]: Ssh object
   osInfo[string]: os type
   sdbuser[string]: the user to be add for running sequoiadb program
   sdbpasswd[string]: the password of sdbuser
   packet[string]: the full name of the packet,
                   e.g. /tmp/packet/sequoiadb-1.8-linux_x86_64-installer.run
   path[string]: the path where the install packet is in local host, we need 
                 to push this packet to remote host
@return void
***************************************************************************** */
function installDBPacket( ssh, osInfo, sdbuser, sdbpasswd, packet, path )
{
   var cmd = "" ;
   var option = "" ;
   option += " --mode unattended " + " --prefix " + path ;
   option += " --username " + sdbuser + " --userpasswd " + sdbpasswd ;
   var packetName = getInstallPacketName( osInfo, packet ) ; 
   if ( OMA_LINUX == osInfo )
   {
      cmd = OMA_PATH_TEMP_PACKET_DIR_L + packetName + option ;
      try
      {
         ssh.exec( cmd ) ; 
      }
      catch ( e )
      {
         setLastErrMsg( "Failed to insall db packet" ) ;
         setLastError( SDB_SYS ) ;
         throw SDB_SYS ;
      }
   }
   else
   {
      // TODO: tanzhaobo
      // execute in windows
   }
}

function main()
{
   var sdbUser         = BUS_JSON[SdbUser] ;
   var sdbPasswd       = BUS_JSON[SdbPasswd] ;
   var sdbUserGroup    = BUS_JSON[SdbUserGroup] ;
   var installPacket   = BUS_JSON[InstallPacket] ;
   var infoArr         = BUS_JSON[HostInfo] ;
   var arrLen          = infoArr.length ;
   if ( arrLen == 0 )
   {
      setLastErrMsg( "Not specified any hosts to add" ) ;
      setLastError( SDB_INVALIDARG ) ;
      throw SDB_INVALIDARG ;
   }
   // get os infomation
   var osInfo = System.type() ;
   for ( var i = 0; i < arrLen; i++ )
   {
      var ssh         = null ;
      var obj         = infoArr[i] ;
      var ip          = obj[IP] ;
      var user        = obj[User] ;
      var passwd      = obj[Passwd] ;
      var sshPort     = obj[SshPort] ;
      var agentPort   = obj[AgentPort] ;
      var installPath = obj[InstallPath] ;
      var retObj      = new addHostResult() ;
      retObj[IP]      = ip ;
      try
      {
         // ssh
         var ssh = new Ssh( ip, user, passwd ) ;
         // judge whether it's in local mathine, if so, no need to install
         var isLocal = isInLocalHost( ssh ) ;
         if ( isLocal )
         {
            retObj[HasInstall] = true ;
            RET_JSON[HostInfo].push( retObj ) ;
            continue ;
         }
         // push packet to remote machine
         pushInstallationPacket( ssh, osInfo, installPacket ) ;
         installDBPacket( ssh, osInfo, sdbUser, sdbPasswd, installPacket, installPath ) ;
         retObj[HasInstall] = true ;
      }
      catch ( e )
      {
         if ( "number" == typeof(e) && e < 0 )
         {
            retObj[Errno] = GETLASTERROR( e, false ) ;
            retObj[Detail] = GETLASTERRMSG() ;
            RET_JSON[HostInfo].push( retObj ) ;
            RET_JSON[Errno] = e ;
            RET_JSON[Detail] = "Failed to install sdbcm in [" + ip + "]: " + retObj[Detail] ;
            break ;
         }
         else
         {
            retObj[Errno] = GETLASTERROR( e, false ) ;
            retObj[Detail] = GETLASTERRMSG() ;
            RET_JSON[HostInfo].push( retObj ) ;
            RET_JSON[Errno] = SDB_SYS ;
            RET_JSON[Detail] = "Failed to install sdbcm in [" + ip + "]" ;
            break ;
         }
      }
      RET_JSON[HostInfo].push( retObj ) ;
   }
//print("RET_JSON is: " + JSON.stringify(RET_JSON) + "\n") ;
   // return the result
   return RET_JSON ;
}

// execute
   main() ;


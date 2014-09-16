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
@description: install agent process in remote mechine
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/
if ( typeof(SDBUSER) == "undefined" ) {}
if ( typeof(SDBPASSWD) == "undefined" ) {}
if ( typeof(SDBUSERGROUP) == "undefined" ) {}
if ( typeof(IP) == "undefined" ) {}
if ( typeof(HOSTNAME) == "undefined" ) {}
if ( typeof(USERNAME) == "undefined" ) {}
if ( typeof(PASSWORD) == "undefined" ) {}
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }
if ( typeof(PACKET_PATH) == "undefined" ) {}
if ( typeof(INSTALL_PATH) == "undefined" ) {}

// linux
var OMA_TMP_DIR              = "/tmp/omatmp" ;
var OMA_INSTALL_PACKET_DIR_L = "/tmp/omatmp/packet/" ;
var OMA_VCOORD_INSTALL_DIR_L = "/tmp/omatmp/data/vCoord/" ;
// windows
var INSTALL_PACKET_PATH_W = "" ;

var objRet = new Object() ;
objRet.Rc         = 0 ;
objRet.detail     = "" ;
objRet.HasPush    = false ;
objRet.HasInstall = false ;


function isLocalHost( osInfo )
{
   var isLocal = false ;
   var hostname = null ;
   var hosts = null ;
   var name = null ;
   var ip = null ;
   var len = 0 ;
   var i = 0 ;

   // get localhost name
   hostname = Cmd.run("hostname") ;
   if ( null != hostname )
   {
      if ( "LINUX" == osInfo )
      {
         i = hostname.indexOf( "\n" ) ;
      }
      else
      {
         i = hostname.indexOf( "\n\r" ) ;
      }
      if ( -1 != i )
      {
         hostname = hostname.substring(0, i);
      }
   }

   // check whether it's in local host env
   hosts = eval( '(' + System.getHostsMap() + ')' ) ;
   if ( null != hosts )
   {
      len = hosts["Hosts"].length ;
      for ( i = 0; i < len; i++ )
      {
         ip = hosts["Hosts"][i]["Ip"] ;
         if ( IP == ip )
         {
            name = hosts["Hosts"][i]["HostName"] ;
            if ( hostname == name )
            {
               isLocal = true ;
               break ;
            }
         }
      }
   }
   return isLocal ;
}

function getInstallPacketName( osInfo )
{
   var s = "" ;
   var i = 1 ;
   var packet = PACKET_PATH ;
   var subStr = "" ;
   if ( "LINUX" == osInfo )
      s = "/" ;
   else
      s = "\\" ;
   i = packet.lastIndexOf( s ) ;
   if ( -1 != i )
   {
      subStr = packet.substring( i+1 ) ;
   }
   else
   {
      subStr = packet ;
   }
   return subStr ;
}

function createTmpDir( ssh, osInfo )
{
   var cmd = "" ;
   if ( "LINUX" == osInfo )
   {
      // mkdir /tmp/omatmp
      cmd = "mkdir -p " + OMA_TMP_DIR ;
      ssh.exec( cmd ) ;
      // mkdir  /tmp/omatmp/packet
      cmd = "mkdir -p " + OMA_INSTALL_PACKET_DIR_L ; 
      ssh.exec( cmd ) ;
      // mkdir  /tmp/omatmp/data/vCoord
      cmd = "mkdir -p " + OMA_VCOORD_INSTALL_DIR_L ;
      ssh.exec( cmd ) ;
      // chmod
      cmd = "chmod -R 777 " + OMA_TMP_DIR ;
      ssh.exec( cmd ) ;
   }
   else
   {
      // DOTO: tanzhaobo
      // windows
   }

}

function pushInstallationPacket( ssh, osInfo )
{
   var src = "" ;
   var dest = "" ;
   var packetName = getInstallPacketName( osInfo ) ;
   createTmpDir( ssh, osInfo ) ;
   if ( "LINUX" == osInfo )
   {
      // installer.run
      src = PACKET_PATH;
      dest = OMA_INSTALL_PACKET_DIR_L + packetName ;
      ssh.push( src, dest ) ;
      var cmd = "chmod a+x " + OMA_INSTALL_PACKET_DIR_L + packetName ;
      ssh.exec( cmd ) ;
   }
   else
   {
      // TODO: tanzhaobo
      // push packet in windows
   }
   objRet.HasPush = true ;
}

function installPacket( ssh, osInfo )
{
   var cmd = "" ;
   var option = " --mode unattended " + " --prefix " + INSTALL_PATH +
                " --username " + SDBUSER + " --userpasswd " + SDBPASSWD ;
   var packetName = getInstallPacketName( osInfo ) ; 
   if ( "LINUX" == osInfo )
   {
      cmd = OMA_INSTALL_PACKET_DIR_L + packetName + option ;
      ssh.exec( cmd ) ;
   }
   else
   {
      // TODO: tanzhaobo
      // execute in windows
   }
   objRet.HasInstall = true ;
}

function main()
{
   try
   {
      // check argument
      if ( typeof(SDBUSER) == "undefined"       ||
           typeof(SDBPASSWD) == "undefined"     ||
           typeof(SDBUSERGROUP) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.detail = "not specified user name, password or user group" ;
         return objRet ;
      }
      if ( typeof(USERNAME) == "undefined"      ||
           typeof(PASSWORD) == "undefined"      ||
           typeof(IP) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.detail = "not specified username, password or ip" ;
         return objRet ;
      }
      if ( typeof(PACKET_PATH) == "undefined" ||
           typeof(INSTALL_PATH) == "undefined" )
      {
         objRet.Rc = -10 ;
         objRet.detail = "not specified install packet or install path" ;
         return objRet ;
      }
      // get os info
      var osInfo = System.type() ;
      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      // judge whether it's in local mathine, if so, no need to install
      var isLocal = isLocalHost( osInfo ) ;
      if ( isLocal )
      {
         objRet.HasPush    = false ;
         objRet.HasInstall = true ;
         return objRet ;
      }
      // push packet to remote machine
      pushInstallationPacket( ssh, osInfo ) ;
      installPacket( ssh, osInfo ) ;
      // return the result
      return objRet ;
   }
   catch ( e )
   {
      if ( typeof(e) != "number" )
      {
         objRet.Rc = -10 ;
         objRet.detail = "system error" ;
      }
      else
      {
         var errMsg = "" ;
         objRet.Rc = e ;
         errMsg = getLastErrMsg() ;
         if ( "" != errMsg && null != errMsg && undefined != errMsg )
         {
            objRet.detail = eval( '(' + errMsg + ')' ) ;
         }
      }
      return objRet ;
   }
}

// execute
   main() ;


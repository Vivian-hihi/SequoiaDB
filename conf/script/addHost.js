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
if ( typeof(INSTALL_PACKET) == "undefined" ) {}
if ( typeof(INSTALL_PATH) == "undefined" ) {}

var install_packet_name = null ;

// TODO: tanzhaobo
// just use for test
SDBUSER = "sdbadmin" ;
SDBPASSWD = "sdbadmin" ;
SDBUSERGROUP = "sdbadmin" ;
INSTALL_PACKET = "/home/users/tanzhaobo/sequoiadb/bin/sequoiadb-1.8-linux_x86_64-installer.run" ;

// linux
var INSTALL_PACKET_PATH_L = "/tmp/tmpoma/packet/" ;
// windows
var INSTALL_PACKET_PATH_W = "" ;

var objRet = new Object() ;
objRet.Rc         = 0 ;
objRet.Detail     = "" ;
objRet.HasPush    = false ;
objRet.HasInstall = false ;

function getInstallPacketName( osInfo )
{
   var s = "" ;
   var i = 1 ;
   var packet = INSTALL_PACKET ;
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
      cmd = "mkdir -p " + INSTALL_PACKET_PATH_L ;
print("AAAAAAAAAAAAAAAAAAAAAAA cmd is: " + cmd + "\n") ;
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
print("********************************packetName is: " + packetName + "\n") ;
   createTmpDir( ssh, osInfo ) ;
   if ( "LINUX" == osInfo )
   {
      // installer.run
      src = INSTALL_PACKET;
      dest = INSTALL_PACKET_PATH_L + packetName ;
print("44444src is: " + src + "\n" ) ;
print("44444dest is: " + dest + "\n" ) ;
      ssh.push( src, dest ) ;
print(">>>>>>>>finish push packet>>>>>>>>>>\n") ;
      var cmd = "chmod a+x " + INSTALL_PACKET_PATH_L + packetName ;
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
      cmd = INSTALL_PACKET_PATH_L + packetName + option ;
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
print("sdbuser is: " + SDBUSER + "\n" )  ;
print("sdbpasswd is: " + SDBPASSWD + "\n" ) ;
print("sdbusergroup is: " + SDBUSERGROUP + "\n" ) ;
print("username is: " + USERNAME + "\n" ) ;
print("password is: " + PASSWORD + "\n" ) ;
print("ip is: " + IP + "\n" ) ;
print("hostname is: " + HOSTNAME + "\n" ) ;
print("install path is: " + INSTALL_PATH + "\n" ) ;
print("packet is: " + INSTALL_PACKET + "\n" ) ;
   try
   {
      // check argument
      if ( typeof(SDBUSER) == "undefined"       ||
           typeof(SDBPASSWD) == "undefined"     ||
           typeof(SDBUSERGROUP) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "not specified user name, password or user group for program" ;
         return objRet ;
      }
      if ( typeof(USERNAME) == "undefined"      ||
           typeof(PASSWORD) == "undefined"      ||
           typeof(IP) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "not specified username, password or ip" ;
         return objRet ;
      }
      if ( typeof(INSTALL_PACKET) == "undefined" ||
           typeof(INSTALL_PATH) == "undefined" )
      {
         objRet.Rc = -10 ;
         objRet.Detail = "not specified installation packet or installation path" ;
         return objRet ;
      }
      // get os info
      var osInfo = System.type() ;
print("111111111111111111111111111111111111111111111\n") ;
      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
print("22222222222222222222222222222222222222222222222\n") ;
      // push packet to remote machine
      pushInstallationPacket( ssh, osInfo ) ;
print("33333333333333333333333333333333333333333333333\n") ;
      installPacket( ssh, osInfo ) ;
print("55555555555555555555555555555555555555555555555555\n") ;
      // return the result
      return objRet ;
   }
   catch ( e )
   {
      if ( typeof(e) != "number" )
      {
         objRet.Rc = -10 ;
         objRet.Detail = "system error" ;
      }
      else
      {
         objRet.Rc = e ;
         objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
      }
      return objRet ;
   }
}

// execute
   main() ;


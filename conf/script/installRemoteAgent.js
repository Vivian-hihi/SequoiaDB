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

if ( typeof(USERNAME) == "undefined" ) {}
if ( typeof(PASSWORD) == "undefined" ) {}
if ( typeof(IP) == "undefined" ) {}
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }
if ( typeof(LOCAL_PROG_PATH) == "undefined" ) {}
if ( typeof(LOCAL_SPT_PATH) == "undefined" ) {}
if ( typeof(LOCAL_CM_CONF) == "undefined" ) {}

// linux
var TOPDIR_L               = "/tmp/omatmp/" ;
var PROGDIR_L              = "/tmp/omatmp/bin/" ;
var CONFDIR_L              = "/tmp/omatmp/conf/" ;
var LOGDIR_L               = "/tmp/omatmp/conf/log/" ;
var SPTDIR_L               = "/tmp/omatmp/conf/script/" ;

var SDBCM_L                = "sdbcm" ;
var SDBCMD_L               = "sdbcmd" ;
var SDBCMART_L             = "sdbcmart" ;
var SDBCMTOP_L             = "sdbcmtop" ;

// windows
var TOPDIR_W               = "" ;
var PROGDIR_W              = "" ;
var CONFDIR_W              = "" ;
var LOGDIR_W               = "" ;
var SPTDIR_W               = "" ;

var SDBCM_W                = "sdbcm.exe" ;
var SDBCMD_W               = "sdbcmd.exe" ;
var SDBCMART_W             = "sdbcmart.exe" ;
var SDBCMTOP_W             = "sdbcmtop.exe" ;

var SDBCMCONF              = "sdbcm.conf" ;
var FILE_CHECK_HOST        = "checkHost.js" ;

var objRet = new Object() ;
objRet.Rc              = 0 ;
objRet.Detail          = "" ;
objRet.IsNeedUninstall = true ;

function createTmpDir( ssh, osInfo )
{
   var cmd = "" ;
   if ( "LINUX" == osInfo )
   {
     // rm /tmp/omatmp
     cmd = "rm " + TOPDIR_L + " -rf " ;
     ssh.exec( cmd ) ;
     // mkdir /tmp/omatmp
     cmd = "mkdir " + TOPDIR_L ;
     ssh.exec( cmd ) ;
     // mkdir /tmp/omatmp/bin
     cmd = "mkdir " + PROGDIR_L ;
     ssh.exec( cmd ) ;
     // mkdir /tmp/omatmp/conf
     cmd = "mkdir " + CONFDIR_L ;
     ssh.exec( cmd ) ;
     // mkdir /tmp/omatmp/conf/log
     cmd = "mkdir " + LOGDIR_L ;
     ssh.exec( cmd ) ;
     // mkdir /tmp/omatmp/conf/script
     cmd = "mkdir " + SPTDIR_L ;
     ssh.exec( cmd ) ;
   }
   else
   {
      // TODO: tanzhaobo
      // create dir in windows
   }
}

function pushPacket( ssh, osInfo )
{
   var src = "" ;
   var dest = "" ;
   if ( "LINUX" == osInfo )
   {
      // sdbcm
      src = LOCAL_PROG_PATH + SDBCM_L;
      dest = PROGDIR_L + SDBCM_L ;
      ssh.push( src, dest ) ;
      // sdbcmd
      src = LOCAL_PROG_PATH + SDBCMD_L;
      dest = PROGDIR_L + SDBCMD_L ;
      ssh.push( src, dest ) ;
      // sdbcmart
      src = LOCAL_PROG_PATH + SDBCMART_L ;
      dest = PROGDIR_L + SDBCMART_L ;
      ssh.push( src, dest ) ;
      // sdbcmtop
      src = LOCAL_PROG_PATH + SDBCMTOP_L ;
      dest = PROGDIR_L + SDBCMTOP_L ;
      ssh.push( src, dest ) ;
      // conf file
      src = LOCAL_CM_CONF ;
      dest = CONFDIR_L + SDBCMCONF ;
      ssh.push( src, dest ) ;
      // script checkHost.js
      src = LOCAL_SPT_PATH + FILE_CHECK_HOST ;
      dest = SPTDIR_L + FILE_CHECK_HOST ;
      ssh.push( src, dest ) ;
   }
   else
   {
      // TODO: tanzhaobo
      // push program in windows
   }
}

function hasCMInstalledInLocal( osInfo )
{
   var isLocalHost = false ;
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
               isLocalHost = true ;
               break ;
            }
         }
      }
   }
   return isLocalHost ;
}

// check whether sdbcm is running, if so, not need to install
function cmProgHasInstalled( ssh, osInfo )
{
   var hasInstalled = false ;
   // case 1: check whether sdbcm has been installed
   // in local machine
   var flag = hasCMInstalledInLocal( osInfo ) ;
   if ( flag )
   {
      hasInstalled = true ;
      return hasInstalled ;
   }
   // TODO: tanzhaobo
   // case 2: check whether sdbcm has been installed
   // in remote machine

   return hasInstalled ;
}

function startCMProg( ssh, osInfo )
{
   var cmd = "" ;
   if ( "LINUX" == osInfo )
   {
      cmd = PROGDIR_L + SDBCMART_L ;
      ssh.exec( cmd ) ;
   }
   else
   {
      cmd = PROGDIR_W + SDBCMART_W ;
      ssh.exec( cmd ) ;
   }
}

function stopCMProg( ssh, osInfo )
{
   var cmd = "" ;
   if ( "LINUX" == osInfo )
   {
      cmd = PROGDIR_L + SDBCMTOP_L ;
      ssh.exec( cmd ) ;
   }
   else
   {
      cmd = PROGDIR_W + SDBCMTOP_W ;
      ssh.exec( cmd ) ;
   }

}

function main()
{
   var ssh    = null ;
   var osInfo = null ;
   try
   {
      // check argument
      if ( typeof(USERNAME) == "undefined"      ||
           typeof(PASSWORD) == "undefined"      ||
           typeof(IP) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "not specified username, password or ip" ;
         return objRet ;
      }
      if ( typeof(LOCAL_PROG_PATH) == "undefined" ||
           typeof(LOCAL_SPT_PATH) == "undefined"  ||
           typeof(LOCAL_CM_CONF) == "undefined" )
      {
         objRet.Rc = -10 ;
         objRet.Detail = "not specified sdbcm, js script or sdbcm config file" ;
         return objRet ;
      }

      // get os info
      osInfo = System.type() ;

      // ssh
      ssh = new Ssh( IP, USERNAME, PASSWORD ) ;

      // test whether sdbcm has been installed or not      
      var flag = cmProgHasInstalled( ssh, osInfo ) ;
      if ( flag )
      {
         objRet.IsNeedUninstall = false ;
         return objRet ;
      }

      // build directory in remote mechine
      createTmpDir( ssh, osInfo ) ;

      // push packet
      pushPacket( ssh, osInfo ) ;

      // TODO: tanzhaobo
      // it's better for us to check it than to stop it
      // stop the existing sdbcm program anyway
      stopCMProg( ssh, osInfo ) ;

      // start the omagent program
      startCMProg( ssh, osInfo ) ;

      sleep( 3000 ) ;
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
         var errMsg = "" ;
         objRet.Rc = e ;
         errMsg = getLastErrMsg() ;
         if ( "" != errMsg && null != errMsg && undefined != errMsg )
         {
            objRet.Detail = eval( '(' + errMsg + ')' ) ;
         }
      }
      return objRet ;
   }
}

// execute
   main() ;


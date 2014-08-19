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
var SDBCM_W                = "sdbcm.exe" ;
var SDBCMD_W               = "sdbcmd.exe" ;
var SDBCMART_W             = "sdbcmart.exe" ;
var SDBCMTOP_W             = "sdbcmtop.exe" ;

var SDBCMCONF              = "sdbcm.conf" ;
var FILE_CHECK_HOST        = "checkHost.js" ;
var FILE_EXIT_AGENT        = "exitAgent.js" ;

var objRet = new Object() ;
objRet.Rc = 0 ;
objRet.Detail = "" ;
objRet.HasPush = false ;
objRet.HasRunning = false ;

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
      // script exitAgent.js
      src = LOCAL_SPT_PATH + FILE_EXIT_AGENT ;
      dest = SPTDIR_L + FILE_EXIT_AGENT ;
      ssh.push( src, dest ) ;
   }
   else
   {
      // TODO: tanzhaobo
      // push program in windows
   }
}

function startCMProg( ssh, osInfo )
{
   var cmd = "" ;
   if ( "LINUX" == osInfo )
   {
      cmd = PROGDIR_L + "/" + SDBCMART_L ;
      ssh.exec( cmd ) ;
   }
   else
   {
      cmd = PROGDIR_L + "\\" + SDBCMART_L ;
      ssh.exec( cmd ) ;
   }
}

function main()
{
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
      var osInfo = System.type() ;
      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      // build directory in remote mechine
      createTmpDir( ssh, osInfo ) ;
      // push packet
      pushPacket( ssh, osInfo ) ;
      objRet.HasPush = true ;
      // start the omagent program
      startCMProg( ssh, osInfo ) ;
      objRet.HasRunning = true ;
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


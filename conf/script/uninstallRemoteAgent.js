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
*/

if ( typeof(USERNAME) == "undefined" ) {}
if ( typeof(PASSWORD) == "undefined" ) {}
if ( typeof(IP) == "undefined" ) {}
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }

// linux
var TOPDIR_L               = "/tmp/omatmp/"
var PROGDIR_L              = "/tmp/omatmp/bin/" ;
var SDBCMTOP_L             = "sdbcmtop" ;
//windows
var PROGDIR_W              = "" ;
var TOPDIR_W               = ""
var SDBCMTOP_W             = "sdbcmtop.exe" ;

var objRet = new Object() ;
objRet.Rc = 0 ;
objRet.Detail = "" ;
objRet.HasUninstall = false ;

function uninstallRemoteSdbcmPacket( ssh, osInfo )
{
   var cmd = "" ;
   if ( osInfo == "LINUX" )
   {
      cmd = "rm -rf " + TOPDIR_L ;
//      ssh.exec( cmd ) ;
   }
   else
   {
      cmd = "DEL /Q " + TOPDIR_W
      ssh.exec( cmd ) ;
   }
   objRet.HasUninstall = true ;
}

function stopRemoteSdbcmProgram( ssh, osInfo )
{
   var cmd = "" ;
   if ( osInfo == "LINUX" )
   {
      cmd = PROGDIR_L + SDBCMTOP_L ;
      ssh.exec( cmd ) ;
   }
   else
   {
      // TODO: tanzhaobo
   } 
}

function main()
{
   try
   {
      // check argument
      if ( typeof(USERNAME) == "undefined" ||
           typeof(PASSWORD) == "undefined" ||
           typeof(IP) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "not specified username, password or ip" ;
         return objRet ;
      }
      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      // get os infomation
      var osInfo = System.type() ;
      // stop remote sdbcm program
      stopRemoteSdbcmProgram( ssh, osInfo ) ;
      // remove the packet in remote machine
      uninstallRemoteSdbcmPacket( ssh, osInfo ) ;
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
         if ( "" != errMsg )
         {
            objRet.Detail = eval( '(' + errMsg + ')' ) ;
         }
      }
      return objRet ;
   }
}

// execute
   main() ;


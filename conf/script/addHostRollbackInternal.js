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
*/
if ( typeof(IP) == "undefined" ) {}
if ( typeof(USERNAME) == "undefined" ) {}
if ( typeof(PASSWORD) == "undefined" ) {}
if ( typeof(INSTALL_PATH) == "undefined" ) {}

// linux
var UNISTALL_L = "uninstall" ;
// windows
var UNISTALL_W = "uninstall.exe" ;

var objRet = new Object() ;
objRet.Rc         = 0 ;
objRet.Detail     = "" ;

function checkAndModifyThePath( osInfo, path )
{
   var s = "" ;
   var i = -1 ;
   if ( "LINUX" == osInfo )
   {
      s = "/" ; 
   }
   else
   {
      s = "\\" ;
   }
   i = path.indexOf( s ) ;
   if ( (i != -1) && (i != path.length) )
      path += s ; 
   return path ;
}

function uninstallPacket ( ssh, osInfo )
{
   var cmd = "" ;
   var path = checkAndModifyThePath( osInfo, INSTALL_PATH ) ;
   if ( "LINUX" == osInfo )
   {
      cmd = path + UNISTALL_L ;
      ssh.exec( "chmod a+x " + cmd ) ;
      ssh.exec( cmd + " --mode unattended " ) ;
   }
   else
   {
      // DOTO: tanzhaobo
      // windows
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
      if ( typeof(INSTALL_PATH) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "not specified installation path in remote host" ;
         return objRet ;
      }
      // get os info
      var osInfo = System.type() ;
      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      // uninstall business packet from remote host
      uninstallPacket( ssh, osInfo ) ;
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


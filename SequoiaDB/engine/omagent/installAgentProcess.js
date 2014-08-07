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

if ( typeof(USERNAME) == "undefined" ) { USERNAME = "" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "" ; }
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }
// todo: modify default value
// *********************************************************
if ( typeof(LOCAL_PACKET_PATH) == "undefined" ) { LOCAL_PACKET_PATH = "/home/users/tanzhaobo/sequoiadb/bin/sdb" ; }
if ( typeof(REMOTE_PACKET_PATH) == "undefined" ) { REMOTE_PACKET_PATH = "/tmp/sdb" ; }


var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;
objRet.HasPush = false ;
objRet.HasRunning = false ;
objRet.HostName = "" ;

function main()
{
   try
   {
      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      objRet.HostName = ssh.exec("hostname") ;
      // push packet
      ssh.push( LOCAL_PACKET_PATH, REMOTE_PACKET_PATH ) ;
      objRet.HasPush = true ;
      // start the process
      var sysType = System.type() ;
      var str = "" ;
      if ( sysType == "LINUX" )
      {
         str = " 1>/dev/null 2>&1 & disown"
         ssh.exec( REMOTE_PACKET_PATH + str ) ;
//println("////////////" + REMOTE_PACKET_PATH + str) ;
      }
      else
      {
         str = "start /b"
         ssh.exec( str + REMOTE_PACKET_PATH ) ;
      }
      objRet.HasRunning = true ;

      return objRet ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = getLastErrMsg() ;
      return objRet ;
   }
}

// execute
   main() ;


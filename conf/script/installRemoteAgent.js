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
if ( typeof(LOCAL_CM_PROG) == "undefined" ) {}
if ( typeof(REMOTE_CM_PROG) == "undefined" ) {}

var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;
objRet.HasPush = false ;
objRet.HasRunning = false ;
objRet.HostName = "" ;

function main()
{
print("username is: " + USERNAME + "\n" ) ;
print("password is: " + PASSWORD + "\n" ) ;
print("ip is: " + IP + "\n" ) ;
print("local cm prog is: " + LOCAL_CM_PROG + "\n" ) ;
print("remote cm prog is: " + REMOTE_CM_PROG + "\n" ) ;
   try
   {
      // check argument
      if ( typeof(USERNAME) == "undefined"      ||
           typeof(PASSWORD) == "undefined"      ||
           typeof(IP) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "user name, password and ip, but some of them are not defined" ;
         return objRet ;
      }
      if ( typeof(LOCAL_CM_PROG) == "undefined" ||
           typeof(REMOTE_CM_PROG) == "undefined" )
      {
         objRet.Rc = -10 ;
         objRet.Detail = "no local sdbcm packet or remote sdbcm packet" ;
         return objRet ;
      }
print("111111111111111111111111111111111111111111111\n") ;
      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      objRet.HostName = ssh.exec("hostname") ;
      // push packet
      ssh.push( LOCAL_CM_PROG, REMOTE_CM_PROG ) ;
      objRet.HasPush = true ;
print("22222222222222222222222222222222222222222222222\n") ;
      // start the omagent process
      var sysType = System.type() ;
      var str = "" ;
      if ( sysType == "LINUX" )
      {
         str = " 1>/dev/null 2>&1 & disown"
         ssh.exec(  + str ) ;
//println("////////////" + REMOTE_PACKET_PATH + str) ;
      }
      else
      {
         str = "start /b"
         ssh.exec( str + REMOTE_CM_PROG ) ;
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


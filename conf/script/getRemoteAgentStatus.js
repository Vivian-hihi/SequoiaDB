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
@description: check whether remote omagent is running,
              if so, get it's version
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/
if ( typeof(USERNAME) == "undefined" ) {}
if ( typeof(PASSWORD) == "undefined" ) {}
if ( typeof(IP) == "undefined" ) {}
if ( typeof(TIMES) == "undefined" ) {}

var objRet = new Object() ;
objRet.IsRunning     = false ;
objRet.Version       = "" ;

objRet.Rc            = 0 ;
objRet.Detail        = "" ;


// TODO: not finish yet
function getRemoteSdbcmStatus( ssh, osInfo )
{

      // test whether omagent is running in remote mechine
      var cmd1 = "ps -e | grep sequoiadb" ;
      var ret1 = ssh.exec( cmd1 ) ;
print("IIIIIIIIIIIIIIIIIIIIIIIIII\n") ;
println( "ret1 is: " + ret1 );
      if ( "" != ret1 )
      {
         objRet.IsRunning = true ;
         // get the process id of remote omagent
         var cmd2 = "ps -ef | grep sequoiadb | grep -v grep | awk '{print \"/proc/\"$2\"/exe\"}' | xargs ls -l | head -1 | awk 'BEGIN{FS=\"-> \"} {print $2}'" ;
         var ret2 = ssh.exec( cmd2 ) ;
println( "ret2 is: " + ret2 ) ;
         // get the version of remote omagent
         var cmd3 = ret2 + "--version | grep \"SequoiaDB version\" | awk 'BEGIN{FS=\": \"} {print $2}'" ;
         var ret3 = ssh.exec( cmd3 ) ;
println( "ret3 is: " + ret3 ) ;
         if ( "" != ret3 )
         {
            objRet.Version = ret3 ;
         }
      }


}

function main()
{
   var ssh = null ;
   var osInfo = null ;

   try
   {
      // check arguments
      if ( typeof(USERNAME) == "undefined" ||
           typeof(PASSWORD) == "undefined" ||
           typeof(IP)       == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "not specified user name, password or ip" ;
         return objRet ;
      }
      // ssh
      ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      // os infomation
      osInfo = System.type() ;
      // get remote sdbcm status
      getRemoteSdbcmStatus( ssh, osInfo ) ;
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


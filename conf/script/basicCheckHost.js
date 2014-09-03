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
@description: check wether localhost can ping and ssh to remote host
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/

if ( typeof(USERNAME) == "undefined" ) {}
if ( typeof(PASSWORD) == "undefined" ) {}
if ( typeof(IP) == "undefined" ) {}
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }

var objRet = new Object() ;
objRet.Ping = false ;
objRet.Ssh = false ;
objRet.Rc = 0 ;
objRet.Detail = "" ;

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
         objRet.Detail = "user name, password or ip is not defined" ;
         return objRet ;
      }

      // ping
      var ret = System.ping( IP, TIMES ) ;
      var ping = eval( "(" + ret + ")" ) ;
      if ( true != ping.Reachable )
         return objRet ;
      objRet.Ping = true ;

      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      if ( null != typeof(ssh) && "undefined" != typeof(ssh) )
         objRet.Ssh = true ;

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


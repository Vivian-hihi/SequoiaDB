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
@description: scan host
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
objRet.HostName = null ;
objRet.Rc = 0 ;
objRet.Detail = "" ;

function main()
{
   try
   {
      // check argument
      if ( typeof(USERNAME) == "undefined" ||
           typeof(PASSWORD) == "undefined" ||
           typeof(IP) == "undefined" ) )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "user name, password or ip is not defined" ;
         return objRet ;
      }

      // ping
      var ping = System.ping( IP, TIMES ) ;
      if ( null != typeof(ping) && "undefined" != typeof(ping) )
         objRet.Ping = true ;

      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      if ( null != typeof(ssh) && "undefined" != typeof(ssh) )
         objRet.Ssh = true ;

      // hostName
      var name = ssh.exec("hostname") ; // if no host name, what can I do ?

      if ( null != typeof(name) && "undefined" != typeof(name) )
      {
         var i = name.indexOf( "\n" ) ;
         var substr = name.substring(0, i);
         objRet.HostName = substr ;
      }
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


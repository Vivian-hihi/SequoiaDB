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
@description: get host name
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "" ; }
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }

var objRet = new Object() ;

objRet.HostName   = null ;
objRet.Rc         = 0 ;
objRet.Detail     = null ;

function main()
{
   // ssh and get host name
   try
   {
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      var name = ssh.exec("hostname") ;
      var i = name.indexOf( "\n" ) ;
      var substr = name.substring(0, i);
      objRet.HostName = substr ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }
   return objRet ;
}

// execute
main() ;


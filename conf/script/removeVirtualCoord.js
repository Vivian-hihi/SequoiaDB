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
@description: remove virtual coord
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/

if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(USERNAME) == "undefined" ) {}
if ( typeof(PASSWORD) == "undefined" ) {}
// todo::add path in windows
if ( typeof(PROGRAM) == "undefined" ) { PROGRAM_PATH = "/opt/sequoiadb/bin/sdbstop" ; }
//if ( typeof(CONFIG_PATH) == "undefined" ) { CONFIG_PATH = "/tmp/virtualCoord" ; }
//if ( typeof(CONFIG_FILE) == "undefined" ) { CONFIG_FILE = "/tmp/virtualCoord/sdb.conf" ; }
if ( typeof(DB_PATH) == "undefined" ) { CONFIG = "/tmp/virtualCoord" ; }
if ( typeof(COORD_SERVICE) == "undefined" ) { COORD_SERVICE = "11810" ; }
var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;

function main()
{
   try
   {
      // check arguments
      if ( typeof(USERNAME) == "undefined" ||
          typeof(PASSWORD) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "Invalid arguments" ;
         return objRet ;
      }

      // ssh to local
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      // stop virtual coord
      var cmd = PROGRAM + " -p " + COORD_SERVICE ;
      ssh.exec( cmd ) ;
      // remove rubbish files
      File.remove( CONFIG_PATH ) ;
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


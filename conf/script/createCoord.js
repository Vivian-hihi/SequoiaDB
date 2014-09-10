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
@description: create coord
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/
// todo:windows
if ( typeof(COORD_HOSTNAME) == "undefined" ) { HOSTNAME = "localhost" ; }
if ( typeof(COORD_SERVICE) == "undefined" ) { SERVICE = "11810" ; }
if ( typeof(DB_USERNAME) == "undefined" ) { SERVICE = "" ; }
if ( typeof(DB_PASSWORD) == "undefined" ) { SERVICE = "" ; }
if ( typeof(INSTALL_HOSTNAME) == "undefined" ) { HOSTNAME = "localhost" ; }
if ( typeof(INSTALL_SERVICE) == "undefined" ) { SERVICE = "11810" ; }
if ( typeof(INSTALL_PATH) == "undefined" ) { INSTALL_PATH = "/opt/sequoiadb/database/coord/11810" ; }
if ( typeof(CONFIG) == "undefined" ) { CONFIG = "{}" ; }

var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.detail = "" ;

function main()
{
   try
   {
      // connect to coord
      var db = new Sdb( COORD_HOSTNAME, COORD_SERVICE, DB_USERNAME, DB_PASSWORD ) ;
      // create cataRG
      var coord = db.createCoord( INSTALL_HOSTNAME, INSTALL_SERVICE,
                                  INSTALL_PATH, CONFIG ) ;
      return objRet ;
   }
   catch ( e )
   {
      if ( typeof(e) != "number" )
      {
         objRet.Rc = -10 ;
         objRet.detail = "system error" ;
      }
      else
      {
         var errMsg = "" ;
         objRet.Rc = e ;
         errMsg = getLastErrMsg() ;
         if ( "" != errMsg && null != errMsg && undefined != errMsg )
         {
            objRet.detail = eval( '(' + errMsg + ')' ) ;
         }
      }
      return objRet ;
   }
}

// execute
   main() ;


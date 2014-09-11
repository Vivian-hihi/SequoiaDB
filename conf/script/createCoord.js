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
if ( typeof(COORD_HOSTNAME) == "undefined" ) { COORD_HOSTNAME = "127.0.0.1" ; }
if ( typeof(COORD_SERVICE) == "undefined" ) { COORD_SERVICE = "10000" ; }
if ( typeof(DB_USERNAME) == "undefined" ) { DB_USERNAME = "" ; }
if ( typeof(DB_PASSWORD) == "undefined" ) { DB_PASSWORD = "" ; }
if ( typeof(INSTALL_HOSTNAME) == "undefined" ) {}
if ( typeof(INSTALL_SERVICE) == "undefined" ) {}
if ( typeof(INSTALL_PATH) == "undefined" ) {}
if ( typeof(CONFIG) == "undefined" ) { CONFIG = eval( '(' + '{}' + ')' ) ; }

var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.detail = "" ;

function createCoordNode( db )
{
   var rg = null ;
   var node = null ;
   try
   {
      rg = db.getRG("SYSCoord") ;
   }
   catch ( e )
   {
      if ( -154 == e )
      {
         // create coord replica group
         rg = db = db.createCoordRG() ;
      }
      else
      {
         throw e ;
      }
   }
   // create and start coord node
   node = rg.createNode( INSTALL_HOSTNAME, INSTALL_SERVICE,
                         INSTALL_PATH, CONFIG ) ;
   node.start() ;
}

function main()
{
   var db = null ;
   try
   {
      // check arguments
      if ( typeof(COORD_HOSTNAME) == "undefined" ||
           typeof(COORD_SERVICE) == "undefined"  ||
           typeof(DB_USERNAME) == "undefined"    ||
           typeof(DB_PASSWORD) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.detail = "not specified hostname, svcname"
                         + " username or password to connect to database" ;
      }
      if ( typeof(INSTALL_HOSTNAME) == "undefined" ||
           typeof(INSTALL_SERVICE) == "undefined"  ||
           typeof(INSTALL_PATH) == "undefined"     ||
           typeof(CONFIG) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.detail = "hostname, svcname, install path and config info" +
                         " are need for create coord" ;
         return objRet ;
      }
      // connect to coord
      db = new Sdb( COORD_HOSTNAME, COORD_SERVICE, DB_USERNAME, DB_PASSWORD ) ;
      // create coord node
      createCoordNode( db ) ;

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


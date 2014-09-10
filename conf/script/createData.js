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
@description: create data node
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/
if ( typeof(INSTALL_HOSTNAME) == "undefined" ) {}
if ( typeof(INSTALL_SERVICE) == "undefined" ) {}
if ( typeof(INSTALL_PATH) == "undefined" ) {}
if ( typeof(CONFIG) == "undefined" ) { CONFIG = eval( '(' + '{}' + ')') ; }
if ( typeof(GROUPNAME) == "undefined" ) {}

if ( typeof(COORD_HOSTNAME) == "undefined" ) { COORD_HOSTNAME = "127.0.0.1" ; }
if ( typeof(COORD_SERVICE) == "undefined" ) { COORD_SERVICE = "10000" ; }
if ( typeof(DB_USERNAME) == "undefined" ) { DB_USERNAME = "" ; }
if ( typeof(DB_PASSWORD) == "undefined" ) { DB_PASSWORD = "" ; }

var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.detail = "" ;

function main()
{
   try
   {
      // check arguments
      if ( typeof(INSTALL_HOSTNAME) == "undefined" ||
           typeof(INSTALL_SERVICE) == "undefined" ||
           typeof(INSTALL_PATH) == "undefined" ||
           typeof(GROUPNAME) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.detail = "install hostname, svcname, path and the groupname are"
                         + " need for creating data node" ;
         return objRet ;
      }
// todo: remove this debug info
print("INSTALL_HOSTNAME is: " + INSTALL_HOSTNAME + '\n') ;
print("INSTALL_SERVICE is: " + INSTALL_SERVICE + '\n') ;
print("INSTALL_PATH is: " + INSTALL_PATH + '\n') ;
print("CONFIG is: " + CONFIG + '\n') ;
print("GROUPNAME is: " + GROUPNAME + '\n') ;
      // connect to virtual coord
      var db = new Sdb( COORD_HOSTNAME, COORD_SERVICE, DB_USERNAME, DB_PASSWORD ) ;
print("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
      // get rg
      var rg = null ;
      try
      {
         rg = db.getRG( GROUPNAME ) ;
print("***********************************************8\n") ;
      }
      catch ( e )
      {
         if ( -154 == e )
         {
            // when rg not exit, create it
            try
            {
println("((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))\n") ;
               rg = db.createRG( GROUPNAME ) ;
            }
            catch ( e )
            {
              throw e ;
            }
         }
         else
         {
            throw e ;
         }
      }
      // create data node
      var node  = rg.createNode( INSTALL_HOSTNAME, INSTALL_SERVICE,
                                 INSTALL_PATH, CONFIG ) ;
      // start node
      node.start() ;
// todo: maybe need to check whether the node has been start or not
      return objRet ;
   }
   catch ( e )
   {
//print("error is e = " + e + "\n") ;
      if ( typeof(e) != "number" )
      {
         objRet.Rc = -10 ;
         objRet.detail = "system error" ;
      }
      else
      {
         var errMsg = "" ;
         objRet.Rc = e ;
         // TODO: getLastErrMsg may return nothing
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


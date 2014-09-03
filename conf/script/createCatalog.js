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
@description: create catalog
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/
if ( typeof(COORD_HOSTNAME) == "undefined" ) { COORD_HOSTNAME = "127.0.0.1" ; }
if ( typeof(COORD_SERVICE) == "undefined" ) { COORD_SERVICE = "13579" ; }
if ( typeof(DB_USERNAME) == "undefined" ) { DB_USERNAME = "" ; }
if ( typeof(DB_PASSWORD) == "undefined" ) { DB_PASSWORD = "" ; }
if ( typeof(INSTALL_HOSTNAME) == "undefined" ) {}
if ( typeof(INSTALL_SERVICE) == "undefined" ) {}
if ( typeof(INSTALL_PATH) == "undefined" ) {}
if ( typeof(typeof(CONFIG) == "undefined") )
{  
   CONFIG = eval( '(' + '{}' + ')') ;
}

var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;

function main()
{
   try
   {
      // check arguments
      if ( typeof(COORD_HOSTNAME) == "undefined" ||
           typeof(COORD_SERVICE) == "undefined" ||
           typeof(DB_USERNAME) == "undefined" ||
           typeof(DB_PASSWORD) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "not specified hostname, svcname"
                         + " username or password to connect to database" ;
      }
      if ( typeof(INSTALL_HOSTNAME) == "undefined" ||
           typeof(INSTALL_SERVICE) == "undefined" ||
           typeof(INSTALL_PATH) == "undefined" ||
           typeof(CONFIG) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "hostname, svcname, install path and config info" +
                         " are need for create catalog" ;
         return objRet ;
      }
print("COORD_HOSTNAME is: " + COORD_HOSTNAME + '\n') ;
print("COORD_SERVICE is: " + COORD_SERVICE + '\n') ;
print("DB_USERNAME is: " + DB_USERNAME + '\n') ;
print("DB_PASSWORD is: " + DB_PASSWORD + '\n') ;

      // connect to virtual coord
      var db = new Sdb( COORD_HOSTNAME, COORD_SERVICE, DB_USERNAME, DB_PASSWORD ) ;
print("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n") ;
      // try to get system catalog group
      var rg = null ;
      try
      {
         rg = db.getRG("SYSCatalogGroup") ;
      }
      // catalog has not been created
      catch ( e )
      {
         if ( -180 == e )
         {
            try
            {
               rg = db.createCataRG( INSTALL_HOSTNAME, INSTALL_SERVICE,
                                     INSTALL_PATH, CONFIG ) ;
            }
            catch ( e )
            {
               throw e ;
            }
            return objRet ;
         }
         else
         {
            throw e ;
         }
      }
      // catalog has been created
      var node = rg.createNode( INSTALL_HOSTNAME, INSTALL_SERVICE,
                                INSTALL_PATH, CONFIG ) ;
      node.start() ;

print("hahahhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh ok !!!\n") ;
      return objRet ;
   }
   catch ( e )
   {
print("error is e = " + e + "\n" ) ;
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
         if ( ( "" != errMsg ) && ( null != errMsg ) && ( undefined != errMsg ) )
         {
            objRet.Detail = eval( '(' + errMsg + ')' ) ;
         }
      }
print("Errmsg is: " + objRet.Detail + "\n") ;
      return objRet ;
   }
}

// execute
   main() ;


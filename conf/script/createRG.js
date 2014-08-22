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
@description: create replica group
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/
if ( typeof(COORD_HOSTNAME) == "undefined" ) {}
if ( typeof(COORD_SERVICE) == "undefined" ) {}
if ( typeof(DB_USERNAME) == "undefined" ) {}
if ( typeof(DB_PASSWORD) == "undefined" ) {}
if ( typeof(GROUPNAME) == "undefined" ) {}

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
           typeof(DB_PASSWORD) == "undefined" ||
           typeof(GROUPNAME) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "Invalid argument" ;
         return objRet ;
      }
      // connect to coord
      var db = new Sdb( COORD_HOSTNAME, COORD_SERVICE, DB_USERNAME, DB_PASSWORD ) ;
      // create replica group
      var rg = db.createRG( GROUPNAME ) ;
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


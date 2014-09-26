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
@description: remove the newly created data group
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/
if ( typeof(COORD_HOSTNAME) == "undefined" ) {}
if ( typeof(COORD_SERVICE) == "undefined" ) {}
// TODO: username and password are need
if ( typeof(DB_USERNAME) == "undefined" ) { DB_USERNAME = "" ; }
if ( typeof(DB_PASSWORD) == "undefined" ) { DB_PASSWORD = "" ; }
if ( typeof(CREATED_DATA_GROUP) == "undefined" ) {}

var objRet = new Object() ;

objRet.Errno = 0 ;
objRet.detail = "" ;

function removeGroup( db, name )
{
   var rg = null ;
   
   try
   {
      rg = db.getRG( name ) ;
   }
   catch ( e )
   {
      if ( -154 == e )
      {
         return ;
      }
      else
      {
         throw e ;
      }
   }
   // stop all the data node in this group
   rg.stop() ;
   // remove data group
   db.removeRG( name ) ;
}

function removeDataGroup( db, groups )
{
   var arr = null ;

   arr = groups["GroupName"] ;
   for ( var i = 0; i < arr.length; i++ )
   {
      removeGroup( db, arr[i] ) ;
   }
}

function main()
{
   var db = null ;
   var groups = null ;
   try
   {
      // check arguments
      if ( typeof(COORD_HOSTNAME) == "undefined" ||
           typeof(COORD_SERVICE) == "undefined" )
      {
         objRet.Errno = -6 ;
         objRet.detail = "virtual coord hostname and svcname are need"
         return objRet ;
      }
      if ( typeof(CREATED_DATA_GROUP) == "undefined" )
      {
         objRet.Errno = -6 ;
         objRet.detail = "not specified data group to remove"
         return objRet ;
      }

      // connect to virtual coord
      var db = new Sdb( COORD_HOSTNAME, COORD_SERVICE, DB_USERNAME, DB_PASSWORD ) ;
      // get groups info
      groups = eval( '(' + CREATED_DATA_GROUP + ')' ) ;
      // remove data group
      removeDataGroup( db, groups ) ;

      return objRet ;
   }
   catch ( e )
   {
      if ( typeof(e) != "number" )
      {
         objRet.Errno = -10 ;
         objRet.detail = "system error" ;
      }
      else
      {
         var errMsg = "" ;
         objRet.Errno = e ;
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


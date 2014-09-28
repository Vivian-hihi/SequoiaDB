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
@description: remove the newly created catalog group
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter
   BUS_JSON:
   SYS_JSON: the format is: { "VCoordSvcName": "11792" }
   ENV_JSON:
@return
   RET_JSON: the format is: {"errno":0,"detail":""}
*/

//var SYS_JSON = { "VCoordSvcName": "11792" } ;

var RET_JSON     = new Object() ;
RET_JSON[Errno]  = SDB_OK ;
RET_JSON[Detail] = "" ;

/* *****************************************************************************
@discretion: remove catalog group
@parameter
   db[object]: Sdb object
@return void
***************************************************************************** */
function removeCatalogGroup( db )
{
   var rg = null ;

   try
   {
      rg = db.getCatalogRG() ;
   }
   catch ( e )
   {
      if ( SDB_CLS_GRP_NOT_EXIST == e )
      {
         return ;
      }
      else
      {
         if ( "number" == typeof(e) )
         {
            setLastErrMsg( "Failed to get catalog group: " + getErr( e ) ) ;
            setLastError( e ) ;
         }
         throw e ;
      }
   }
   // remove
   try
   {
      db.removeCatalogRG() ;
   }
   catch ( e )
   {
      if ( "number" == typeof(e) )
      {
         setLastErrMsg( "Failed to remove catalog group: " + getErr( e ) ) ;
         setLastError( e ) ;
      }
      throw e ;
   }
}

function main()
{
   var vCoordHostName   = System.getHostName() ;
   var vCoordSvcName    = SYS_JSON[VCoordSvcName] ;
   // connect to virtual coord
   db = new Sdb( vCoordHostName, vCoordSvcName, "", "" ) ;
   // remove coord nodes
   removeCatalogGroup( db ) ;

//print("RET_JSON is: " + JSON.stringify(RET_JSON) + "\n") ;
   return RET_JSON ;
}

// execute
   main() ;


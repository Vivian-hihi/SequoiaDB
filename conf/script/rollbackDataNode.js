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
@parameter
   BUS_JSON: the format is:  
   SYS_JSON: the format is: { "VCoordSvcName": "11792", "InstallGroupNames": [ "group1", "group2" ] }
   ENV_JSON:
@return
   RET_JSON: the format is: {"errno":0,"detail":""}
*/

//var SYS_JSON = { "VCoordSvcName": "11792", "InstallGroupNames": [ "group3" ] } ;

var RET_JSON     = new Object() ;
RET_JSON[Errno]  = SDB_OK ;
RET_JSON[Detail] = "" ;

/* *****************************************************************************
@discretion: remove data group
@parameter
   db[object]: Sdb object
   name[string]: data group name
@return void
***************************************************************************** */
function removeGroup( db, name )
{
   var rg = null ;
   // get rg
   try
   {
      rg = db.getRG( name ) ;
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
            setLastErrMsg( "Failed to get group[" + name + "]: " + getErr( e ) ) ;
            setLastError( e ) ;
         }
         throw e ;
      }
   }
   // stop all the data node in this group
   try
   {
      rg.stop() ;
   }
   catch ( e )
   {
      if ( "number" == typeof(e) )
      {
         setLastErrMsg( "Failed to stop group[" + name + "]: " + getErr( e ) ) ;
         setLastError( e ) ;
      }
      throw e ;
   } 
   // remove data group
   try
   {
      db.removeRG( name ) ;
   }
   catch ( e )
   {
      if ( "number" == typeof(e) )
      {
         setLastErrMsg( "Failed to remove group[" + name + "]: " + getErr( e ) ) ;
         setLastError( e ) ;
      }
      throw e ;
   }
}

function removeDataGroup( db, groups )
{
   for ( var i = 0; i < groups.length; i++ )
   {
      removeGroup( db, groups[i] ) ;
   }
}

function main()
{
   var vCoordHostName   = System.getHostName() ;
   var vCoordSvcName    = SYS_JSON[VCoordSvcName] ;
   var groups           = SYS_JSON[InstallGroupNames] ;

   // connect to virtual coord
   var db = new Sdb( vCoordHostName, vCoordSvcName, "", "" ) ;
   // remove data group
   removeDataGroup( db, groups ) ;
print("RET_JSON is: " + JSON.stringify(RET_JSON) + "\n") ;
   return RET_JSON ;

}

// execute
   main() ;


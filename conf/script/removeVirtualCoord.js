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
@parameter
   SYS_JSON: the format is: { "VCoordSvcName": "11792" }
   ENV_JSON:
@return
   RET_JSON: the format is: {"errno":0,"detail":""}
*/

//var SYS_JSON = { "VCoordSvcName": "11792" } ;

var RET_JSON     = new Object() ;
RET_JSON[Errno]  = SDB_OK ;
RET_JSON[Detail] = "" ;

function main()
{
   var omaHostName   = System.getHostName() ;
   var omaSvcName    = Oma.getAOmaSvcName( "localhost" ) ;
   var vCoordSvcName = SYS_JSON[VCoordSvcName] ;
   var oma = null ;
   try
   {
      // new oma object
      var oma = new Oma( omaHostName, omaSvcName ) ;
      
      // stop virtual coord
      oma.stopNode( vCoordSvcName ) ;
   
      // remomve virtual coord
      oma.removeCoord( vCoordSvcName ) ;
   
      // close connection
      oma.close() ;
      oma = null ;
   }
   catch ( e )
   {
      if ( null != oma && "undefined" != typeof(oma) )
      {
         try
         {
            oma.close() ;
            oma = null ;
         }
         catch ( e )
         {
         }
      }
      else
      {
         throw e ;
      }
   }
//print("RET_JSON is: " + JSON.stringify(RET_JSON) + "\n") ;
   return RET_JSON ;
}

// execute
   main() ;


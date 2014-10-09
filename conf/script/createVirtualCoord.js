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
@description: create virtual coord
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter void
@return
   RET_JSON: the format is: { "Port", "10001" }
*/

var RET_JSON             = new Object() ;
RET_JSON[VCoordSvcName]  = "" ;

function main()
{
   var oma = null ;
   try
   {
      var osInfo        = System.type() ;
      var omaHostName   = System.getHostName() ;
      var omaSvcName    = Oma.getAOmaSvcName( "localhost" ) ;
      var vCoordSvcName = getAUsablePortFromLocal( osInfo ) + "" ;
      oma               = new Oma( omaHostName, omaSvcName ) ;
      // create virtual coord
      oma.createCoord( vCoordSvcName, OMA_PATH_VCOORD_PATH_L, {} ) ;
      // start virtual coord
      oma.startNode( vCoordSvcName ) ;
      // close connection
      oma.close() ;
      oma = null ;
      RET_JSON[VCoordSvcName]  = vCoordSvcName ;
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
         catch ( e2 )
         {
         }
      }
      throw e ; 
   }
//print("RET_JSON is: " + JSON.stringify(RET_JSON) + "\n") ;
   return RET_JSON ;
}

// execute
   main() ;


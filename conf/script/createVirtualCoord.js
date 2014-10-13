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
@parameter
   BUS_JSON: the format is: { "CataAddr":[{"HostName":"rhel64-test8", "CataSvcName":"11800"}]}
@return
   RET_JSON: the format is: { "Port", "10001" }
*/

//var BUS_JSON = { "CataAddr":[] } ;
//var BUS_JSON = { "CataAddr":[ { "HostName":"rhel64-test8", "CataSvcName":"11803" }, { "HostName":"rhel64-test9", "CataSvcName":"11803" }, { "HostName":"rhel64-test9", "CataSvcName":"11903" } ] } ;

var RET_JSON             = new Object() ;
RET_JSON[VCoordSvcName]  = "" ;

/* *****************************************************************************
@discretion: get catalog address for creating virtual coord
@parameter
   addrInfo[json]: address info object
@return
   retObj[json]: the return catalog address, e.g.
                 { "catalogaddr" : "rhel64-test8:11803,rhel64-test9:11803" }
***************************************************************************** */
function getCatalogAddress( addrInfo )
{
   var retObj = new Object() ;
   var infoArr = addrInfo[CataAddr] ;
   var len = infoArr.length ;
   var addr = "" ;
   if ( 0 == len )
   {
      return retObj ;
   }
   for( var i = 0; i < len; i++ )
   {
      var obj = infoArr[i] ;
      var hostname = obj[HostName] ;
      var svcname = obj[CataSvcName] ;
      if ( 0 == i )
      {
         addr = hostname + ":" + svcname ;
      }
      else
      {
         addr += "," + hostname + ":" + svcname ;
      }
   }
   retObj[CatalogAddr] = addr ;
   return retObj ;
}

function main()
{
   var oma = null ;
   try
   {
      var osInfo        = System.type() ;
      var omaHostName   = System.getHostName() ;
      var omaSvcName    = Oma.getAOmaSvcName( "localhost" ) ;
      var vCoordSvcName = getAUsablePortFromLocal( osInfo ) + "" ;
      var cataAddr      = getCatalogAddress( BUS_JSON ) ;
println("cataAddr is: " + JSON.stringify(cataAddr) ) ;
      var dataPath      = OMA_PATH_VCOORD_PATH_L + vCoordSvcName ;
      oma               = new Oma( omaHostName, omaSvcName ) ;
// catalogaddr=rhel64-test8:11803,rhel64-test9:11803,rhel64-test9:11903
      // create virtual coord
      oma.createCoord( vCoordSvcName, dataPath, cataAddr ) ;
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


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
@description: remove standalone
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter
   BUS_JSON: the format is: { "HostInfo" : [ { "UninstallHostName": "rhel64-test8", "UninstallSvcName": "11820" } ] }
@return
   RET_JSON: the format is: {}
*/

//var BUS_JSON = { "HostInfo" : [ { "UninstallHostName": "rhel64-test8", "UninstallSvcName": "11820" } ] } ;

var RET_JSON = new Object() ;

/* *****************************************************************************
@discretion remove standalone
@parameter
   hostName[string]: uninstall host name
   svcName[string]: uninstall svc name
   agentPort[string]: the port of sdbcm in standalone host
@return void
***************************************************************************** */
function removeStandalone( hostName, svcName, agentPort )
{
   var oma = new Oma( hostName, agentPort ) ;
   try
   {
      oma.removeData( svcName ) ;
      oma.close() ;
      oma.null ;
   }
   catch ( e ) 
   {
      if ( null != oma && "undefined" != typeof(oma) )
      {
         try
         {
            oma.close() ;
         }
         catch ( e2 )
         {
         }
      }
      if ( "number" == typeof(e) && e < 0 )
      {
         setLastErrMsg( "Failed to remove standalone: " + getErr(e) ) ;
         setLastError( e )
         throw e ;
      }
      else
      {
         setLastErrMsg( "Failed to remove standalone: " + getLastErrMsg() ) ;
         setLastError( SDB_SYS ) ;
         throw SDB_SYS ;
      }
   }
}

function main()
{
   var infoArr = BUS_JSON[HostInfo] ;
   var arrLen = infoArr.length ;
   if ( 0 == arrLen )
   {
      return RET_JSON ;
   }
   for ( var i = 0; i < arrLen; i++ )
   {
      var obj = infoArr[i] ;
      var uninstallHostName   = obj[UninstallHostName] ;
      var uninstallSvcName    = obj[UninstallSvcName] ;
      var agentPort           = getAgentPort( uninstallHostName ) ;
      
      removeStandalone( uninstallHostName, uninstallSvcName, agentPort ) ;
      break ;
   }
   return RET_JSON ;
}

// execute
   main() ;


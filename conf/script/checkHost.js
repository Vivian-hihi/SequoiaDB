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
@description: get host info
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/
if ( typeof(USERNAME) == "undefined" ) {}
if ( typeof(PASSWORD) == "undefined" ) {}
if ( typeof(HOSTNAME) == "undefined" ) {}
if ( typeof(IP) == "undefined" ) {}
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }

var objRet = new Object() ;
objRet.HostName   = null ;
//objRet.IP         = null ;
objRet.User       = null ;
objRet.Password   = null ;
 
objRet.HostsTable = null ;
objRet.OS         = null ;
objRet.CPU        = null ;
objRet.Memory     = null ;
objRet.Disk       = null ;
objRet.Net        = null ;
objRet.Port       = null ;
objRet.Service    = null ;
objRet.OM         = null ;
objRet.Safety     = null ;

objRet.HostName   = null ;
objRet.Rc         = 0 ;
objRet.Detail     = null ;
/*
// hostname
function getHostName( ssh )
{
   var name = ssh.exec( "hostname" ) ;
   var i = name.indexOf( "\n" ) ;
   var substr = name.substring(0, i);
   objRet.HostName = substr ;
}
*/
// hosts table
function getHostsTableInfo()
{
   var hostsInfo = eval( '(' + System.getHostsMap() + ')' ) ;
   objRet.HostsTable = hostsInfo ;
}

// os info
function getOSInfo()
{
   var osInfo = eval( '(' + System.getReleaseInfo() + ')' ) ;
   objRet.OS = osInfo ;
}

// cpu
function getCPUInfo()
{
   var cpuInfo = eval( '(' + System.getCpuInfo() + ')' ) ;
   objRet.CPU = cpuInfo ;
}

// memory
function getMemInfo()
{
   var memInfo = eval( '(' + System.getMemInfo() + ')' ) ;
   objRet.Memory = memInfo ;
}

// disk
function getDiskInfo()
{
   var diskInfo = eval( '(' + System.getDiskInfo() + ')' ) ;
   objRet.Disk = diskInfo ;
}

// net card
function getNetCardInfo()
{
   var netcardInfo = eval( '(' + System.getNetcardInfo() + ')' ) ;
   objRet.Net = netcardInfo ;
}

// port status 
// servic
// om status and version

// safety
function getSafetyInfo()
{
   var safetyInfo = eval( '(' + System.getIpTablesInfo() + ')' ) ;
   objRet.Safety = safetyInfo ;
}

function main()
{
   try
   {
      // check argument
      if ( typeof(USERNAME) == "undefined" ||
           typeof(PASSWORD) == "undefined" ||
           typeof(IP) == "undefined" ||
           typeof(HOSTNAME) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "not specified username, password or ip" ;
         return objRet ;
      }
      objRet.HostName   = HOSTNAME ;
//      objRet.IP         = IP ;
      objRet.User       = USERNAME ;
      objRet.Password   = PASSWORD ;

      // get local host info
      getHostsTableInfo()
      getOSInfo()
      getCPUInfo()
      getMemInfo()
      getDiskInfo()
      getNetCardInfo()
      getSafetyInfo()
      // TODO: tanzhaobo
      objRet.Port       = eval( '(' + '{}' + ')' ) ;
      objRet.Service    = eval( '(' + '{}' + ')' ) ;
      objRet.OM         = eval( '(' + '{}' + ')' ) ;
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
   return objRet ;
}

// execute
main() ;


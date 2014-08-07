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
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "" ; }
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }

var objRet = new Object() ;
objRet.HostsTable = null ;
objRet.OS         = null ;
objRet.Cpu        = null ;
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

function main()
{
   // ssh and get host name
   // todo: this step is no need for this file
   try
   {
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      var name = ssh.exec("hostname") ;
      var i = name.indexOf( "\n" ) ;
      var substr = name.substring(0, i);
      objRet.HostName = substr ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }
   // hosts
   try
   {
      var hostsInfo = eval( '(' + System.getHostsMap() + ')' ) ;
      objRet.HostsTable = hostsInfo ;
println( "hosts map is: " + System.getHostsMap() + "\n" ) ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }

   // Os release info
   try
   {
      var osInfo = eval( '(' + System.getReleaseInfo() + ')' ) ;
      objRet.OS = osInfo ;
println( "OS info: " + System.getReleaseInfo() ) ;
print("\n") ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }
   // cpu
   try
   {
      var cpuInfo = eval( '(' + System.getCpuInfo() + ')' ) ;
      objRet.Cpu = cpuInfo ;
println( "CPU info: " + System.getCpuInfo() ) ;
print("\n") ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }
   // memory
   try
   {
      var memInfo = eval( '(' + System.getMemInfo() + ')' ) ;
      objRet.Memery = memInfo ;
println( "Memory info: " + System.getMemInfo() + "\n" ) ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }
   // disk
   try
   {
      var diskInfo = eval( '(' + System.getDiskInfo() + ')' ) ;
      objRet.Disk = diskInfo ;
println( "Disk info: " + System.getDiskInfo() + "\n" ) ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }
   // net card
   try
   {
      var netcardInfo = eval( '(' + System.getNetcardInfo() + ')' ) ;
      objRet.Net = netcardInfo ;
println( "Net card info: " + System.getNetcardInfo() + "\n" ) ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }
/*
   // port status
   try
   {
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
println( "Net card infoL " + netCardInfo + "\n" ) ;
   }

   // service
   try
   {
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }

   // om
   try
   {
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }
*/
   // safety
   try
   {
      var safetyInfo = eval( '(' + System.getIpTablesInfo() + ')' ) ;
      objRet.Safety = safetyInfo ;
println( "Safety info: " + System.getIpTablesInfo() + "\n" ) ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }
   return objRet ;
}

// execute
main() ;


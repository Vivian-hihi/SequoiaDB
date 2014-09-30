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
@description: update host info in local host table
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter
   BUS_JSON: the format is:
   SYS_JSON: the format is:
   ENV_JSON:
@return
   RET_JSON: the format is: {"errno":0,"detail":""}
*/


var BUS_JSON = {"HostName":"rhel64-test8","IP":"192.168.20.165","User":"root","Passwd":"sequoiadb","HostInfo":[{"HostName":"rhel64-test77","IP":"192.168.20.167","AgentPort":"12345"},{"HostName":"rhel64-test99","IP":"192.168.20.168","AgentPort":"12345"}]} ;


var RET_JSON          = new Object() ;
RET_JSON[Errno]       = 0 ;
RET_JSON[Detail]      = "" ;


/* *****************************************************************************
@discretion: get local db business install path
@author: Tanzhaobo
@parameter
   osInfo[string]: os information, "LINUX" or "WINDOWS"
@return
   installpath[string]: the db business install director
***************************************************************************** */
function getLocalDBInstallPath( osInfo )
{
   var omaInstallInfo = null ;
   var installpath = null ;
   try
   {
      omaInstallInfo = eval( '(' + Oma.getOmaInstallInfo() + ')' ) ;
   }
   catch( e )
   {
      if ( "number" == typeof( e ) )
      {
         setLastErrMsg( "Failed to get oma install info: " + getErr( e ) ) ;
         setLastError( e ) ;
      }
      throw e ;
   }
   installpath = adaptPath ( osInfo, omaInstallInfo[INSTALL_DIR] ) ;
   return installpath ;
}

/* *****************************************************************************
@discretion: update host table
@author: Tanzhaobo
@parameter
   ssh[object]: ssh Object
   osInfo[string]: os information, "LINUX" or "WINDOWS"
   arr[Array]: update info
@return void
***************************************************************************** */
function updateHostInfo( ssh, osInfo, arr )
{
   var installpath = getLocalDBInstallPath( osInfo ) ; 
   if ( OMA_LINUX == osInfo )
   {
      var sdbpath = installpath + OMA_PROG_BIN_SDB_L ;
      var sptpath = installpath + OMA_FILE_UPDATE_HOSTS_L ;
print("sdbpath is: " + sdbpath + "\n")
print("sptpath is: " + sptpath + "\n")
      for ( var i = 0; i < arr.length; i++ )
      {
         var str      = null ;
         var obj      = arr[i] ;
         var hostname = obj[HostName] ;
         var ip       = obj[IP] ;
         str = sdbpath + ' -e ' +  ' \"var HOSTNAME = \\\"' + hostname + '\\\"; var IP = \\\"' + ip + '\\\" ;\" '  + ' -f ' + sptpath ;
print("str is: " + str + "\n")
         try
         {
            ssh.exec( str ) ;
         }
         catch ( e )
         {
print("e is: " + e + "\n") ;
            setLastErrMsg( "Failed to set info [" + ip + "   " + hostname + "] to hosts table in " + ssh.getLocalIP() ) ;
            setLastError( SDB_SYS ) ;
            throw SDB_SYS ;
         }
      }
print("33333333333333\n")
   }
   else
   {
      // TODO: windows
   }
}

/* *****************************************************************************
@discretion: update sdbcm config file
@author: Tanzhaobo
@parameter
   ssh[object]: ssh Object
   osInfo[string]: os information, "LINUX" or "WINDOWS"
   arr[Array]: update info
@return void
***************************************************************************** */
function updateSdbcmCfgFile( ssh, osInfo, arr )
{
   var agentport = null ;
   var hostname  = null ;
//   var installpath = getLocalDBInstallPath( osInfo ) ;
//   var configfile = installpath + OMA_FILE_SDBCM_CONF2_L ;
   var configobj = null ;
   try
   {
      configobj = eval ( '(' + Oma.getOmaConfigs() + ')' ) ;
   }
   catch ( e )
   {
      if ( "number" == typeof( e ) )
      {
         setLastErrMsg( "Failed to get oma config info: " + getErr( e ) ) ;
         setLastErr( e ) ;
      }
      throw e ;
   }
print("444444444444444444444444\n")
   for ( var i = 0; i < arr.length; i++ )
   {
      try
      {
         agentport = arr[i][AgentPort] ;
         hostname  = arr[i][HostName] ;
      }
      catch ( e )
      {
         continue ;
      }
//      var obj = arr[i] ;
      var str = hostname + OMA_MISC_CONFIG_PORT ;
print("config str is: " + str + "\n")
      configobj[str] = agentport ; 
print("obj is: " + JSON.stringify(configobj) + "\n")
   }
print("66666666666666666666666\n")
   try
   {
      Oma.setOmaConfigs( configobj ) ;
   }
   catch ( e )
   {
      if ( "number" == typeof(e) )
      {
         if ( e < 0 )
         {
            setLastErrMsg( "Failed to set oma config info: " + getErr( e ) )
            setLastError( e ) ;
         }
         else
         {
            setLastErrMsg( "Failed to set oma config info" )
            setLastError( SDB_SYS ) ;
            e = SDB_SYS ;
         }
      }
      throw e ;
   }
print("7777777777777777777\n")
}


function main()
{
// {"HostName":"rhel64-test8","IP":"192.168.20.165","User":"root","Passwd":"sequoiadb","HostInfo":[{"HostName":"rhel64-test77","IP":"192.168.20.167","AgentPort":"12345"},{"HostName":"rhel64-test99","IP":"192.168.20.168","AgentPort":"12345"}]}

    var ip               = BUS_JSON[IP] ;
    var user             = BUS_JSON[User] ;
    var passwd           = BUS_JSON[Passwd] ;
    var updateInfoArr    = BUS_JSON[HostInfo] ;
    var osInfo           = null ;
    var ssh              = null ;


   // new ssh
   ssh = new Ssh( ip, user, passwd ) ;
   // get os info
   osInfo = System.type() ;

   // update host table
   updateHostInfo( ssh, osInfo, updateInfoArr ) ;     
print("11111111111111111111111\n")
   // update sdbcm comfig file
   updateSdbcmCfgFile( ssh, osInfo, updateInfoArr ) ;
print("222222222222222\n")
   return RET_JSON ;
}

// execute
main() ;


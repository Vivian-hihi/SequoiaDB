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
@description: register host info to each other
@parameter
   HOSTS_INFO[sting]: hosts info, like {HostInfo:[{"HostName":"h1",
                      "IP":"192.168.20.30", AgentPort:""}, ...], User:"",
                      Passwd:""}
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/

if ( typeof(HOSTS_INFO) == "undefined" ) {}
//if ( typeof(sdbcmCfgFile) == "undefined" ) {}

/*
HOSTS_INFO = "{ \"HostName\": \"rhel64-test8\", \"IP\": \"192.168.20.165\", \"User\": \"root\", \"Passwd\": \"sequoiadb\", \"HostInfo\": [ { \"HostName\": \"rhel64-test77\", \"IP\": \"192.168.20.167\",\"AgentPort\":\"12345\" }, { \"HostName\": \"rhel64-test99\", \"IP\": \"192.168.20.168\" ,\"AgentPort\":\"12345\" } ] }" ;

var localHosts = { "Hosts": [ { "Ip": "127.0.0.1", "HostName": "localhost" }, { "Ip": "192.168.20.42", "HostName": "susetzb" }, { "Ip": "192.168.20.40", "HostName": "ubuntu-dev1" }, { "Ip": "192.168.20.165", "HostName": "rhel64-test8" }, { "Ip": "192.168.20.166", "HostName": "rhel64-test9" } ] } ;

*/

var objRet = new Object() ;

objRet.Rc          = 0 ;
objRet.detail      = "" ;

// globbal ver
var sdbcmCfgFile   = "" ;
var separator      = "   " ;

/* *****************************************************************************
@discretion: get the updated info from a json object to an array
@author: Tanzhaobo
@parameter
   obj[object]: the json object contained the info to update
@return
   retArr[Array]: the array of updated info extract from the json object,
                  [ "192.168.10.10 unbuntu-dev", ...]
***************************************************************************** */
function getUpdateHostsInfo( obj )
{
   var retArr = new Array() ;
   var arr = obj[HostInfo] ;
   var ip = "" ;
   var hostName = "" ;

   for( var i = 0; i < arr.length; i++ )
   {
      ip = arr[i][IP] ;
      hostName = arr[i][HostName] ;
      retArr.push( ip + "   " + hostName ) ;
   }
   return retArr ;   
}

/* *****************************************************************************
@discretion: get the hosts info from local hosts table to an array
@author: Tanzhaobo
@parameter
   obj[object]: the json object contained the local hosts table info
@return
   retArr[Array]: the array of local hosts info,
                  [ "192.168.10.10 unbuntu-dev", ...]
***************************************************************************** */
function getLocalHostsInfo( obj )
{
   var retArr = new Array() ;
   var arr = obj[Hosts] ;
   var ip = "" ;
   var hostName = "" ;

   for( var i = 0; i < arr.length; i++ )
   {
      ip = arr[i][Ip] ;
      hostName = arr[i][HostName] ;
      retArr.push( ip + separator + hostName ) ;
   }
   return retArr ;
}

/* *****************************************************************************
@discretion: remove duplicate "ip hostname" in updated hosts info arr
@author: Tanzhaobo
@parameter
   arr1[Array]: update hosts info array
   arr2[Array]: local hosts info array
@return 
   retArr[Array]: the array after de-weight "ip hostname"
***************************************************************************** */
function deWeight( arr1, arr2 )
{
   var retArr = new Array() ;
   var tmpArr = new Array() ;
   
   tmpArr = singleSideDeWeight( arr1 ) ;
   retArr = bothSidesDeWeight( tmpArr, arr2 ) ;
   return retArr ;
}

// de-weight single side
function singleSideDeWeight( arr )
{
   var retArr = new Array() ;
   var flag = 0 ;
   
   for ( var i = 0; i < arr.length; i++ )
   {
      flag = 0 ;
      for ( var j = i+1; j < arr.length; j++ )
      {
         if ( arr[i] == arr[j] )
         {
            flag = 1 ;
         }
      }
      if ( flag == 0 )
      {
         retArr.push( arr[i] ) ;
      }
   }
   return retArr ;
}

// de-weight both sides
function bothSidesDeWeight( arr1, arr2 )
{
   var retArr = new Array() ;
   var flag = 0 ;

   for ( var i = 0; i < arr1.length; i++ )
   {
      flag = 0 ;
      for ( var j = 0; j < arr2.length; j++ )
      {
         if ( arr1[i] == arr2[j] )
         {
            flag = 1 ;
         }
      }
      if ( flag == 0 )
      {
         retArr.push( arr1[i] ) ;
      }
   }
   return retArr ;
}

/* *****************************************************************************
@discretion: check whether there is "ip hostname" conflicting,
             like "192.168.20.40 ubuntu-dev" and "192.168.20.41 ubuntu-dev"
             are conflicting
@author: Tanzhaobo
@parameter
   arr1[Array]: update hosts info array
   arr2[Array]: local hosts info array
@return
   retArr[Array]: the array contained the conflicting content,
                  like "[{hostname:ip1, hostname:ip2},...]"
***************************************************************************** */
function conflictCheck( arr1, arr2 )
{
   var retArr = new Array() ;

   if ( arr1.length == 0 && arr2.length == 0 )
   {
      return retArr ;
   }
   // check conflict in update hosts info array
   retArr = singleSideConflictCheck( arr1 ) ; 
   if ( retArr.length )
   {
print("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@2\n") ;
      return retArr ;
   }
   // check conflict in local hosts info array
   retArr = singleSideConflictCheck( arr2 ) ;
   if ( retArr.length )
   {
      return retArr ;
   }
   // check conflict in both sides
   retArr = bothSidesConfictCheck( arr1, arr2 ) ;
   if ( retArr.length )
   {
      return retArr ;
   }

   return retArr ;
}
// check in single side 
function singleSideConflictCheck( arr )
{
print("singleSideCheck>>>>>>>>>>>>>>>>>>>>>\n") ;
   var retArr = new Array() ;

   if ( arr.length == 0 )
   {
      return retArr ;
   }
   for ( var i = 0; i < arr.length; i++ )
   {
      var strs1 = arr[i].split( separator ) ;
      var ip1 = strs1[0] ;
      var hostname1 = strs1[1] ;
      for ( var j = i + 1; j < arr.length; j++ )
      {
         var strs2 = arr[j].split( separator ) ;
         var ip2 = strs2[0] ;
         var hostname2 = strs2[1] ;

         if ( hostname2 == hostname1 && ip2 != ip1 )
         {
print("ip1 is: " + ip1 + "\n") ;
print("ip2 is: " + ip2 + "\n") ;
print("hostName1 is: " + hostname1 + "\n") ;
print("hostName2 is: " + hostname2 + "\n") ;
            retArr.push( hostname1 + ":" + ip1 + ", " + hostname2 + ":" + ip2 ) ;
         }
      }
   }
print("############### retArr size is: " + retArr.length + "\n") ;
   return retArr ;
}
// check in both sides
function bothSidesConfictCheck( arr1, arr2 )
{
print("both sides check>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n") ;
   var retArr = new Array() ;

   if ( arr1.length == 0 || arr2.lenght == 0 )
   {
      return retArr ;
   }
   for ( var i = 0; i < arr1.length; i++ )
   {
      var strs1 = arr1[i].split( separator ) ;
      var ip1 = strs1[0] ;
      var hostname1 = strs1[1] ;
      for ( var j = 0; j < arr2.length; j++ )
      {
         var strs2 = arr2[j].split( separator ) ;
         var ip2 = strs2[0] ;
         var hostname2 = strs2[1] ;

         if ( hostname2 == hostname1 && ip2 != ip1 )
         {
print("ip1 is: " + ip1 + "\n") ;
print("ip2 is: " + ip2 + "\n") ;
print("hostName1 is: " + hostname1 + "\n") ;
print("hostName2 is: " + hostname2 + "\n") ;
            retArr.push( hostname1 + ":" + ip1 + ", " + hostname2 + ":" + ip2 ) ;
         }
      }
   }
print("$$$$$$$$$$$$$$$$$$$$$$$$ retArr size is: " + retArr.length + "\n") ;
   return retArr ;
}

/* *****************************************************************************
@discretion: backup the hosts table before modify it
@author: Tanzhaobo
@parameter
   ssh[object]: ssh Object
   osInfo[string]: os information, "LINUX" or "WINDOWS"
   time[string]: backup time
@return void
***************************************************************************** */
function backupHostsTable( ssh, osInfo, time )
{
   var file = null ;
/*
   var date = new Date() ;
   var dateStr = date.toLocaleDateString() ;
   var timeStr = date.toLocaleTimeString() ;
   var strs = dateStr.split( '/' ) ;
   var str = strs[2] + "-" + strs[1] + "-" + strs[0] + ":" + timeStr ;
*/ 
print("6666666666666666666666\n") ;
   if ( "LINUX" == osInfo )
   {
      var backupFile = OMA_HOSTS_L + ".BackupBySdbcm." + time ;
print( "backupFile is: " + backupFile + "\n") ;
      ssh.push( OMA_HOSTS_L, backupFile ) ;
print("77777777777777777777777777\n") ;
   }
   else
   {
      // TODO: windows
   } 
}

/* *****************************************************************************
@discretion: add the hosts info to hosts table
@author: Tanzhaobo
@parameter
   ssh[object]: ssh Object
   osInfo[string]: os information, "LINUX" or "WINDOWS"
   arr[Array]: update hosts info array
@return void
***************************************************************************** */
function updateHostsInfo( ssh, osInfo, arr )
{
   var file = null ;
   var time = genTimeStamp() ;
   var promptStr = OMA_HOSTS_TABLE_PROMPT1 + OMA_HOSTS_TABLE_PROMPT2 +
                   time + OMA_HOSTS_TABLE_PROMPT1 ;
print("@@@@@@@@@@@@@@@ffffffffffff osInfo is: " + osInfo + "\n") ; 
   if ( arr.length == 0 )
   {
      return ;
   }
   // backup hosts table first
   backupHostsTable( ssh, osInfo, time ) ;

print("88888888888888888888888888888\n") ;
   // copy hosts table to OMA_TEMP_HOSTS_TABLE_L then modify it
   if ( "LINUX" == osInfo )
   {
//      ssh.exec( "mkdir -p " + OMA_TEMP_HOSTS_TABLE_PATH_L ) ;
      Cmd.run( "mkdir -p " + OMA_TEMP_HOSTS_TABLE_PATH_L ) ;
print("OMA_HOSTS_L is: " + OMA_HOSTS_L + "\n") ;
print("OMA_TEMP_HOSTS_TABLE_L is: " + OMA_TEMP_HOSTS_TABLE_L + "\n") ;
      ssh.pull( OMA_HOSTS_L, OMA_TEMP_HOSTS_TABLE_L ) ;
      file = new File( OMA_TEMP_HOSTS_TABLE_L ) ;
      file.seek( 0, 'e' ) ;
      file.write( OMA_NEW_LINE_L ) ;
      file.write( promptStr ) ;
      for( var i = 0; i < arr.length; i++ )
      {
         file.write( "\n" ) ;
         file.write( arr[i] ) ; 
      }
      file.close() ;
      ssh.push( OMA_TEMP_HOSTS_TABLE_L, OMA_HOSTS_L ) ;
   }
   else
   {
      // TODO: windows
   }
}

/* *****************************************************************************
@discretion: get the info to update sdbcm config file
@author: Tanzhaobo
@parameter
   obj[object]: the object of hosts info
@return
   retArr[Array]: the array of update info, ["ubuntu-dev1_Port=12000", ...]
***************************************************************************** */
function getSdbcmCfgUpdateInfo( obj )
{
   var retArr = new Array() ;
   var arr = obj[HostInfo] ;
   var hostName = "" ;
   var agentPort = undefined ;

   for ( var i = 0; i < arr.length; i++ )
   {
      agentPort = undefined ;
      hostName = arr[i][HostName] ;
      agentPort = arr[i][AgentPort] ;
      if ( typeof(agentPort) == "undefined" )
      {
         continue ;
      }
      retArr.push( hostName + "_Port=" + agentPort ) ;
   }
print("retArr's size is: " + retArr.length + "\n") ;
   return retArr ;
}

/* *****************************************************************************
@discretion: add the sdbcm config info to local sdbcm config file
@author: Tanzhaobo
@parameter
   ssh[object]: ssh Object
   osInfo[string]: os information, "LINUX" or "WINDOWS"
   arr[Array]: update info
@return void
***************************************************************************** */
function updateSdbcmCfgFile( ssh, osInfo, arr )
{
   var file = null ;
print("8888888888888888888arr size is:" + arr.length + "\n") ;
   // copy hosts table to OMA_TEMP_HOSTS_TABLE_L then modify it
   if ( "LINUX" == osInfo )
   {
      file = new File( sdbcmCfgFile ) ;
      file.seek( 0, 'e' ) ;
      for( var i = 0; i < arr.length; i++ )
      {
         file.write( "\n" ) ;
         file.write( arr[i] ) ;
      }
      file.close() ;
print("99999999999999999999999\n") ;
   }
   else
   {
      // TODO: windows
   }
}

function main()
{
   var user                     = null ;
   var passwd                   = null ;
   var ssh                      = null ;
   var osInfo                   = null ;
   var updateInfoObj            = null ;
   var hostsArray               = null ;
   var cfgArray                 = null ; 
   var localHostsInfoObj        = null ;
   var localHostsArray          = null ;

   try
   {
print("111111222222222223333333333\n") ;
      // check arguments
      if ( typeof ( HOSTS_INFO ) == "undefined"  )
      {
         objRet.Rc = -6 ;
         objRet.detail = "not specified hosts info for update" ;
         return objRet ;
      }
print("44444444444455555555555555666666666666\n") ;
      // get sdbcm config file
      sdbcmCfgFile = Oma.getOmaConfigFile() ;
print("######### sdbcmCfgFile is: " + sdbcmCfgFile + "\n") ;
      if ( typeof ( sdbcmCfgFile ) == "undefined"  )
      {
         objRet.Rc = -6 ;
         objRet.detail = "can't get sdbcm config file" ;
         return objRet ;
      }

      // get update hosts info
      updateInfoObj = eval( '(' + HOSTS_INFO + ')' ) ;
      // get user/passwd(root)
print("User: " + User + '\n') ;
print("Passwd: " + Passwd + '\n') ;
      user = updateInfoObj[User] ;
      passwd = updateInfoObj[Passwd] ;
print("user is: " + user + "\n") ;
print("passwd is: " + passwd + "\n") ;
      if ( typeof( user ) == null || typeof( user) == "undefined" ||
           typeof( passwd ) == null || typeof( passwd ) == "undefined" )
      {
print("7777777788888888888888888889999999999999\n") ;
         objRet.Rc = -6 ;
         objRet.detail = "not specifed username and password " ;
         return objRet ;
      }

      // new ssh
      ssh = new Ssh( LocalHost, user, passwd ) ;
      // get os info
      osInfo = System.type() ;
print("000000000000 osInfo is: " + osInfo + "\n") ;


      // get hosts updated info
      hostsArray = getUpdateHostsInfo ( updateInfoObj ) ;


/// debug
/*
      localHostsInfoObj = localHosts ;
      localHostsArray = getLocalHostsInfo( localHostsInfoObj ) ;
*/
      // get local hosts info and convert it into array
      localHostsInfoObj = System.getHostsMap() ;
      localHostsArray = getLocalHostsInfo(
                        eval( '(' + localHostsInfoObj + ')' ) ) ;
print("11111111111111\n") ;
       // conflict check
      var conflictArr = new Array() ;
      conflictArr = conflictCheck( hostsArray, localHostsArray ) ;
      if ( conflictArr.length )
      {
         objRet.Rc = -6 ;
         objRet.detail = "hosts info conflict:" ;
         for ( var i = 0; i < conflictArr.length; i++ )
         {
            objRet.detail = objRet.detail + " " + conflictArr[i] + "; " ;
         }
print("objRet.detail is: " + objRet.detail + "\n") ;
         return objRet ;
      }
print("33333333333333333\n") ;
print("updateHostsInfoArray size is:" + hostsArray.length + "\n") ;
for ( var i = 0; i < hostsArray.length; i++ )
{
   print( "result is: " + hostsArray[i].toString() + " 3\n") ;
}

      // de-weight
      hostsArray = deWeight( hostsArray, localHostsArray ) ;
print("updateHostsInfoArray size is:" + hostsArray.length + "\n") ;
for ( var i = 0; i < hostsArray.length; i++ )
{
   print( "result is: " + hostsArray[i].toString() + " 1\n") ;
}
print("4444444444444444\n") ;
      // update to local hosts table
      updateHostsInfo( ssh, osInfo, hostsArray ) ;
print("555555555555555\n") ;



      // get sdbcm config file updated info
      cfgArray = getSdbcmCfgUpdateInfo ( updateInfoObj ) ;
print("666666666666666655555555555555555555555555555555\n") ;
      // update to local sdbcm config file
      updateSdbcmCfgFile( ssh, osInfo, cfgArray ) ;
print("0000000000000000000000000000000000000\n") ;
      return objRet ;
   }
   catch ( e )
   {
print("err is : " + e + "\n") ; 
      if ( typeof(e) != "number" )
      {
         objRet.Rc = -10 ;
         objRet.detail = "system error" ;
      }
      else
      {
         var errMsg = "" ;
         objRet.Rc = e ;
         errMsg = getLastErrMsg() ;
         if ( "" != errMsg && null != errMsg && undefined != errMsg )
         {
            objRet.detail = eval( '(' + errMsg + ')' ) ;
         }
      }
      return objRet ;
   }
   return objRet ;
}

// execute
main() ;


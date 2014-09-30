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
@description: update sdbcm config file 
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter
   BUS_JSON: the format is: 
   SYS_JSON: the format is: 
   ENV_JSON:
@return
   RET_JSON: the format is: {"errno":0,"detail":""}
*/



if ( typeof(HOSTS_INFO) == "undefined" ) {}
if ( typeof(SDBCM_CONFIG_FIEL) == "undefined" ) {}

var SDBCM_CONFIG_FIEL = "/home/users/tanzhaobo/sequoiadb/conf/sdbcm.conf" ;

// right

HOSTS_INFO = "{ \"HostName\": \"rhel64-test8\", \"IP\": \"192.168.20.165\", \"User\": \"root\", \"Passwd\": \"sequoiadb\", \"HostInfo\": [ { \"HostName\": \"rhel64-test7\", \"IP\": \"192.168.20.165\", \"AgentPort\": \"54321\" }, { \"HostName\": \"rhel64-test10\", \"IP\": \"192.168.20.166\", \"AgentPort\": \"12345\" } ] }" ;

var objRet = new Object() ;

objRet.Errno          = 0 ;
objRet.detail      = "" ;

//TODO: add to common.js
var User           = "User" ;
var Passwd         = "Passwd" ;
var HostName       = "HostName" ;
var IP             = "IP" ;
var Ip             = "Ip" ;
var HostInfo       = "HostInfo" ;
var Hosts          = "Hosts" ;
var AgentPort      = "AgentPort" ;

/* *****************************************************************************
@discretion: get the info to update sdbcm config file
@author: Tanzhaobo
@parameter
   obj[object]: the object of hosts info
@return
   retArr[Array]: the array of update info, ["ubuntu-dev1_Port=12000", ...]
***************************************************************************** */
function getUpdateInfo( obj )
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
function updateSdbcmConfigFile( ssh, osInfo, arr )
{
   var file = null ;
print("88888888888888888888888888888\n") ;
   // copy hosts table to OMA_TEMP_HOSTS_TABLE_L then modify it
   if ( "LINUX" == osInfo )
   {
      file = new File( SDBCM_CONFIG_FIEL ) ;
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
   var user = null ;
   var passwd = null ;
   var ssh = null ;
   var osInfo = null ;
   var updateInfoObj = null ;
   var updateInfoArray = new Array() ;

   try
   {
      // check arguments
      if ( typeof ( HOSTS_INFO ) == "undefined"  )
      {
         objRet.Errno = -6 ;
         objRet.detail = "not specified hosts info" ;
         return objRet ;
      }
      if ( typeof ( SDBCM_CONFIG_FIEL ) == "undefined"  )
      {
         objRet.Errno = -6 ;
         objRet.detail = "not specified sdbcm config file" ;
         return objRet ;
      }

      // get update info
      updateInfoObj = eval( '(' + HOSTS_INFO + ')' ) ;
      user = updateInfoObj[User] ;
      passwd = updateInfoObj[Passwd] ;
      if ( typeof( user ) == null || typeof( user) == "undefined" ||
           typeof( passwd ) == null || typeof( passwd ) == "undefined" )
      {
         objRet.Errno = -6 ;
         objRet.detail = "not specifed username and password " +
                         "for update sdbcm config file" ;
         return objRet ;
      }
print("user is: " + user + "\n") ;
print("passwd is: " + passwd + "\n") ;
      // new ssh
      ssh = new Ssh( "127.0.0.1", user, passwd ) ;
      // get os info
      osInfo = System.type() ;
print("000000000000 osInfo is: " + osInfo + "\n") ;
      // get update hosts array
      updateInfoArray = getUpdateInfo ( updateInfoObj ) ;
print("update info array's size is: " + updateInfoArray.length + "\n") ;
print("444444444444444\n") ;
      // update to local hosts table
      updateSdbcmConfigFile( ssh, osInfo, updateInfoArray ) ;
print("555555555555555\n") ;

      return objRet ;
   }
   catch ( e )
   {
print("err is : " + e + "\n") ; 
      if ( typeof(e) != "number" )
      {
         objRet.Errno = -10 ;
         objRet.detail = "system error" ;
      }
      else
      {
         var errMsg = "" ;
         objRet.Errno = e ;
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


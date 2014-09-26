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
@description: check wether localhost can ping and ssh to remote host
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter
   BUS_JSON: the info for basic check host, it's format is as: { "HostInfo": [ { "IP": "192.168.20.165", "HostName": "rhel64-test8", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb", "SshPort": "22", "AgentPort": "11790" }, { "IP": "192.168.20.166", "HostName": "rhel64-test9", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb", "SshPort": "22", "AgentPort": "11790" } ] }
   SYS_JSON:
   ENV_JSON:
@return
   RET_JSON: the basic check result, the format is as: { "HostInfo": [ { "IP": "192.168.20.165", "Ping": true, "Ssh": true, "errno": 0, "detail": "" }, { "IP": "192.168.20.166", "Ping": true, "Ssh": true, "errno": 0, "detail": "" } ] }

*/

//var BUS_JSON = { "HostInfo": [ { "IP": "192.168.20.165", "HostName": "rhel64-test8", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb", "SshPort": "22", "AgentPort": "11790" }, { "IP": "192.168.20.166", "HostName": "rhel64-test9", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb", "SshPort": "22", "AgentPort": "11790" } ] } ;


var RET_JSON = new Object() ;
RET_JSON[HostInfo] = [] ;

/* *****************************************************************************
@discretion: check wether host can been "ping" and "ssh" or not
@author: Tanzhaobo
@parameter
   user[string]: the user name
   passwd[string]: the password
   ip[string]: the ip address
@note
   either ip or hostname must be specified
@return
   retStr[string]: the hostname after adapting
***************************************************************************** */
function basicCheckHost( user, passwd, ip )
{
   var retObj          = new Object() ;
   retObj[Errno]       = SDB_OK ;
   retObj[Detail]      = "" ;
   retObj[CanPing]     = false ;
   retObj[CanSsh]      = false ;
   retObj[IP]          = "" ;

   // ip
   retObj[IP] = ip ;
   // ping
   var ret = System.ping( ip, 3 ) ;
   var ping = eval( "(" + ret + ")" ) ;
   if ( true != ping[Reachable] )
   {
      return retObj ;
   }
   retObj[CanPing] = true ;
   // ssh
   try
   {
      var ssh = new Ssh( ip, user, passwd ) ;
      retObj[CanSsh] = true ;
   }
   catch ( e )
   {
      retObj[CanSsh] = false ;
      retObj[Errno] = getLastError() ;
      retObj[Detail] = getLastErrMsg() ;
   }

   return retObj ;
}

function main()
{
   // check input argument
   var infoArr = BUS_JSON[HostInfo] ;
   var arrLen = infoArr.length ;
   if ( arrLen == 0 )
   {
      setLastErrMsg( "Not specified any host to check" ) ;
      throw SDB_INVALIDARG ;
   }
   for( var i = 0; i < arrLen; i++ )
   {
      var obj      = infoArr[i] ;
      var user     = obj[User] ;
      var passwd   = obj[Passwd] ;
      var ip       = obj[IP] ;
      var ret      = null ;
      ret = basicCheckHost( user, passwd, ip ) ;
      RET_JSON[HostInfo].push( ret ) ;
   }
//print("RET_JSON is: " + JSON.stringify(RET_JSON) + "\n") ;
   return RET_JSON ;
}

// execute
main() ;


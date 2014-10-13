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
@description: scan host
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter
   BUS_JSON: the info for scan host, it's format is as: { "HostInfo": [ { "IP": "192.168.20.165", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb", "SshPort": "22", "AgentPort": "11790" }, { "HostName": "rhel64-test9", "User": "root", "Passwd": "sequoiadb", "InstallPath": "/opt/sequoiadb", "SshPort": "22", "AgentPort": "11790" } ] } ;
   SYS_JSON:
   ENV_JSON:
@return
   RET_JSON the scan result, the format is as: { "HostInfo": [ { "errno": 0, "detail": "", "Ping": true, "Ssh": false, "HostName": "rhel64-test8", "IP": "" }, { "errno": 0, "detail": "", "Ping": true, "Ssh": true, "HostName": "rhel64-test9", "IP": "192.168.20.166" } ] }
*/

var RET_JSON       = new Object() ;
RET_JSON[HostInfo] = [] ;

/* *****************************************************************************
@discretion: scan a remote host, to check wether it can been "ping" and "ssh"
             or not, and try to get it's hostname if hostname is not specified
@author: Tanzhaobo
@parameter
   user[string]: the user name
   passwd[string]: the password
   hostname[string] the hostname
   ip[string]: the ip address
@note
   either ip or hostname must be specified
@return
   retStr[string]: the hostname after adapting
***************************************************************************** */
function scanHost( user, passwd, hostname, ip )
{
   var retObj          = new Object() ;
   retObj[Errno]       = SDB_OK ;
   retObj[Detail]      = "" ;
   retObj[CanPing]     = false ;
   retObj[CanSsh]      = false ;
   retObj[HostName]    = "" ;
   retObj[IP]          = "" ;

   // in case hostname is specified
   if ( null != hostname && undefined != hostname )
   {
      // hostname
      retObj.HostName = hostname ;
      // ping
      var ret = System.ping( hostname ) ;
      var ping = eval( "(" + ret + ")" ) ;
      if ( true != ping[Reachable] )
      {
         return retObj ;
      }
      retObj[CanPing] = true ;
      // ssh
      try
      {
         var ssh = new Ssh( hostname, user, passwd ) ;
         retObj[CanSsh] = true ;
      }
      catch ( e )
      {
         retObj[Errno] = getLastError() ;
         retObj[Detail] = getLastErrMsg() ;
         retObj[CanSsh] = false ;
      }
      // ip
      var ipTmp = null ;
      try
      {
         ipTmp = ssh.getPeerIP() ;
      }
      catch ( e )
      {
         retObj[Errno] = getLastError() ;
         retObj[Detail] = getLastErrMsg() ;
         return retObj ;
      }
      // if no error, extract the ip
      if ( "string" == typeof(ipTmp) )
      {
         retObj[IP] = removeLineBreak( ipTmp ) ;
      }
   }
   else if ( null != ip && undefined != ip )
   {
      // ip
      retObj[IP] = ip ;
      // ping
      var ret = System.ping( ip, 3 ) ;
      var ping = eval( "(" + ret + ")" ) ;
      if ( true != ping[Reachable] )
         return retObj ;
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
      // hostName
      try
      {
         var name = ssh.exec("hostname") ;
      }
      catch ( e )
      {
         retObj[Errno] = getLastError() ;
         retObj[Detail] = getLastErrMsg() ;
         return retObj ;
      }
      try
      {
         SSH_CHECK ( ssh, SDB_SYS ) ;
      }
      catch( e )
      {
         retObj[Errno] = e ;
         retObj[Detail] = getLastErrMsg() ;
         return retObj ;
      }
      if ( "string" == typeof(name) )
      {
         retObj[HostName] = removeLineBreak( name ) ;
      }
   }
   return retObj ;
}

function main()
{
   var infoArr = BUS_JSON[HostInfo] ;
   var arrLen = infoArr.length ;
   if ( arrLen == 0 )
   {
      setLastErrMsg( "Not specified any host to scan" ) ;
      setLastError( SDB_INVALIDARG ) ;
      throw SDB_INVALIDARG ;
   }
   for( var i = 0; i < arrLen; i++ )
   {
      var obj      = infoArr[i] ;
      var user     = obj[User] ;
      var passwd   = obj[Passwd] ;
      var hostname = obj[HostName] ;
      var ip       = obj[IP] ;
      var ret      = null ;
      if ( undefined != hostname )
      { 
         ret = scanHost( user, passwd, hostname, null ) ;
      }
      else if ( undefined != ip )
      {
         ret = scanHost( user, passwd, null, ip ) ;
      }
      else
      {
         setLastErrMsg( "Not specified hostname or ip" ) ;
         setLastError( SDB_INVALIDARG ) ;
         throw SDB_INVALIDARG ;
      }
      RET_JSON[HostInfo].push( ret ) ;
   }

   return RET_JSON ;
}

// execute
main() ;


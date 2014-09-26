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
@description: common things for all the js files in current document
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/

/* *****************************************************************************
@discretion: get last error number
@author: Tanzhaobo
@parameter
   e: the exception
   flag[bool]: throw exception nor not
@return
   
***************************************************************************** */
function GETLASTERROR ( e, flag )
{
   var ret = getLastError() ;
   if ( SDB_OK == typeof(ret) )
   {
      return ret ;
   }
   else
   {
      if ( "number" == typeof(e) )
      {
         return e ;
      }
      else
      {
         if ( flag )
         {
            throw e ;
         }
         else
         {
            return SDB_SYS ;
         } 
      }
   }
}

/* *****************************************************************************
@discretion: get last error message
@author: Tanzhaobo
@parameter void
@return
   retStr[string]: error message
***************************************************************************** */
function GETLASTERRMSG ()
{
   var retStr = "" ;
   var msg = getLastErrMsg() ;
   if ( "undefined" != typeof(msg) )
   {
      retStr = msg ;
   }
   return retStr ;
}

/* *****************************************************************************
@discretion: check whether a value is usable or not
@author: Tanzhaobo
@parameter
   val: the value to check
@return void
***************************************************************************** */
function VALUE_CHECK ( val )
{
   if ( val == null || typeof(val) == "undefined" )
   {
      setLastErrMsg( "value is undefined" ) ;
      setLastError( SDB_INVALIDARG ) ;
      throw SDB_INVALIDARG ;
   }
}

/* *****************************************************************************
@discretion: check whether cmd execute is successful or not
@author: Tanzhaobo
@parameter
   ssh[object]: the ssh object
   exp[number]: error number
@return void
***************************************************************************** */
function CMD_CHECK( cmd, exp )
{
   if ( typeof (cmd) == "undefined" )
   {
      setLastErrMsg( "Invalid Cmd object" ) ;
      setLastError( SDB_INVALIDARG ) ;
      throw SDB_INVALIDARG ;
   }
   if ( cmd.getLastRet() )
   {
      setLastErrMsg( ssh.getLastOut() ) ;
      setLastError( exp ) ;
      throw exp ;
   }
}

/* *****************************************************************************
@discretion: check whether ssh execute is successful or not
@author: Tanzhaobo
@parameter
   ssh[object]: the ssh object
   exp[number]: error number
@return void
***************************************************************************** */
function SSH_CHECK( ssh, exp )
{
   if ( typeof (ssh) == "undefined" )
   {
      setLastErrMsg( "Invalid Ssh object" ) ;
      setLastError( SDB_INVALIDARG ) ;
      throw SDB_INVALIDARG ;
   }
   if ( ssh.getLastRet() )
   {
      setLastErrMsg( ssh.getLastOut() ) ;
      setLastError( exp ) ;
      throw exp ;
   }
}

/* *****************************************************************************
@discretion: remove the "\n" or "\n\r" in the end of string
@author: Tanzhaobo
@parameter
   str[string]: the string to deal with
@return
   retStr[string]: the return string without "\n" or "\n\r"
***************************************************************************** */
function removeLineBreak ( str )
{
   var osInfo = System.type() ;
   var retStr = str ;

   if ( "LINUX" == osInfo )
   {
      var i = str.indexOf( "\n" ) ;
      if ( -1 != i )
      {
         var substr = str.substring(0, i);
         retStr = substr ;
      }
      else
      {
         retStr = str ;
      }
   }
   else
   {
      // TODO:
   }
   return retStr ;
}

/* *****************************************************************************
@discretion: get the script's path
@author: Tanzhaobo
@parameter
   path[string]: the path of sdbcm config file
@return
   retStr[string]: the path of script files
***************************************************************************** */
function getScriptPath( path )
{
   var osInfo = System.type() ;
   var retStr = "" ;
   var str = "" ;

   if ( "LINUX" == osInfo )
   {
      str = "/" ;
      var pos = path.lastIndexOf( str ) ;
      if ( -1 == pos )
      {
         setLastErrMsg( "Invalid sdbcm config file's path: " + path ) ;
         setLastError( SDB_INVALDARG ) ;
         throw SDB_INVALIDARG ;
      }
      retStr = path.substring( 0, pos + 1 ) + OMA_PATH_SCRIPT_L ;
   }
   else
   {
      // TODO:
   }
   return retStr ;
}

/* *****************************************************************************
@discretion: gen a string with the format "YYYY-MM-DD-HH.mm.ss.ffffff"
@author: Tanzhaobo
@parameter
@return
   retStr[string]: a string with the format "YYYY-MM-DD-HH.mm.ss.ffffff"
                   to express current timestamp
***************************************************************************** */
function genTimeStamp()
{
   var retStr = null ;
   var dateVar = new Date() ;
   var dateStr = dateVar.toLocaleDateString() ;
   var timeStr = dateVar.toLocaleTimeString() ;
   var strs = dateStr.split( '/' ) ;
   retStr = strs[2] + "-" + strs[1] + "-" + strs[0] + "-" + timeStr ;

   return retStr ;
}

/* *****************************************************************************
@discretion: get the total number of program about sdbcm in remote host
             according the result of sdblist
@author: Tanzhaobo
@parameter
   str[string]: the result of sdblist
@return
   retNum[number]: the total nunber or -1
***************************************************************************** */
function extractTotalNumber( str )
{
   var retNum = -1 ;
   var symbol = "Total:" ;

   var pos = str.lastIndexOf( symbol ) ;
   if ( -1 != pos )
   {
      var subStr = str.substring( pos + symbol.length, str.length ) ;
      retNum = parseInt( subStr ) ;
   }
   return retNum ;
}

/* *****************************************************************************
@discretion: get remote sdbcm port according the result of sdblist
@author: Tanzhaobo
@parameter
   str[string]: the result of sdblist
@return
   retPort[number]: the port or -1
***************************************************************************** */
function extractPort( str )
{
   var retPort = -1 ;
   var symbol = "(" ;

   var pos = str.indexOf( symbol ) ;
   if ( -1 != pos )
   {
      var subStr = str.substring( pos + symbol.length, str.length ) ;
      retNum = parseInt( subStr ) ;
   }
   return retNum ;
}

/* *****************************************************************************
@discretion: get remote sdbcm port
@author: Tanzhaobo
@parameter
   ssh[object]: ssh object
   osInfo[string]: os type
@exception
@return
   retPort[number]: the port of remote sdbcm program
***************************************************************************** */
function getRemoteSdbcmPort( ssh, osInfo )
{
   var retPort = -1 ;
   var str = null ;

   if ( OMA_LINUX == osInfo )
   {
      var prog = OMA_PATH_TEMP_BIN_DIR_L + OMA_PROG_SDBLIST_L ;
      var cmd = prog + " -t cm " ;
      var ret = SDB_OK ;
      try
      {
         str = ssh.exec( cmd ) ;
      }
      catch ( e)
      {
         ret = ssh.getLastRet() ;
         if ( ret < 0 )
         {
            setLastErrMsg( "Failed to get remote sdbcm status" ) ;
            setLastError( SDB_SYS ) ;
            throw SDB_SYS ;
         }
         else if ( ret > 0 )
         {
            setLastErrMsg( "Remote sdbcm is not running" ) ;
            setLastError( SDB_SYS ) ;
            throw SDB_SYS ;
         }
      }
      retPort = extractPort ( str ) ;
      if ( -1 == retPort )
      {
         setLastErrMsg( "Failed to get remote sdbcm's port" ) ;
         setLastError( SDB_SYS ) ;
         throw SDB_SYS ;
      }
   }
   else
   {
      // TODO:
   }
   return retPort ;
}

/* *****************************************************************************
@discretion: check whether sdbcm is running in remote host
@author: Tanzhaobo
@parameter
   ssh[object]: ssh object
   osInfo[string]: os type
@exception
@return
   isRunning[bool]: whether sdbcm is running in remote host
***************************************************************************** */
function isSdbcmRunningInRemote( ssh, osInfo )
{
   var isRunning = false ;
   var str = null ;
   var ret = SDB_OK ;
   if ( OMA_LINUX == osInfo )
   {
      var prog = OMA_PATH_TEMP_BIN_DIR_L + OMA_PROG_SDBLIST_L ;
      var cmd = prog + " -t cm " ;
      try
      {
         str = ssh.exec( cmd ) ;
      }
      catch ( e )
      {
         ret = ssh.getLastRet() ;
         if ( ret > 0 )
         {
            isRunning = false ;
            return isRunning ;
         }
         else if ( ret < 0 )
         {
            setLastErrMsg( "Failed to get remote sdbcm status" ) ;
            setLastError( SDB_SYS ) ;
            throw SDB_SYS ;
         }
      }
      // when sdbcm is running in remote
      var num = extractTotalNumber ( str ) ;
      if ( -1 == num )
      {
         setLastErrMsg( "Failed to get remote sdbcm status" ) ;
         setLastError( SDB_SYS ) ;
         throw SDB_SYS ;
      }
      else if ( 0 == num )
      {
         isRunning = false ;
      }
      else
      {
         isRunning = true ;
      }
   }
   else
   {
      // TODO:
   }
   return isRunning ;
}

/* *****************************************************************************
@discretion: check whether it's in local host environment
@author: Tanzhaobo
@parameter
   ssh[object]: Ssh object
@return
   [bool]: whether it's in local host environment
***************************************************************************** */
function isInLocalHost( ssh )
{
   var ip1 = ssh.getLocalIP() ;
   var ip2 = ssh.getPeerIP() ;
   if( ip1 == ip2 )
   {
      return true ;
   }
   else
   {
      return false ;
   }
}

/* *****************************************************************************
@discretion: adapt path with "\"(linux) or "//"(window) in the end
@author: Tanzhaobo
@parameter
   ssh[object]: ssh object
   osInfo[string]: os type
@return void
***************************************************************************** */
function adaptPath( osInfo, path )
{
   var s = "" ;
   var i = -1 ;
   if ( OMA_LINUX == osInfo )
   {
      s = "/" ;
   }
   else
   {
      s = "\\" ;
   }
   i = path.indexOf( s ) ;
   if ( (i != -1) && (i != path.length) )
      path += s ;
   return path ;
}

/* *****************************************************************************
@discretion: judge whethe a value is array or not
@author: Tanzhaobo
@parameter
   value[]: input value
@return
   [bool]: return whethe the value is array
***************************************************************************** */
function isArray( value )
{
   return Object.prototype.toString.call(value) === '[object Array]' ;
}

/* *****************************************************************************
@discretion: judge whethe a value is array or not
@author: Tanzhaobo
@parameter
   port[int]: input port
@return
   ret[bool]: return whether the port is a reserved port
***************************************************************************** */
function isReservedPort( port )
{
   var ret = false ;
   var len = OMA_RESERVED_PORT.length ;
   for ( var i = 0; i < len; i++ )
   {
      var val = OMA_RESERVED_PORT[i] ;
      var flag = isArray( val ) ;
      if ( flag )
      {
         var port1 = val[0] ;
         var port2 = val[1] ;
         if ( port1 <= port && port <= port2 )
         {

            ret = true ;
            break ;
         } 
      }
      else
      {
         if ( port === val )
         {
            ret = true ;
            break ;
         }
      }
   }
   return ret ;
}

/* *****************************************************************************
@discretion: get a usable port from local host
@author: Tanzhaobo
@parameter
   osInfo[string]: os type
@return
   retPort[nunber]: return a usable port or OMA_PORT_INVALID
***************************************************************************** */
function getAUsablePortFromLocal( osInfo )
{
   var retPort = OMA_PORT_INVALID ;
   var flag = false ;   

   if ( OMA_LINUX == osInfo )
   {
      for ( var port = OMA_PORT_TEMP_AGENT_PORT ;
            port <= OMA_PORT_MAX; port++ )
      {
         flag = isReservedPort( port ) ;
         if ( flag )
         {
            continue ;
         } 
         var ret =  eval( '(' + System.sniffPort( port ) + ')' ) ;
         flag = ret[Usable];
         if ( flag )
         {
            retPort = port ;
            break ;
         }
         else
         {
            port++ ;
         }
      }
   }
   else
   {
      // TODOL
   }
   return retPort ;
}

/* *****************************************************************************
@discretion: get a usable port from remote host
@author: Tanzhaobo
@parameter
   ssh[object]: ssh object
   osInfo[string]: os type
@return
   retPort[nunber]: return a usable port or OMA_PORT_INVALID
***************************************************************************** */
function getAUsablePortFromRemote( ssh, osInfo )
{
   var retPort = OMA_PORT_INVALID ;
   var cmd = "" ;
   var port = OMA_PORT_TEMP_AGENT_PORT ;
   var flag = false ;

   if ( OMA_LINUX == osInfo )
   {
      for ( var port = OMA_PORT_TEMP_AGENT_PORT ;
            port <= OMA_PORT_MAX; port++ )
      {
         flag = isReservedPort( port ) ;
         if ( flag )
         {
            continue ;
         }
         cmd = "netstat -nap | grep " + port + " | grep -v grep" ;
         try
         {
            ssh.exec( cmd ) ;
         }
         catch ( e )
         {
            var ret = ssh.getLastRet() ;
            if ( 1 == ret )
            {
               retPort = port ;
               break ;
            }
            else
            {
               retPort = OMA_PORT_INVALID ;
               break ;
            }
         }
      }
   }
   else
   {
      // TODO:
   }
   return retPort ;
}

/* *****************************************************************************
@discretion: get the right place to change the owner of a directory
@author: Tanzhaobo
@parameter
   ssh[object]: ssh object
   osInfo[string]: os type
@return
   retPort[nunber]: return a dirctory path
***************************************************************************** */
function getThePlaceToChangeOwner( osInfo, path )
{
   var retStr = path ;
   var pos = -1 ;
   if ( OMA_LINUX == osInfo )
   {
      var arr = path.split( '/' ) ;
      var num = arr.length ;
      // in case: "/" or "/opt"
      if ( num <= 2 )
      {
         return path ;
      }
      pos = path.lastIndexOf( '/' ) ;
      // in case: "/opt/"
      if ( pos == path.length -1 && num == 3 )
      {
         return path ;
      }
      // otherwise
      var len = arr[num - 1].length ; ;
      // in case: "/opt/.../123/345"
      if ( len )
      {
         pos = path.length - 1 - len ;
         retStr = path.substring( 0, pos ) ;
      }
      // in case: "/opt/.../123/345/"
      else
      {
         len = arr[num - 2].length ;
         pos = path.length - 1 - len - 1 ;
         retStr = path.substring( 0, pos ) ;
      }
   }
   else
   {
     // TODO:
   }

   return retStr ;
}

/* *****************************************************************************
@discretion: change the  owner of the directory
@author: Tanzhaobo
@parameter
   ssh[object]: ssh object
   osInfo[string]: os type
   path[string]: the path of the directory
   user[string]: the user to change to
   userGroup[sring]: the user group to change to
@return void
***************************************************************************** */
function changeDirOwner( ssh, osInfo, path, user, userGroup )
{
   var ret = null ;
   var str = null ;
   var cmd = null ;
   if ( OMA_LINUX == osInfo )
   {
      cmd = " mkdir -p " + path ;
      try
      {
         ssh.exec( cmd ) ;
      }
      catch ( e )
      {
         setLastErrMsg( "Failed to create path: " + path ) ;
         setLastError( SDB_SYS ) ;
         throw SDB_SYS ;
      }
      path = getThePlaceToChangeOwner( osInfo, path ) ;
      str = user + ":" + userGroup ;
      cmd = " chown -R " + str + " " + path ;
      try
      {
         ssh.exec( cmd ) ;
      }
      catch ( e )
      {
         setLastErrMsg( "Failed to change path owner" ) ;
         setLastError( SDB_SYS ) ;
         throw SDB_SYS ;
      }
   }
   else
   {
      // TODO: windows
   }
}

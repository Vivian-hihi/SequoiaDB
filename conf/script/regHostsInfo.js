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
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/

if ( typeof(USERNAME) == "undefined" ) {}
if ( typeof(PASSWORD) == "undefined" ) {}
if ( typeof(IP) == "undefined" ) {}
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }
if ( typeof(HOSTSINFO) == "undefined" ) {}

// linux
var HOSTS_FILE_L        = "/etc/hosts" ;
var HOSTS_FIEL_BACKUP_L = "/etc/hosts_backup_by_omagent" ;
// windows
var HOSTS_FILE_W        = "" ;
var HOSTS_FILE_BACKUP_W = "" ;

var objRet = new Object() ;

objRet.HostName    = null ;
objRet.hasLeftCopy = false ;
objRet.Errno          = 0 ;
objRet.detail      = null ;

function backupRemoteHostsFile( ssh, osInfo )
{
   // judge wether the backup file is exsit or not
   
   // backup the hosts file
}



function main()
{
   var ssh = null ;
   var osInfo = null ;
   var file = null ;

   try
   {
      // ssh and get host name
      ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
/*
      var name = ssh.exec("hostname") ;
      var i = name.indexOf( "\n" ) ;
      var substr = name.substring(0, i) ;
      objRet.HostName = substr ;
*/
      // get OS info
      osInfo = System.type() ;
      // backup hosts file
      var remoteFile = "" ;
      if ( "LINUX" == osInfo )
      {
         remoteFile = "/etc/hosts" ;
         // save a copy for rollback
         ssh.exec( "cp /etc/hosts /etc/hostsbak" ) ;
         objRet.hasLeftCopy = true ;
      }
      else if ( "WINDOWS" == osInfo )
      {

      }
      // cp remote hosts file to local
      var localFile = "" ;
      if ( "LINUX" == osInfo )
      {
         localFile = "/tmp/hosts" ;
         ssh.pull( remoteFile, localFile ) ;
      }
      else if ( "WINDOWS" == osInfo )
      {

      }
      // append content to the file
      file = new File( localFile ) ;
      file.seek( 0, 'e' ) ;
      file.write( HOSTSINFO ) ;
      file.close() ;
      // copy file back to remote and delete the local copy
      ssh.push( localFile, remoteFile ) ;
//      File.remove( localFile ) ;
   }
   catch ( e )
   {
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


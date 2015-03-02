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
@description: check whether target host has installed or not,
              then record the result to a file in /tmp/omatmp/tmp
@modify list:
   2015-1-6 Zhaobo Tan  Init
@parameter
   BUS_JSON: the format is: { "clustername": "c1", "businessname": "b1", "usertag": "", "svcname": "20000" } ;
@return void
*/

// println
var BUS_JSON = { "clustername": "c1", "businessname": "b1", "usertag": "", "svcname": "20000" } ;

var FILE_NAME_INST_SA_PRE_CHECK = "installStandalonePreCheck.js" ;
var rc                 = SDB_OK ;
var errMsg             = "" ;

var result_file        = OMA_FILE_TEMP_INST_SA_CHECK ;

/* *****************************************************************************
@discretion: init
@author: Tanzhaobo
@parameter void
@return void
***************************************************************************** */
function _init()
{
   PD_LOG2( LOG_NONE, arguments, PDEVENT, FILE_NAME_INST_SA_PRE_CHECK,
            "Begin to pre-check install standalone" ) ;
}

/* *****************************************************************************
@discretion: final
@author: Tanzhaobo
@parameter void
@return void
***************************************************************************** */
function _final()
{
   PD_LOG2( LOG_NONE, arguments, PDEVENT, FILE_NAME_INST_SA_PRE_CHECK,
            "Finish pre-checking install standalone" ) ;
}

/* *****************************************************************************
@discretion: get installed standalone's info
@author: Tanzhaobo
@parameter void
@return
   retObj[object]:
***************************************************************************** */
function _getStandaloneInfo()
{
   var retObj            = new installedSAInfo() ;
   var standaloneInfoArr = null ;
   var installedSANum    = 0 ;
   var obj               = null ;
   
   // get the info of installed standalone
   try
   {
      var option            = new installedSAOption() ;
      var matcher           = new installedSAMather() ;
      option[SvcName2]      = BUS_JSON[SvcName2] ;
      matcher[ClusterName]  = BUS_JSON[ClusterName] ;
      matcher[BusinessName] = BUS_JSON[BusinessName] ;
      matcher[UserTag]      = BUS_JSON[UserTag] ;
      
      PD_LOG2( LOG_NONE, arguments, PDDEBUG, FILE_NAME_INST_SA_PRE_CHECK,
               sprintf( "option is: ?, matcher is: ?", JSON.stringify(option),
               JSON.stringify(matcher) ) ) ;
      standaloneInfoArr = Sdbtool.listNodes( option, matcher ) ;
      if ( "undefined" == typeof( standaloneInfoArr ) )
         exception_handle( SDB_SYS, "The information of installed standalone is undefined" ) ;
   }
   catch( e )
   {
      SYSEXPHANDLE( e ) ;
      rc = GETLASTERROR() ;
      errMsg = "Failed to get the information of installed standalone" ;
      PD_LOG2( LOG_NONE, arguments, PDERROR, FILE_NAME_INST_SA_PRE_CHECK,
               sprintf( errMsg + ", rc: ?, detail: ?", rc, GETLASTERRMSG() ) ) ;
      exception_handle( rc, errMsg ) ;
   }
   // get installed standalone amount
   try
   {
      installedSANum = standaloneInfoArr.size() ;
      if ( "number" != typeof( installedSANum ) )
         exception_handle( SDB_SYS, sprintf( "installedSANum is not a number" ) ) ;
   }
   catch( e )
   {
      SYSEXPHANDLE( e ) ;
      rc = GETLASTERROR() ;
      errMsg = "Failed to get the amount of matched standalone" ;
      PD_LOG2( LOG_NONE, arguments, PDERROR, FILE_NAME_INST_SA_PRE_CHECK,
               sprintf( errMsg + ", rc: ?, detail: ?", rc, GETLASTERRMSG() ) ) ;
      exception_handle( rc, errMsg ) ;
   }
   
   errMsg = sprintf( "[?] standalone exist(s) in host[?] with the same service name[?]",
                     installedSANum, System.getHostName(), BUS_JSON[SvcName2] ) ;
   PD_LOG2( LOG_NONE, arguments, PDEVENT, FILE_NAME_INST_SA_PRE_CHECK, errMsg ) ;
   
   if ( 0 == installedSANum )
   {
      retObj[HASINSTALLED] = false ;
   }
   else if ( 1 == installedSANum )
   {
      PD_LOG2( LOG_NONE, arguments, PDEVENT, FILE_NAME_INST_SA_PRE_CHECK,
               "Target standalone had been installed" ) ;
      // get the installed standalone info
      if ( SYS_LINUX == SYS_TYPE )
      {
         try
         {
            obj  = eval( '(' + standaloneInfoArr.pos() + ')' ) ;
            PD_LOG2( LOG_NONE, arguments, PDDEBUG, FILE_NAME_INST_SA_PRE_CHECK,
                     sprintf( "obj is: ?", JSON.stringify(obj) ) ) ;
            retObj[HASINSTALLED] = true ;
            retObj[CLUSTERNAME]  = obj[ClusterName] ;
            retObj[BUSINESSNAME] = obj[BusinessName] ;
            retObj[USERTAG]      = obj[UserTag] ;
         }
         catch( e )
         {
            SYSEXPHANDLE( e ) ;
            errMsg = sprintf( "Failed to get installed standalone's info in host[?]",
                              System.getHostName() ) ;
            rc = GETLASTERROR() ;
            PD_LOG2( LOG_NONE, arguments, PDWARNING, FILE_NAME_INST_SA_PRE_CHECK,
                     sprintf( errMsg + ", rc: ?, detail: ?", rc, GETLASTERRMSG() ) ) ;
            exception_handle( rc, errMsg ) ;
         }
      }
      else
      {
         // TODO: windows
      }
   }
   else
   {
      exception_handle( SDB_SYS, errMsg ) ;
   }
   
   return retObj ;
}

function main()
{
   _init() ;

   var info = null ;
   var str  = "" ;

   // . copy conf file to /tmp/omatmp/conf for using Sdbtool.listNodes
   
   // . get installed standalone's info
   info = _getStandaloneInfo() ;
   PD_LOG2( LOG_NONE, arguments, PDDEBUG, FILE_NAME_INST_SA_PRE_CHECK,
            sprintf( "Installed standalone's info is: [?] in host[?]",
            JSON.stringify(info), System.getHostName() ) ) ;
   
   // . write the result to file
   try
   {
      if ( File.exist( result_file ) )
         File.remove( result_file ) ;
      file = new File( result_file ) ;
      str = HASINSTALLED + "=" + info[HASINSTALLED] + OMA_NEW_LINE ;
      file.write( str ) ;
      str = CLUSTERNAME + "=" + info[CLUSTERNAME] + OMA_NEW_LINE ;
      file.write( str ) ;
      str = BUSINESSNAME + "=" + info[BUSINESSNAME] + OMA_NEW_LINE ;
      file.write( str ) ;
      str = USERTAG + "=" + info[USERTAG] ;
      file.write( str ) ;
      file.close() ;
   }
   catch( e )
   {
      SYSEXPHANDLE( e ) ;
      rc = GETLASTERROR() ;
      errMsg = sprintf( "Failed to write pre-check result to file[?] in host[?]",
                        result_file, System.getHostName() ) ;
      PD_LOG2( LOG_NONE, arguments, PDERROR, FILE_NAME_INST_SA_PRE_CHECK,
               sprintf( errMsg + ", rc: ?, detail: ?", rc, GETLASTERRMSG() ) ) ;
      exception_handle( rc, errMsg ) ;
   }
   
   _final() ;
}

// execute
   main() ;


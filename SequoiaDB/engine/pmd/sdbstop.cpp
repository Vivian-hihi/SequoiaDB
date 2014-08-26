/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = sdbstop.cpp

   Descriptive Name = sdbstop Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbstop,
   which is used to stop SequoiaDB engine.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "core.hpp"
#include "ossUtil.hpp"
#include "ossProc.hpp"
#include "ossMem.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "pmdDef.hpp"
#include "utilParam.hpp"
#include "pmdOptions.hpp"
#include "omagentDef.hpp"
#include "ossVer.h"

#include <string>
#include <iostream>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

#if defined (_LINUX)
#include <dirent.h>
#include <sys/types.h>
#include <signal.h>
#elif defined (_WINDOWS)
#include "ossNPipe.hpp"
#endif


using namespace std;
namespace po = boost::program_options;

// timeout for 30 seconds
#define TERMINATE_TIMEOUT 30

#define ADD_PARAM_OPTIONS_BEGIN( desc )\
        desc.add_options()

#define ADD_PARAM_OPTIONS_END ;

#define COMMANDS_STRING( a, b ) (string(a) +string( b)).c_str()
#define COMMANDS_OPTIONS \
       ( COMMANDS_STRING(PMD_OPTION_HELP, ",h"), "help" )\
       ( PMD_OPTION_VERSION, "version" ) \
       ( COMMANDS_STRING(PMD_OPTION_SVCNAME, ",p"), boost::program_options::value<string>(), "service name" )

#if defined (_WINDOWS)
#define ENGINE_NPIPE_MSG_SHUTDOWN "$shutdown"
#endif

#define SERVICE_SEPARATOR ","

vector<string> stopServices ;
// initialize options
void init ( po::options_description &desc )
{
   ADD_PARAM_OPTIONS_BEGIN ( desc )
      COMMANDS_OPTIONS
   ADD_PARAM_OPTIONS_END
}

void displayArg ( po::options_description &desc )
{
   std::cout << desc << std::endl ;
}

static OSS_INLINE std::string &ltrim ( std::string &s )
{
   s.erase ( s.begin(), std::find_if ( s.begin(), s.end(),
             std::not1 ( std::ptr_fun<int, int>(std::isspace)))) ;
   return s ;
}

static OSS_INLINE std::string &rtrim ( std::string &s )
{
   s.erase ( std::find_if ( s.rbegin(), s.rend(),
             std::not1 ( std::ptr_fun<int, int>(std::isspace))).base(),
             s.end() ) ;
   return s ;
}

static OSS_INLINE std::string &trim ( std::string &s )
{
   return ltrim ( rtrim ( s ) ) ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_SVCSPLIT, "serviceSplit" )
INT32 serviceSplit ( string &input )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SVCSPLIT );
   CHAR *cstr, *p ;
   CHAR *pContext = NULL ;
   INT32 bufSize = input.size() ;
   cstr = (CHAR*)SDB_OSS_MALLOC ( bufSize+1 ) ;
   if ( !cstr )
   {
      PD_LOG ( PDERROR, "Failed to allocate memory" ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( cstr, 0, bufSize + 1 ) ;
   ossStrncpy ( cstr, input.c_str(), bufSize ) ;
   p = ossStrtok ( cstr, SERVICE_SEPARATOR, &pContext ) ;
   while ( p )
   {
      string ts ( p ) ;
      stopServices.push_back ( trim ( ts ) ) ;
      p = ossStrtok ( NULL, SERVICE_SEPARATOR, &pContext ) ;
   }
done :
   if ( cstr )
      SDB_OSS_FREE ( cstr ) ;
   PD_TRACE_EXITRC ( SDB_SVCSPLIT, rc );
   return rc ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTOP_RESVARG, "resolveArgument" )
INT32 resolveArgument ( po::options_description &desc, INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBSTOP_RESVARG );
   po::variables_map vm ;

   rc = engine::utilReadCommandLine( argc, argv, desc, vm ) ;
   if ( rc )
   {
      goto error ;
   }

   if ( vm.count ( PMD_OPTION_HELP ) )
   {
      displayArg ( desc ) ;
      rc = SDB_PMD_HELP_ONLY ;
      goto done ;
   }
   else if ( vm.count( PMD_OPTION_VERSION ) )
   {
      ossPrintVersion( "SDB Stop Version" ) ;
      rc = SDB_PMD_VERSION_ONLY ;
      goto done ;
   }
   if ( vm.count ( PMD_OPTION_SVCNAME ) )
   {
      string svcname = vm[PMD_OPTION_SVCNAME].as<string>() ;
      // break service names using ';'
      rc = serviceSplit ( svcname ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to split service names, rc = %d", rc ) ;
         goto error ;
      }
   }
done :
   PD_TRACE_EXITRC ( SDB_SDBSTOP_RESVARG, rc );
   return rc ;
error :
   goto done ;
}

#if defined (_LINUX)
// PD_TRACE_DECLARE_FUNCTION ( SDB_TERMPROC, "terminateProcess" )
INT32 terminateProcess ( pid_t &pid, CHAR *pName )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_TERMPROC );
   PD_TRACE1 ( SDB_TERMPROC, PD_PACK_INT(pid) );
   INT32 round = 0 ;
   CHAR fileNameBuffer [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   ossSnprintf ( fileNameBuffer, OSS_MAX_PATHSIZE, "/proc/%d/stat", pid ) ;
   ossPrintf ( "Terminating process %d: %s"OSS_NEWLINE, pid, pName ) ;
   // send SIGTERM to process
   rc = kill ( pid, SIGTERM ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed sending SIGTERM to %d, errno=%d",
               pid, errno ) ;
      rc = SDB_SYS ;
      goto error ;
   }
   // wait until process terminate
   while ( round < TERMINATE_TIMEOUT )
   {
      FILE *fp = fopen ( fileNameBuffer, "r" ) ;
      ++round ;
      if ( fp )
      {
         fclose ( fp ) ;
         sleep ( 1 ) ;
         continue ;
      }
      break ;
   }
   if ( TERMINATE_TIMEOUT == round )
   {
      ossPrintf ( "FAILED"OSS_NEWLINE ) ;
      rc = SDB_TIMEOUT ;
   }
   else
      ossPrintf ( "DONE"OSS_NEWLINE ) ;
done :
   PD_TRACE_EXITRC ( SDB_TERMPROC, rc );
   return rc ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_STOPENGINE, "stopEngine" )
void stopEngine ( string serviceName, INT32 &success, INT32 &total )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_STOPENGINE );
   DIR *dirp ;
   struct dirent *dp ;
   CHAR engineName [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   BOOLEAN stopAll = FALSE ;
   if ( !serviceName.empty () )
   {
      ossSnprintf ( engineName, OSS_MAX_PATHSIZE, ENGINE_NAME_PATTERN,
                    serviceName.c_str() ) ;
   }
   else
   {
      stopAll = TRUE ;
   }
   if ( ( dirp = opendir ( "/proc" )) == NULL )
   {
      PD_LOG ( PDERROR, "Failed to open /proc, errno = %d",
               ossGetLastError () ) ;
      goto error ;
   }
   do
   {
      if ( ( dp = readdir ( dirp ) ) != NULL )
      {
         FILE *fp = NULL ;
         CHAR *p = NULL ;
         pid_t pid ;
         CHAR pathName [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
         CHAR commandLine [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
         CHAR tempName [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
         ossSnprintf ( pathName, OSS_MAX_PATHSIZE, "/proc/%s/cmdline",
                       dp->d_name ) ;
         fp = fopen ( pathName, "r" ) ;
         if ( !fp )
         {
            // we do not care if we can't open the file
            continue ;
         }
         if ( NULL == fgets ( commandLine, OSS_MAX_PATHSIZE, fp ) )
         {
            // we do not care if the file is empty (even thou it shouldn't
            // happen )
            fclose ( fp ) ;
            continue ;
         }
         fclose ( fp ) ;
         if ( stopAll )
         {
            // if we ask to stop all engines in the box, we don't need to test
            // specific engine name
            if ( NULL == ( p = ossStrstr ( commandLine, ENGINE_NAME_PATTERN1 )))
            {
               // if it doens't include sequoiadb(, we just continue
               continue ;
            }
            if ( NULL == ( p = ossStrstr ( p, ENGINE_NAME_PATTERN2 ) ) ||
                 ossStrlen ( p ) != 1 )
            {
               // if it's not end with ), let's continue
               continue ;
            }
         }
         else
         {
            // if we were told to stop specific service name, so we have to
            // compare each process name with engineName
            if ( NULL == ( p = ossStrstr ( commandLine, engineName ) ) )
            {
               // if it's not including what we need, let's continue
               continue ;
            }
            if ( ossStrlen ( p ) != ossStrlen ( engineName ) )
            {
               // if it's not the final command ( part of path ), let's skip
               continue ;
            }
         }
         // get pid
         pid = atoi ( dp->d_name ) ;
         // verify
         ossSnprintf ( tempName, OSS_MAX_PATHSIZE, "%d", pid ) ;
         if ( ossStrncmp ( tempName, dp->d_name, OSS_MAX_PATHSIZE ) == 0 )
         {
            rc = terminateProcess ( pid, commandLine ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to terminate process %d, rc = %d",
                        pid, rc ) ;
            }
            else
            {
               ++success ;
            }
            ++total ;
         }
      }
   } while ( dp != NULL ) ;
   closedir ( dirp ) ;
done :
   PD_TRACE1 ( SDB_STOPENGINE, PD_PACK_INT(rc) );
   PD_TRACE_EXIT ( SDB_STOPENGINE );
   return ;
error :
   goto done ;
}

#elif defined (_WINDOWS)
// PD_TRACE_DECLARE_FUNCTION ( SDB_CNVPIPE2NAME, "convertPipeToName" )
void convertPipeToName ( const CHAR *pPipeName, CHAR *pName, INT32 len )
{
   PD_TRACE_ENTRY ( SDB_CNVPIPE2NAME );
   vector<string> splitedString ;
   INT32 strLen = ossStrlen(pPipeName)+1 ;
   CHAR *pContext = NULL ;
   // free by end of the function
   CHAR *pBuffer = (CHAR*)SDB_OSS_MALLOC ( strLen ) ;
   if ( !pBuffer )
   {
      ossPrintf ( "Cannot allocate memory for %d bytes"OSS_NEWLINE,
                  strLen ) ;
      goto error  ;
   }
   ossMemcpy ( pBuffer, pPipeName, strLen ) ;
   CHAR *p = ossStrtok ( pBuffer, ENGINE_NPIPE_PATTERN_SEP, &pContext ) ;
   while ( p )
   {
      string ts ( p ) ;
      splitedString.push_back ( trim ( ts ) ) ;
      p = ossStrtok ( NULL, ENGINE_NPIPE_PATTERN_SEP, &pContext ) ;
   }
   ossMemset ( pName, 0, len ) ;
   if ( splitedString.size() == 3 )
   {
      // if the type is xxxx_yyyy_zzzz
      // first let's check yyyy
      if ( splitedString[1] == ENGINE_NPIPE_PATTERN2 )
      {
         // so we are in engine, the expected string should be xxxx(zzzz)
         ossSnprintf ( pName, len, "%s(%s)", splitedString[0].c_str(),
                                             splitedString[2].c_str() ) ;
      }
   }
done :
   if ( pBuffer )
   {
      SDB_OSS_FREE ( pBuffer ) ;
      pBuffer = NULL ;
   }
   PD_TRACE_EXIT ( SDB_CNVPIPE2NAME );
   return ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_TERMPROC2, "terminateProcess" )
INT32 terminateProcess ( const CHAR *pPipeName )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_TERMPROC2 );
   OSSNPIPE handle ;
   INT32 round = 0 ;
   CHAR localBuf [ OSS_NPIPE_MAX_NAME_LEN + 1 ] = { 0 } ;
   convertPipeToName ( pPipeName, localBuf, sizeof(localBuf) ) ;
   ossPrintf ( "Terminating engine service %s"OSS_NEWLINE, localBuf ) ;
   rc = ossOpenNamedPipe ( pPipeName, OSS_NPIPE_DUPLEX | OSS_NPIPE_BLOCK,
                           OSS_NPIPE_INFINITE_TIMEOUT,
                           handle ) ;
   if ( rc && SDB_FE != rc )
   {
      PD_LOG ( PDERROR, "Failed to create named pipe: %s, rc %d",
               pPipeName, rc ) ;
      goto error ;
   }

   rc = ossWriteNamedPipe ( handle, ENGINE_NPIPE_MSG_SHUTDOWN,
                            sizeof(ENGINE_NPIPE_MSG_SHUTDOWN),
                            NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to send "ENGINE_NPIPE_MSG_SHUTDOWN" to %s "
               "rc = %d", pPipeName, rc ) ;
   }
   rc = ossCloseNamedPipe ( handle ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to close named pipe" ) ;
      goto error ;
   }
   // wait until process terminate
   while ( round < TERMINATE_TIMEOUT )
   {
      vector<string> names ;
      rc = ossEnumNamedPipes ( names, pPipeName ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to enum named pipes, rc = %d", rc ) ;
         goto error ;
      }
      if ( names.size() == 0 )
         break ;
      ++round ;
      // sleep for 1 second every round
      ossSleepsecs ( 1 ) ;
   }
   if ( TERMINATE_TIMEOUT == round )
   {
      ossPrintf ( "FAILED"OSS_NEWLINE ) ;
      rc = SDB_TIMEOUT ;
   }
   else
      ossPrintf ( "DONE"OSS_NEWLINE ) ;
done :
   PD_TRACE_EXITRC ( SDB_TERMPROC2, rc );
   return rc ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_STOPENGINE2, "stopEngine" )
void stopEngine ( string serviceName, INT32 &success, INT32 &total )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_STOPENGINE2 );
   vector<string> names ;
   CHAR enginePipeName [ OSS_NPIPE_MAX_NAME_LEN + 1 ] = {0} ;
   BOOLEAN stopAll = FALSE ;
   rc = ossEnumNamedPipes ( names, NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to enum named pipes, rc = %d", rc ) ;
      goto error ;
   }

   if ( !serviceName.empty () )
   {
      ossSnprintf ( enginePipeName, OSS_NPIPE_MAX_NAME_LEN,
                    ENGINE_NPIPE_PATTERN, serviceName.c_str() ) ;
   }
   else
   {
      ossSnprintf ( enginePipeName, OSS_NPIPE_MAX_NAME_LEN,
                    ENGINE_NPIPE_PATTERN, "" ) ;
      stopAll = TRUE ;
   }
   for ( INT32 i = 0; i < names.size(); ++i )
   {
      const CHAR *pName = names[i].c_str() ;
      // if it matches the pattern
      // stopAll: "sequoiadb_engine_"
      // otherwise: "sequoiadb_engine_<servicename>"
      if ( ossStrncmp ( pName, enginePipeName,
                        ossStrlen ( enginePipeName ) ) == 0 )
      {
         // in stopAll mode, we terminate process as long as it matches pattern
         // in stop specific service mode, make sure the length of pName
         //   and enginePipeName are the same ( if strlen same, and strncmp =0,
         //   then it's safe to say the string are the same )
         if ( stopAll || ossStrlen ( pName ) == ossStrlen ( enginePipeName ) )
         {
            total ++ ;
            rc = terminateProcess ( pName ) ;
            if ( !rc )
            {
               success ++ ;
            }
         }
      }
   }
done :
   PD_TRACE1 ( SDB_STOPENGINE2, PD_PACK_INT(rc) );
   PD_TRACE_EXIT ( SDB_STOPENGINE2 );
   return ;
error :
   goto done ;
}
#endif

// PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTOP_MAIN, "main" )
INT32 main ( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBSTOP_MAIN );
   INT32 success = 0 ;
   INT32 total = 0 ;
   po::options_description desc ( "Command options" ) ;
   init ( desc ) ;
   // validate arguments
   rc = resolveArgument ( desc, argc, argv ) ;
   if ( rc )
   {
      if ( SDB_PMD_HELP_ONLY != rc &&
           SDB_PMD_VERSION_ONLY != rc )
      {
         PD_LOG ( PDERROR, "Invalid argument" ) ;
         displayArg ( desc ) ;
      }
      goto done ;
   }

   if ( 0 == stopServices.size() )
   {
      stopEngine ( "", success, total ) ;
   }
   else
   {
      for ( UINT32 i = 0; i < stopServices.size(); ++i )
      {
         stopEngine ( stopServices[i], success, total ) ;
      }
   }
   ossPrintf ( "Total: %d; Success: %d; Failed: %d"OSS_NEWLINE,
               total, success, total - success ) ;
done :
   if ( total == success )
      rc = SDB_OK ;
   else if ( success == 0)
      rc = STOPFAIL ;
   else
      rc = STOPPART ;
   PD_TRACE_EXITRC ( SDB_SDBSTOP_MAIN, rc );
   return rc ;
}

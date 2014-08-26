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

   Source File Name = sdbstart.cpp

   Descriptive Name = sdbstart Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbstart,
   which is used to start SequoiaDB engine.

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
#include "ossProc.hpp"
#include "ossNPipe.hpp"
#include "ossNPipe.hpp"
#include "pmdDef.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "utilCommon.hpp"
#include "pmdOptions.h"
#include "utilParam.hpp"
#include "ossVer.h"

#include <string>
#include <iostream>
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

#define COMMANDS_OPTIONS \
       ( PMD_COMMANDS_STRING(PMD_OPTION_HELP, ",h"), "help" ) \
       ( PMD_OPTION_VERSION, "version" ) \
       ( PMD_COMMANDS_STRING(PMD_OPTION_CONFPATH, ",c"), boost::program_options::value<string>(), "configure file path" )

// initialize options
void init ( po::options_description &desc )
{
   PMD_ADD_PARAM_OPTIONS_BEGIN ( desc )
      COMMANDS_OPTIONS
   PMD_ADD_PARAM_OPTIONS_END
}

void displayArg ( po::options_description &desc )
{
   std::cout << desc << std::endl ;
}

#if defined (_WINDOWS)
BOOLEAN checkProcess ( const CHAR *pPipeName, OSSPID expPid ) ;
#endif

// PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTART_SERVICEEXISTS, "serviceExists" )
BOOLEAN serviceExists ( const CHAR *pServiceName  )
{
   BOOLEAN exists = FALSE ;
   PD_TRACE_ENTRY ( SDB_SDBSTART_SERVICEEXISTS ) ;
// in linux, we iterate /proc file system and find if any process get cmdline
// matches the service name
#if defined (_LINUX)
   DIR *dirp ;
   struct dirent *dp ;
   CHAR engineName [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   ossSnprintf ( engineName, OSS_MAX_PATHSIZE, ENGINE_NAME_PATTERN,
                 pServiceName ) ;
   if ( ( dirp = opendir ( "/proc" ) ) == NULL )
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
         CHAR pathName [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
         CHAR commandLine [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
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
         // if match, let's mark exists
         if ( ossStrcmp ( commandLine, engineName ) == 0 )
         {
            exists = TRUE ;
            ossPrintf ( "Success: SequoiaDB engine is already "
                        "started (%s)"OSS_NEWLINE, dp->d_name ) ;
            break ;
         }
      }
   } while ( dp != NULL ) ;
   closedir ( dirp ) ;
// in windows, we enumerate all pipes and compare the pipe name
#elif defined (_WINDOWS)
   INT32 rc = SDB_OK ;
   OSSPID pid ;
   vector<string> names ;
   CHAR enginePipeName [ OSS_NPIPE_MAX_NAME_LEN + 1 ] = {0} ;
   rc = ossEnumNamedPipes ( names, NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to enum named pipes, rc = %d", rc ) ;
      goto error ;
   }
   ossSnprintf ( enginePipeName, OSS_NPIPE_MAX_NAME_LEN,
                 ENGINE_NPIPE_PATTERN, pServiceName ) ;
   for ( INT32 i = 0; i < names.size(); ++i )
   {
      const CHAR *pName = names[i].c_str() ;
      if ( ossStrcmp ( pName, enginePipeName ) == 0 )
      {
         exists = TRUE ;
         if ( checkProcess ( pName, pid ) )
         {
            ossPrintf ( "Success: SequoiaDB engine is already "
                        "started (%d)"OSS_NEWLINE, pid ) ;
            goto done ;
         }
         break ;
      }
   }
#endif
done :
   PD_TRACE_EXIT ( SDB_SDBSTART_SERVICEEXISTS ) ;
   return exists ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTART_CHECKSTARTEDSERVICES, "checkStartedServices" )
INT32 checkStartedServices ( const CHAR *pConfPath )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBSTART_CHECKSTARTEDSERVICES ) ;
   po::options_description desc ;
   po::variables_map vm ;
   desc.add_options()
      ( PMD_OPTION_SVCNAME, boost::program_options::value<string>(), "" ) ;
   CHAR conf[OSS_MAX_PATHSIZE + 1] = {0} ;
   rc = engine::utilBuildFullPath ( pConfPath, PMD_DFT_CONF,
                                    OSS_MAX_PATHSIZE + 1, conf ) ;
   if ( rc )
   {
      std::cerr << "Failed to build full path, rc = " << rc << std::endl ;
      goto error ;
   }

   rc = engine::utilReadConfigureFile( conf, desc, vm ) ;
   if ( SDB_IO == rc )
   {
      // if the file does not exist, let's just continue so that sequoiadb is
      // able to create the file
      rc = SDB_OK ;
      goto done ;
   }
   if ( rc )
   {
      goto error ;
   }

   if ( vm.count ( PMD_OPTION_SVCNAME ) )
   {
      const CHAR *pServiceName =
            vm [ PMD_OPTION_SVCNAME ].as<string>().c_str() ;
      // if service already exists, let's just return duplicate service error
      if ( serviceExists ( pServiceName ) )
      {
         rc = SDB_DUPLICATED_SERVICE ;
         goto error ;
      }
   }
done :
   PD_TRACE_EXITRC ( SDB_SDBSTART_CHECKSTARTEDSERVICES, rc ) ;
   return rc ;
error :
   goto done ;
}

// in this function we only validate whether there's unexpected input, we do not
// actually doing anything. We will simply pass all parameters to sequoiadb
// engine
// PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTART_RESVARG, "resolveArgument" )
INT32 resolveArgument ( po::options_description &desc, INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBSTART_RESVARG );
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
      ossPrintVersion( "SDB Start Version" ) ;
      rc = SDB_PMD_VERSION_ONLY ;
      goto done ;
   }

   if ( vm.count ( PMD_OPTION_CONFPATH ) )
   {
      const CHAR *pConfPath = vm[PMD_OPTION_CONFPATH].as<string>().c_str() ;
      PD_LOG ( PDDEBUG, "confpath=%s", pConfPath ) ;
      rc = checkStartedServices ( pConfPath ) ;
   }

   // we do not actually doing anything, we just parse the input parameter and
   // forward the arguments to server
done :
   PD_TRACE_EXITRC ( SDB_SDBSTART_RESVARG, rc );
   return rc ;
error :
   goto done ;
}

// resolve engine path name from a given input path ( not full path name )
// Basically we assume the input path is <xxx>/something, where <xxx> is the
// path where sequoiadb locates
// PD_TRACE_DECLARE_FUNCTION ( SDB_ENGINEPATH, "enginePath" )
CHAR *enginePath ( const CHAR *pInputPath, CHAR *pOutputPath )
{
   PD_TRACE_ENTRY ( SDB_ENGINEPATH );
   // find '\\' or '/'
   CHAR *t = OSS_FILE_SEP ;
   const CHAR *p = ossStrrchr ( pInputPath, t[0] ) ;
   // if we can find it
   if ( p )
   {
      // let's move to the next character after '\\' or '/'
      if ( p - pInputPath + ossStrlen ( ENGINE_NAME ) + 1 > OSS_MAX_PATHSIZE )
      {
         PD_LOG ( PDERROR, "Path is too long: %s", pInputPath ) ;
         return NULL ;
      }
      // copy everything before '\\' or '/' to output
      ossMemcpy ( pOutputPath, pInputPath, p-pInputPath+1 ) ;
      // concat engine name
      ossStrncat ( pOutputPath, ENGINE_NAME, OSS_MAX_PATHSIZE ) ;
   }
   else
   {
      // if we can't find path spliter
      SDB_ASSERT ( ossStrlen ( ENGINE_NAME ) <= OSS_MAX_PATHSIZE,
                   "Engine name is too long: "ENGINE_NAME ) ;
      // let's just build engine name
      ossStrncpy ( pOutputPath, ENGINE_NAME, OSS_MAX_PATHSIZE ) ;
   }
   PD_TRACE1 ( SDB_ENGINEPATH, PD_PACK_STRING(pOutputPath) );
   PD_TRACE_EXIT ( SDB_ENGINEPATH );
   return pOutputPath ;
}

// calculate how much space we need after using engine name
// PD_TRACE_DECLARE_FUNCTION ( SDB_CALCBUFFSIZE, "calcBufferSize" )
INT32 calcBufferSize ( INT32 argc, CHAR **argv )
{
   PD_TRACE_ENTRY ( SDB_CALCBUFFSIZE );
   INT32 totalSize = 0 ;
   CHAR pathBuffer [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   // get the path name for engine
   CHAR *progName = enginePath ( argv[0], pathBuffer ) ;
   if ( !progName )
   {
      PD_LOG ( PDERROR, "Failed to get engine path" ) ;
      totalSize = -1 ;
      goto done ;
   }
   PD_LOG ( PDDEBUG, "progName = %s\n", progName ) ;
   totalSize += ossStrlen ( progName ) + 1 ;
   // count size of rest arguments
   for ( INT32 i = 1; i < argc; ++i )
   {
      PD_LOG ( PDDEBUG, "argv[%d] = %s\n", i, argv[i] ) ;
      totalSize += ossStrlen ( argv[i] ) + 1 ;
   }
   totalSize ++ ;
done :
   PD_TRACE1 ( SDB_CALCBUFFSIZE, PD_PACK_INT(totalSize) );
   PD_TRACE_EXIT ( SDB_CALCBUFFSIZE );
   return totalSize ;
}

// copy over buffer from input argument. Here we are going to use engine path
// name plus other input parameters
// Note engine path and parameters are all sitting in the same buffer. Each
// arguments are separated by '\0'. Two adjcent '\0\0' represent end of buffer
// PD_TRACE_DECLARE_FUNCTION ( SDB_COPYBUFFER, "copyBuffer" )
void copyBuffer ( CHAR *pBuffer, INT32 bufSize, INT32 argc, CHAR **argv )
{
   PD_TRACE_ENTRY ( SDB_COPYBUFFER );
   INT32 pos = 0 ;
   CHAR pathBuffer [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   CHAR *progName = enginePath ( argv[0], pathBuffer ) ;
   SDB_ASSERT ( progName, "progName can't be NULL" ) ;

   // copy program name to buffer
   ossStrncpy ( &pBuffer[0], progName, bufSize ) ;
   pos += ossStrlen ( progName ) ;
   pBuffer[pos] = '\0' ;
   ++pos ;

   // copy other arguments
   for ( INT32 i = 1; i < argc; ++i )
   {
      SDB_ASSERT ( pos < bufSize, "invalid position" ) ;
      ossStrncpy ( &pBuffer[pos], argv[i], bufSize - pos ) ;
      pos += ossStrlen ( argv[i] ) ;
      // each arguments are separated by '\0'
      pBuffer[pos] = '\0' ;
      ++pos ;
   }
   // add another '\0' for end of input
   pBuffer[pos] = '\0' ;
   ++pos ;
   PD_TRACE_EXIT ( SDB_COPYBUFFER );
   SDB_ASSERT ( pos == bufSize, "invalid position" ) ;
}

//#define PROC_START_TIMEOUT 10

// PD_TRACE_DECLARE_FUNCTION ( SDB_CHKPROC, "checkProcess" )
#if defined (_WINDOWS)
BOOLEAN checkProcess ( const CHAR *pPipeName, OSSPID expPid )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CHKPROC );
   BOOLEAN ret = FALSE ;
   OSSNPIPE handle ;
   OSSPID pid ;
   INT64 readSize = 0 ;
   rc = ossOpenNamedPipe ( pPipeName, OSS_NPIPE_DUPLEX | OSS_NPIPE_BLOCK,
                           OSS_NPIPE_BLOCK_WITH_TIMEOUT,
                           handle ) ;
   if ( rc && SDB_FE != rc )
   {
      PD_LOG ( PDERROR, "Failed to create named pipe: %s, rc %d",
               pPipeName, rc ) ;
      ret = FALSE ;
      goto error ;
   }

   rc = ossWriteNamedPipe ( handle, ENGINE_NPIPE_MSG_PID,
                            sizeof(ENGINE_NPIPE_MSG_PID),
                            NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to send "ENGINE_NPIPE_MSG_PID" to %s "
               "rc = %d", pPipeName, rc ) ;
   }
   rc = ossReadNamedPipe ( handle, (CHAR*)&pid, sizeof(pid), &readSize,
                           LIST_TIMEOUT ) ;
   if ( rc || ( readSize != sizeof(pid) ) )
   {
      ossPrintf ( "Failed to read pid from pipe %s"OSS_NEWLINE,
                  pPipeName ) ;
   }
   rc = ossCloseNamedPipe ( handle ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to close named pipe" ) ;
      goto error ;
   }
   if ( pid == expPid )
      ret = TRUE ;
done :
   PD_TRACE1 ( SDB_CHKPROC, PD_PACK_INT(ret) );
   PD_TRACE_EXIT ( SDB_CHKPROC );
   return ret ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_FINDENGINE, "findEngine" )
INT32 findEngine ( OSSPID pid )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_FINDENGINE );
   PD_TRACE1 ( SDB_FINDENGINE, PD_PACK_INT(pid) );
   vector<string> names ;
   INT32 round = 0 ;
   CHAR enginePipeName [ OSS_NPIPE_MAX_NAME_LEN + 1 ] = {0} ;
   ossSnprintf ( enginePipeName, OSS_NPIPE_MAX_NAME_LEN,
                 ENGINE_NPIPE_PATTERN, "" ) ;

   //while ( round < PROC_START_TIMEOUT )
   // we do not timeout. Because in crash recovery mode we may stay in recovery
   // state for long time, in this case we should keep waiting until it success
   // or fail
   while ( TRUE )
   {
      rc = ossEnumNamedPipes ( names, NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to enum named pipes, rc = %d", rc ) ;
         goto error ;
      }

      for ( UINT32 i = 0; i < names.size(); ++i )
      {
         const CHAR *pName = names[i].c_str() ;
         // if it matches the pattern
         // "sequoiadb_engine_"
         if ( ossStrncmp ( pName, enginePipeName,
                           ossStrlen ( enginePipeName ) ) == 0 )
         {
            if ( checkProcess ( pName, pid ) )
            {
               ossPrintf ( "Success: SequoiaDB engine is successfully "
                           "started (%d)"OSS_NEWLINE, pid ) ;
               goto done ;
            }
         }
      }
      if ( !ossIsProcessRunning ( pid ) )
      {
         ossPrintf ( "Error: Unable to start SequoiaDB engine"OSS_NEWLINE ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      ++round ;
      // sleep for 1 second every round
      ossSleepsecs ( 1 ) ;
   }

   /*if ( round == PROC_START_TIMEOUT )
   {
      ossPrintf ( "Error: Starting SequoiaDB engine timeout (%d)"OSS_NEWLINE,
                  pid ) ;
      rc = SDB_SYS ;
      goto error ;
   }*/
done :
   PD_TRACE_EXITRC ( SDB_FINDENGINE, rc );
   return rc ;
error :
   goto done ;
}
#endif

// PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTART_MAIN, "main" )
INT32 main ( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBSTART_MAIN );
   CHAR *pArgumentBuffer = NULL ;
   INT32 bufferSize = 0 ;
   po::options_description desc ( "Command options" ) ;
   OSSPID pid ;
   ossResultCode result ;
   init ( desc ) ;
   // validate arguments
   rc = resolveArgument ( desc, argc, argv ) ;
   if ( rc )
   {
      // if the service is already running, let's just return
      if ( SDB_DUPLICATED_SERVICE == rc )
      {
         // we consider the startup success when the service is already running
         rc = SDB_OK ;
      }
      else if ( SDB_PMD_HELP_ONLY != rc &&
                SDB_PMD_VERSION_ONLY != rc )
      {
         PD_LOG ( PDERROR, "Invalid argument" ) ;
         displayArg ( desc ) ;
      }
      goto done ;
   }
   // estimate the size of final buffer
   bufferSize = calcBufferSize ( argc, argv ) ;
   if ( bufferSize <= 0 )
   {
      PD_LOG ( PDERROR, "Failed to calculate buffer size" ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // allocate buffer
   pArgumentBuffer = (CHAR*)SDB_OSS_MALLOC ( bufferSize ) ;
   if ( !pArgumentBuffer )
   {
      PD_LOG ( PDERROR, "Failed to allocate buffer for %d bytes",
               bufferSize ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   // copy arguments into buffer
   copyBuffer ( pArgumentBuffer, bufferSize, argc, argv ) ;

   // call exec to run the command with arguments, do NOT wait until program
   // finish
   rc = ossExec ( pArgumentBuffer, pArgumentBuffer, NULL,
                  0, pid, result, NULL, NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to execute engine, rc = %d", rc ) ;
      goto error ;
   }

#if defined (_LINUX)
   // in linux, creating process is separated by two steps: fork + exec
   // If we do not have OSS_EXEC_SSAVE option, we do not wait for child. That
   // means as long as fork completes, we consider the program is started.
   // However that is not the case in real world, because exec() may fail due to
   // different reasons. So we have to use extra logic to verify the process is
   // really started after ossExec.
   // We don't need to do that in Windows, because CreateProcess is able to
   // garentee the process starts and run.
   rc = ossVerifyPID ( pid, MODIFIED_ENGINE_NAME, "SequoiaDB engine" ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to verify PID, rc = %d", rc ) ;
      goto error ;
   }
#elif defined (_WINDOWS)
   findEngine ( pid ) ;
#endif
done :
   if ( pArgumentBuffer )
      SDB_OSS_FREE ( pArgumentBuffer ) ;
   PD_TRACE_EXITRC ( SDB_SDBSTART_MAIN, rc );
   return SDB_OK == rc ? 0 : 1 ;
error :
   goto done ;
}

/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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
#include "sdbcm.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "sdbcommon.hpp"
#include "pmdCommon.hpp"
#include "pmdOptions.h"

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

#define ADD_PARAM_OPTIONS_BEGIN( desc )\
        desc.add_options()

#define ADD_PARAM_OPTIONS_END ;

#define COMMANDS_STRING( a, b ) (string(a) +string( b)).c_str()
#define COMMANDS_OPTIONS \
       ( COMMANDS_STRING(PMD_OPTION_HELP, ",h"),                          "help" )\
       ( COMMANDS_STRING(PMD_OPTION_CONFPATH, ",c"), boost::program_options::value<string>(), "configure file path" )

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

#if defined (_WINDOWS)
BOOLEAN checkProcess ( const CHAR *pPipeName, OSSPID expPid ) ;
#endif

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTART_SERVICEEXISTS, "serviceExists" )
BOOLEAN serviceExists ( const CHAR *pServiceName  )
{
   BOOLEAN exists = FALSE ;
   PD_TRACE_ENTRY ( SDB_SDBSTART_SERVICEEXISTS ) ;
// in linux, we iterate /proc file system and find if any process get cmdline
// matches the service name
#if defined (_LINUX)
   DIR *dirp ;
   struct dirent *dp ;
   CHAR engineName [ PROC_PATH_LEN_MAX + 1 ] = {0} ;
   ossSnprintf ( engineName, PROC_PATH_LEN_MAX, ENGINE_NAME_PATTERN,
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
         CHAR pathName [ PROC_PATH_LEN_MAX + 1 ] = {0} ;
         CHAR commandLine [ PROC_PATH_LEN_MAX + 1 ] = {0} ;
         ossSnprintf ( pathName, PROC_PATH_LEN_MAX, "/proc/%s/cmdline",
                       dp->d_name ) ;
         fp = fopen ( pathName, "r" ) ;
         if ( !fp )
         {
            // we do not care if we can't open the file
            continue ;
         }
         if ( NULL == fgets ( commandLine, PROC_PATH_LEN_MAX, fp ) )
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
   CHAR enginePipeName [ PROC_PIPE_NAME_LEN + 1 ] = {0} ;
   rc = ossEnumNamedPipes ( names, NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to enum named pipes, rc = %d", rc ) ;
      goto error ;
   }
   ossSnprintf ( enginePipeName, PROC_PIPE_NAME_LEN, ENGINE_NPIPE_PATTERN,
                 pServiceName ) ;
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

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTART_CHECKSTARTEDSERVICES, "checkStartedServices" )
INT32 checkStartedServices ( const CHAR *pConfPath )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBSTART_CHECKSTARTEDSERVICES ) ;
   po::options_description desc ;
   po::variables_map vm ;
   desc.add_options()
      ( PMD_OPTION_SVCNAME, boost::program_options::value<string>(), "" ) ;
   CHAR conf[OSS_MAX_PATHSIZE + 1] = {0} ;
   rc = engine::pmdBuildFullPath ( pConfPath, PMD_DFT_CONF,
                                   OSS_MAX_PATHSIZE + 1, conf ) ;
   if ( rc )
   {
      std::cerr << "Failed to build full path, rc = " << rc << std::endl ;
      goto error ;
   }
   try
   {
      po::store ( po::parse_config_file<char> ( conf, desc, TRUE ), vm ) ;
      po::notify ( vm ) ;
   }
   catch ( po::reading_file )
   {
      // if the file does not exist, let's just continue so that sequoiadb is
      // able to create the file
      rc = SDB_OK ;
      goto done ;
   }
   catch ( po::unknown_option &e )
   {
      std::cerr << "Unknown config element: "
                << e.get_option_name () << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   catch ( po::invalid_option_value &e )
   {
      std::cerr << ( std::string ) "Invalid config element: "
                << e.get_option_name () << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   catch( po::error &e )
   {
      std::cerr << e.what () << std::endl ;
      rc = SDB_INVALIDARG ;
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
PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTART_RESVARG, "resolveArgument" )
INT32 resolveArgument ( po::options_description &desc, INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBSTART_RESVARG );
   po::variables_map vm ;
   try
   {
      po::store ( po::command_line_parser ( argc, argv ).options(
                  desc ).allow_unregistered().run(), vm ) ;
      //po::store ( po::parse_command_line ( argc, argv, desc ), vm ) ;
      po::notify ( vm ) ;
   }
   catch ( po::unknown_option &e )
   {
      pdLog ( PDWARNING, __FUNC__, __FILE__, __LINE__,
            ( ( std::string ) "Unknown argument: " +
                e.get_option_name ()).c_str () ) ;
              std::cerr <<  "Unknown argument: "
                        << e.get_option_name () << std::endl ;
              rc = SDB_INVALIDARG ;
      goto error ;
   }
   catch ( po::invalid_option_value &e )
   {
      pdLog ( PDWARNING, __FUNC__, __FILE__, __LINE__,
             ( ( std::string ) "Invalid argument: " +
               e.get_option_name () ).c_str () ) ;
      std::cerr <<  "Invalid argument: "
                << e.get_option_name () << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   catch( po::error &e )
   {
      std::cerr << e.what () << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   if ( vm.count ( PMD_OPTION_HELP ) )
   {
      displayArg ( desc ) ;
      rc = SDB_PMD_HELP_ONLY ;
      goto done ;
   }

   if ( vm.count ( PMD_OPTION_CONFPATH ) )
   {
      const CHAR *pConfPath = vm[PMD_OPTION_CONFPATH].as<string>().c_str() ;
      PD_LOG ( PDDEBUG, "confpath=%s",
               pConfPath ) ;
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
PD_TRACE_DECLARE_FUNCTION ( SDB_ENGINEPATH, "enginePath" )
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
                   "Engine name is too long: "ENGINE_NAME )
      // let's just build engine name
      ossStrncpy ( pOutputPath, ENGINE_NAME, OSS_MAX_PATHSIZE ) ;
   }
   PD_TRACE1 ( SDB_ENGINEPATH, PD_PACK_STRING(pOutputPath) );
   PD_TRACE_EXIT ( SDB_ENGINEPATH );
   return pOutputPath ;
}

// calculate how much space we need after using engine name
PD_TRACE_DECLARE_FUNCTION ( SDB_CALCBUFFSIZE, "calcBufferSize" )
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
PD_TRACE_DECLARE_FUNCTION ( SDB_COPYBUFFER, "copyBuffer" )
void copyBuffer ( CHAR *pBuffer, INT32 bufSize, INT32 argc, CHAR **argv )
{
   PD_TRACE_ENTRY ( SDB_COPYBUFFER );
   INT32 pos = 0 ;
   CHAR pathBuffer [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   CHAR *progName = enginePath ( argv[0], pathBuffer ) ;
   SDB_ASSERT ( progName, "progName can't be NULL" )

   // copy program name to buffer
   ossStrncpy ( &pBuffer[0], progName, bufSize ) ;
   pos += ossStrlen ( progName ) ;
   pBuffer[pos] = '\0' ;
   ++pos ;

   // copy other arguments
   for ( INT32 i = 1; i < argc; ++i )
   {
      SDB_ASSERT ( pos < bufSize, "invalid position" )
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
   SDB_ASSERT ( pos == bufSize, "invalid position" )
}

//#define PROC_START_TIMEOUT 10
#if defined (_LINUX)
PD_TRACE_DECLARE_FUNCTION ( SDB_VERIFYPID, "verifyPID" )
INT32 verifyPID ( OSSPID inputpid, const CHAR *engineName )
{
   INT32 rc                                   = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_VERIFYPID );
   INT32 numScaned                            = 0 ;
   INT32 pid                                  = 0 ;
   INT32 ppid                                 = 0 ;
   CHAR procName [ PROC_TEMP_BUF_SZ ]         = {0} ;
   CHAR status [ PROC_TEMP_BUF_SZ ]           = {0} ;
   CHAR pathName [ PROC_PATH_LEN_MAX + 1 ]    = {0} ;
   CHAR pathName1 [ PROC_PATH_LEN_MAX + 1 ]   = {0} ;
   CHAR commandLine [ PROC_PATH_LEN_MAX + 1 ] = {0} ;
   BOOLEAN loop                               = TRUE ;
   INT32 round                                = 0 ;
   // since we are single-thread program, it's safe to use FILE
   FILE *fp                                   = NULL ;
   FILE *fp1                                  = NULL ;
   // read /proc/pid/stat can get both pid and ppid
   ossSnprintf ( pathName, PROC_PATH_LEN_MAX, "/proc/%d/stat", inputpid ) ;
   ossSnprintf ( pathName1, PROC_PATH_LEN_MAX, "/proc/%d/cmdline", inputpid ) ;
   //while ( round < PROC_START_TIMEOUT && loop )
   // we do not timeout. Because in crash recovery mode we may stay in recovery
   // state for long time, in this case we should keep waiting until it success
   // or fail
   while ( loop )
   {
      // open proc/pid/stat file
      fp = fopen ( pathName, "r" ) ;
      fp1 = fopen ( pathName1, "r" ) ;
      if ( fp && fp1 )
      {
         // get first 4 elements
         numScaned = fscanf ( fp, "%d%s%s%d",
                              &pid, // process pid
                              procName, // process name
                              status, // process status
                              &ppid ) ; // parent process id
         // if we can't read 4 elements, something wrong
         if ( 4 == numScaned )
         {
            // if we detected zombie process, let's get out of here. Since we
            // have disabled SIGCHLD, so if fork() success but exec() fail, we
            // are going to get zombie status in child process
            if ( status[0] == PROC_STATUS_ZOMBIE )
            {
               ossPrintf ( "Error: Failed to start SequoiaDB engine"
                           OSS_NEWLINE ) ;
               loop = FALSE ;
            }
            // make sure
            // 1) pid matches what we want
            // 2) parent pid matches myself
            // 3) sequoiadb engine name is part of the process name ( after exec
            //    successfully run )
            else if ( pid == inputpid && getpid() == ppid )
            {
               if ( NULL != fgets ( commandLine, PROC_PATH_LEN_MAX, fp1 ) &&
                    ossStrstr ( commandLine, engineName ) )
               {
                  ossPrintf ( "Success: SequoiaDB engine is successfully "
                              "started (%d)"OSS_NEWLINE, pid ) ;
                  loop = FALSE ;
               }
            }
         }
         // if we can't read 4 elements, something wrong
         else
         {
            ossPrintf ( "Error: Failed to extract process information"
                        OSS_NEWLINE ) ;
            rc = SDB_SYS ;
            loop = FALSE ;
         }
      }
      else
      {
         // if we can't open the /proc/<pid>/stat, the child process is gone (
         // it should never happen thou )
         ossPrintf ( "Error: Unable to start SequoiaDB engine"OSS_NEWLINE ) ;
         rc = SDB_SYS ;
         loop = FALSE ;
      }
      fclose ( fp ) ;
      fclose ( fp1 ) ;
      fp = NULL ;
      fp1 = NULL ;
      ++round ;
      // sleep for 1 second every round
      sleep ( 1 ) ;
   }
   // if we still can't have a match after timeout period, let's dump error
   /*if ( round == PROC_START_TIMEOUT )
   {
      ossPrintf ( "Error: Starting SequoiaDB engine timeout (%d)"OSS_NEWLINE,
                  inputpid ) ;
      rc = SDB_SYS ;
   }*/
   PD_TRACE_EXITRC ( SDB_VERIFYPID, rc );
   return rc ;
}
#elif defined (_WINDOWS)
PD_TRACE_DECLARE_FUNCTION ( SDB_CHKPROC, "checkProcess" )
BOOLEAN checkProcess ( const CHAR *pPipeName, OSSPID expPid )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CHKPROC );
   BOOLEAN ret = FALSE ;
   OSSNPIPE handle ;
   OSSPID pid ;
   INT64 readSize = 0 ;
   INT32 round = 0 ;
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

PD_TRACE_DECLARE_FUNCTION ( SDB_FINDENGINE, "findEngine" )
INT32 findEngine ( OSSPID pid )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_FINDENGINE );
   PD_TRACE1 ( SDB_FINDENGINE, PD_PACK_INT(pid) );
   vector<string> names ;
   INT32 round = 0 ;
   CHAR enginePipeName [ PROC_PIPE_NAME_LEN + 1 ] = {0} ;
   ossSnprintf ( enginePipeName, PROC_PIPE_NAME_LEN, ENGINE_NPIPE_PATTERN,
                 "" ) ;

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

      for ( INT32 i = 0; i < names.size(); ++i )
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

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTART_MAIN, "main" )
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
      else if ( SDB_PMD_HELP_ONLY != rc )
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
   rc = verifyPID ( pid, MODIFIED_ENGINE_NAME ) ;
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
   return rc ;
error :
   goto done ;
}

/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sdbcmart.cpp

   Descriptive Name = sdbcmart Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcmStart,
   which is used to start SequoiaDB Cluster Manager.

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
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "sptCommon.hpp"
#include <string>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

using namespace std;
extern CHAR _pdDiagLogPath[OSS_MAX_PATHSIZE+1] ;
namespace po = boost::program_options;

#define OPTION_HELP "help"
#define OPTION_CONFPATH SDBCM_OPTION_START_CONFPATH

#define ADD_PARAM_OPTIONS_BEGIN( desc )\
        desc.add_options()

#define ADD_PARAM_OPTIONS_END ;

#define COMMANDS_STRING( a, b ) (string(a) +string( b)).c_str()
#define COMMANDS_OPTIONS \
       ( COMMANDS_STRING(OPTION_HELP, ",h"),                          "help" )

#if defined (_LINUX)
#define SDBCM_NAME_BUF_LEN          255
#define SDBCM_NAME_PATTERN1         "sdbcm("
#define SDBCM_NAME_PATTERN2         ")"
#define SDBCM_DMN_SVC_NAME          "sdbcmd"
#endif

#define SDBCM_LOG_PATH              ".." OSS_FILE_SEP "conf" OSS_FILE_SEP "log" \
                                    OSS_FILE_SEP "sdbcmd.log"
#define SDBCM_NAME "sdbcm"
#define SDBCM_DMN_PROC_NAME         "sdbcmd"

// initialize options
void init ( po::options_description &desc )
{
   ADD_PARAM_OPTIONS_BEGIN ( desc )
      COMMANDS_OPTIONS
   ADD_PARAM_OPTIONS_END
}

void displayArg ( po::options_description &desc )
{
   std::cout << "Usage:  sdbcmart [OPTION]" <<std::endl;
   std::cout << desc << std::endl ;
}

// in this function we only validate whether there's unexpected input, we do not
// actually doing anything. We will simply pass all parameters to sequoiadb
// engine
PD_TRACE_DECLARE_FUNCTION ( SDB_CMSTART_RSVARG, "resolveArgument" )
INT32 resolveArgument ( po::options_description &desc, INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CMSTART_RSVARG );
   po::variables_map vm ;
   try
   {
      po::store ( po::parse_command_line ( argc, argv, desc ), vm ) ;
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

   if ( vm.count ( OPTION_HELP ) )
   {
      displayArg ( desc ) ;
      rc = SDB_PMD_HELP_ONLY ;
      goto done ;
   }

   // we do not actually doing anything, we just parse the input parameter and
   // forward the arguments to server
done :
   PD_TRACE_EXITRC ( SDB_CMSTART_RSVARG, rc );
   return rc ;
error :
   goto done ;
}

#if defined (_WINDOWS)
#include <windows.h>
#define SDBCMDMN_SRV_NAME "sdbcmd"

PD_TRACE_DECLARE_FUNCTION ( SDB_STARTSDBCM, "startSdbcm" )
INT32 startSdbcm ( list<const CHAR*> &argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_STARTSDBCM );
   SERVICE_STATUS srvStatus ;
   SC_HANDLE schSCM = NULL, schSRV = NULL ;

   // open a handle to the sc manager database
   schSCM = OpenSCManager ( NULL, NULL, SC_MANAGER_CONNECT ) ;
   if ( schSCM == NULL )
   {
      rc = SDB_SYS ;
      PD_LOG ( PDERROR, "Failed to open SCM" );
      goto error ;
   }

   // open a handle to the sdbcm service
   schSRV = OpenService ( schSCM,
                          TEXT(SDBCMDMN_SRV_NAME),
                          SERVICE_START | SERVICE_QUERY_STATUS ) ;
   if ( schSRV == NULL )
   {
      rc = SDB_SYS ;
      PD_LOG ( PDERROR, "Failed to open service" );
      goto error ;
   }
   QueryServiceStatus ( schSRV, &srvStatus ) ;
   if ( srvStatus.dwCurrentState == SERVICE_RUNNING )
      goto done ;
   else if ( ! StartService ( schSRV, 0, NULL ) )
   {
      rc = SDB_SYS ;
      PD_LOG ( PDERROR, "Failed to set service status RUNNING" );
      goto error ;
   }

done :
   if ( schSCM )
      CloseServiceHandle ( schSCM ) ;
   if ( schSRV )
      CloseServiceHandle ( schSRV ) ;
   PD_TRACE_EXITRC ( SDB_STARTSDBCM, rc );
   return rc ;
error :
   goto done ;
}


#elif defined (_LINUX)
PD_TRACE_DECLARE_FUNCTION ( SDB_CMSTART_BLDARGS, "buildArguments" )
INT32 buildArguments ( CHAR **pArgumentBuffer, list<const CHAR*> &argv )
{
   SDB_ASSERT ( pArgumentBuffer, "Invalid input" ) ;
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CMSTART_BLDARGS );
   INT32 bufferSize = 0 ;
   INT32 pos = 0 ;
   // estimate the size of final buffer
   for ( list<const CHAR*>::iterator it = argv.begin(); it != argv.end(); ++it )
      bufferSize += ( ossStrlen ( *it ) + 1 ) ;
   if ( bufferSize <= 0 )
   {
      PD_LOG ( PDERROR, "Failed to calculate buffer size" ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // allocate buffer, free at calling function
   *pArgumentBuffer = (CHAR*)SDB_OSS_MALLOC ( bufferSize ) ;
   if ( !(*pArgumentBuffer) )
   {
      PD_LOG ( PDERROR, "Failed to allocate buffer for %d bytes",
               bufferSize ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   ossMemset ( *pArgumentBuffer, 0, bufferSize ) ;
   // copy arguments into buffer
   for ( list<const CHAR*>::iterator it = argv.begin(); it != argv.end(); ++it )
   {
      ossStrncpy ( &(*pArgumentBuffer)[pos], *it, bufferSize - pos ) ;
      pos += ossStrlen ( *it ) ;
      // each arguments are separated by '\0'
      (*pArgumentBuffer)[pos] = '\0' ;
      ++pos ;
   }

done:
   PD_TRACE_EXITRC ( SDB_CMSTART_BLDARGS, rc );
   return rc ;
error:
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_CMSTART_VRFPID, "verifyPID" )
INT32 verifyPID ( OSSPID inputpid )
{
   INT32 rc                                   = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CMSTART_VRFPID );
   PD_TRACE1 ( SDB_CMSTART_VRFPID, PD_PACK_INT(inputpid) );
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
               PD_LOG ( PDERROR, "Error: Failed to start sdbcm" ) ;
               rc = SDB_SYS ;
               loop = FALSE ;
            }
            // make sure
            // 1) pid matches what we want
            // 2) parent pid matches myself
            // 3) sdbcm name matchs the pattern: "sdbcm($svcname)" ( after exec
            //    successfully run )
            else if ( pid == inputpid && getpid() == ppid )
            {
               if ( NULL != fgets ( commandLine, PROC_PATH_LEN_MAX, fp1 ) &&
                    //ossStrstr ( commandLine, SDBCM_NAME_PATTERN1 ) )
                    0 == ossStrcmp( commandLine, SDBCM_DMN_SVC_NAME ) )
               {
                  ossPrintf ( "Success: SequoiaDB Cluster manager is successfully "
                              "started (%d)"OSS_NEWLINE, pid ) ;
                  loop = FALSE ;
               }
            }
         }
         // if we can't read 4 elements, something wrong
         else
         {
            PD_LOG ( PDERROR, "Error: Failed to extract process information" ) ;
            rc = SDB_SYS ;
            loop = FALSE ;
         }
      }
      else
      {
         // if we can't open the /proc/<pid>/stat, the child process is gone (
         // it should never happen thou )
         PD_LOG ( PDERROR, "Unable to read %s", pathName ) ;
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
   PD_TRACE_EXITRC ( SDB_CMSTART_VRFPID, rc );
   return rc ;
}

PD_TRACE_DECLARE_FUNCTION( SDB_STARTSDBCM2, "startSdbcm" )
INT32 startSdbcm ( list<const CHAR*> &argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_STARTSDBCM2 );
   CHAR *pArgumentBuffer = NULL ;
   OSSPID pid ;
   ossResultCode result ;

   rc = buildArguments ( &pArgumentBuffer, argv ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to build sdbcm arguments") ;
      goto error ;
   }
   PD_LOG ( PDEVENT, "Execute sdbcm, bin=%s", argv.front() );
   rc = ossExec ( pArgumentBuffer, pArgumentBuffer, NULL,
                  0, pid, result, NULL, NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to execute sdbcm, rc = %d", rc ) ;
      goto error ;
   }

   rc = verifyPID ( pid ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to verify PID, rc = %d", rc ) ;
      goto error ;
   }
   else
   {
      PD_LOG ( PDEVENT, "Successful to start sdbcm, pid = %d", pid ) ;
   }

done :
   if ( pArgumentBuffer )
      SDB_OSS_FREE ( pArgumentBuffer ) ;
   PD_TRACE_EXITRC ( SDB_STARTSDBCM2, rc );
   return rc ;
error :
   goto done ;
}
#endif

PD_TRACE_DECLARE_FUNCTION ( SDB_CMSTART_MAIN, "main" )
INT32 main ( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CMSTART_MAIN );
   list<const CHAR*> argvs ;
   CHAR progName[OSS_MAX_PATHSIZE+1] = {0};
   po::options_description desc ( "Command options" ) ;
   ossResultCode result ;
   init ( desc ) ;
   // validate arguments
   rc = resolveArgument ( desc, argc, argv ) ;
   if ( rc )
   {
      if ( SDB_PMD_HELP_ONLY != rc )
      {
         PD_LOG ( PDERROR, "Invalid argument" ) ;
         displayArg ( desc ) ;
      }
      goto done ;
   }

   rc = ossGetEWD ( progName, OSS_MAX_PATHSIZE ) ;
   if ( rc )
   {
      ossPrintf ( "Failed to get excutable file's working directory"OSS_NEWLINE ) ;
      goto error ;
   }
   ossStrncat( progName, OSS_FILE_SEP, 1 );
   ossMemset ( _pdDiagLogPath, 0, sizeof( _pdDiagLogPath ) ) ;
   ossStrncpy ( _pdDiagLogPath, progName, sizeof(progName) );
   ossStrncat ( _pdDiagLogPath, SDBCM_LOG_PATH, sizeof(SDBCM_LOG_PATH) ) ;
   ossStrncat ( progName, SDBCM_DMN_PROC_NAME, sizeof(SDBCM_DMN_PROC_NAME) ) ;

   argvs.push_back(progName);
   for ( INT32 i = 1; i < argc; ++i )
      argvs.push_back(argv[i]);

   rc = startSdbcm ( argvs ) ;
   if ( rc )
      ossPrintf ( "Failed to start sdbcm"OSS_NEWLINE ) ;
   else
      ossPrintf ( "Successful to start sdbcm"OSS_NEWLINE ) ;

done:
   PD_TRACE_EXITRC ( SDB_CMSTART_MAIN, rc );
   return rc ;
error:
   goto error ;
}

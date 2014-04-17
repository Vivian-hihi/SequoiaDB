/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sdbcmtop.cpp

   Descriptive Name = sdbcmtop Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcmtop,
   which is used to stop SequoiaDB Cluster Manager.

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
#include "sdbcm.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
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
#include <windows.h>
#endif


using namespace std;
namespace po = boost::program_options;
extern CHAR _pdDiagLogPath[OSS_MAX_PATHSIZE+1] ;

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
#define SDBCM_NAME_PATTERN          SDBCM_NAME_PATTERN1 "%s" SDBCM_NAME_PATTERN2
#define SDBCM_DMN_NAME              "sdbcmd"
#endif
// timeout for 30 seconds
#define TERMINATE_TIMEOUT 30
#define SDBCM_NAME "sdbcm"

// initialize options
void init ( po::options_description &desc )
{
   ADD_PARAM_OPTIONS_BEGIN ( desc )
      COMMANDS_OPTIONS
   ADD_PARAM_OPTIONS_END
}

void displayArg ( po::options_description &desc )
{
   std::cout << "Usage:  sdbcmtop [OPTION]" <<std::endl;
   std::cout << desc << std::endl ;
}

// in this function we only validate whether there's unexpected input, we do not
// actually doing anything. We will simply pass all parameters to sequoiadb
// engine
PD_TRACE_DECLARE_FUNCTION ( SDB_CMSTOP_RSVARG, "resolveArgument" )
INT32 resolveArgument ( po::options_description &desc, INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CMSTOP_RSVARG );
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
   PD_TRACE_EXITRC ( SDB_CMSTOP_RSVARG, rc );
   return rc ;
error :
   goto done ;
}

#if defined (_LINUX)
PD_TRACE_DECLARE_FUNCTION ( SDB_CMSTOP_TERMPROC, "terminateProcess" )
INT32 terminateProcess ( pid_t &pid, CHAR *pName )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CMSTOP_TERMPROC );
   PD_TRACE1 ( SDB_CMSTOP_TERMPROC, PD_PACK_INT(pid) );
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
   PD_TRACE_EXITRC ( SDB_CMSTOP_TERMPROC, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_LNX_STOPSDBCM, "stopSdbcm" )
INT32 stopSdbcm ( )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_LNX_STOPSDBCM );
   DIR *dirp ;
   struct dirent *dp ;
   CHAR engineName [ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   ossSnprintf ( engineName, OSS_MAX_PATHSIZE, SDBCM_NAME,
                    "" ) ;

   if ( ( dirp = opendir ( "/proc" )) == NULL )

   {
      PD_LOG ( PDERROR, "Failed to open /proc, errno = %d",
               ossGetLastError () ) ;
      rc = SDB_SYS;
      goto error ;
   }
   do
   {
      if ( ( dp = readdir ( dirp ) ) != NULL )
      {
         FILE *fp = NULL ;
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

         if ( 0 != ossStrcmp( commandLine, SDBCM_DMN_NAME ) )
         {
            continue;
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
         }
      }
   } while ( dp != NULL ) ;
   closedir ( dirp ) ;
done :
   PD_TRACE_EXITRC ( SDB_LNX_STOPSDBCM, rc );
   return rc ;
error :
   goto done ;
}

#elif defined (_WINDOWS)
#define SDBCMDMN_SRV_NAME "sdbcmd"
PD_TRACE_DECLARE_FUNCTION ( SDB_CMSTOP_WFSTRS, "WaitForServiceToReachState" )
BOOL WaitForServiceToReachState(SC_HANDLE hService, DWORD dwDesiredState,
                                 SERVICE_STATUS* pss, DWORD dwMilliseconds) {
    PD_TRACE_ENTRY ( SDB_CMSTOP_WFSTRS );
    DWORD dwLastState, dwLastCheckPoint;
    BOOL  fFirstTime = TRUE; // Don't compare state & checkpoint the first time through
    BOOL  fServiceOk = TRUE;
    DWORD dwTimeout = GetTickCount() + dwMilliseconds;
 
    // Loop until the service reaches the desired state,
    // an error occurs, or we timeout
    while  (TRUE) {
       // Get current state of service
       fServiceOk = ::QueryServiceStatus(hService, pss);
 
       // If we can't query the service, we're done
       if (!fServiceOk) break;
 
       // If the service reaches the desired state, we're done
       if (pss->dwCurrentState == dwDesiredState) break;
 
       // If we timed-out, we're done
       if ((dwMilliseconds != INFINITE) && (dwTimeout > GetTickCount())) {
          SetLastError(ERROR_TIMEOUT); 
          break;
       }
 
       // If this is our first time, save the service's state & checkpoint
       if (fFirstTime) {
          dwLastState = pss->dwCurrentState;
          dwLastCheckPoint = pss->dwCheckPoint;
          fFirstTime = FALSE;
       } else {
          // If not first time & state has changed, save state & checkpoint
          if (dwLastState != pss->dwCurrentState) {
             dwLastState = pss->dwCurrentState;
             dwLastCheckPoint = pss->dwCheckPoint;
          } else {
             // State hasn't change, check that checkpoint is increasing
             if (pss->dwCheckPoint > dwLastCheckPoint) {
                // Checkpoint has increased, save checkpoint
                dwLastCheckPoint = pss->dwCheckPoint;
             } else {
                // Checkpoint hasn't increased, service failed, we're done!
                fServiceOk = FALSE; 
                break;
             }
          }
       }
       // We're not done, wait the specified period of time
       Sleep(pss->dwWaitHint);
    }
 
    // Note: The last SERVICE_STATUS is returned to the caller so
    // that the caller can check the service state and error codes.
    PD_TRACE_EXIT ( SDB_CMSTOP_WFSTRS );
    return(fServiceOk);
 }

PD_TRACE_DECLARE_FUNCTION ( SDB_WIN_STOPSDBCM, "stopSdbcm" )
INT32 stopSdbcm ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_WIN_STOPSDBCM );
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
                          SERVICE_STOP ) ;
   if ( schSRV == NULL )
   {
      rc = SDB_SYS ;
      PD_LOG ( PDERROR, "Failed to open service" );
      goto error ;
   }
   ControlService ( schSRV, SERVICE_CONTROL_STOP, &srvStatus ) ;
   WaitForServiceToReachState ( schSRV, SERVICE_STOP, &srvStatus, 15000 ) ;

done :
   if ( schSCM )
      CloseServiceHandle ( schSCM ) ;
   if ( schSRV )
      CloseServiceHandle ( schSRV ) ;
   PD_TRACE_EXITRC ( SDB_WIN_STOPSDBCM, rc );
   return rc ;
error :
   goto done ;
}

#endif

PD_TRACE_DECLARE_FUNCTION ( SDB_CMSTOP_MAIN, "main" )
INT32 main ( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CMSTOP_MAIN );
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

   ossMemset ( _pdDiagLogPath, 0, sizeof( _pdDiagLogPath ) ) ;
   rc = ossGetEWD ( _pdDiagLogPath, OSS_MAX_PATHSIZE ) ;
   if ( rc )
   {
      ossPrintf ( "Failed to get excutable file's working directory"OSS_NEWLINE ) ;
      goto error ;
   }
   ossStrncat( _pdDiagLogPath, OSS_FILE_SEP, 1 );
   ossStrncat ( _pdDiagLogPath, SDBCM_LOG_PATH, sizeof(SDBCM_LOG_PATH) ) ;

   rc = stopSdbcm () ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to stop sdbcm" ) ;
      ossPrintf ( "Failed to stop sdbcm"OSS_NEWLINE ) ;
   }
   else
   {
      PD_LOG ( PDEVENT, "Successful to stop sdbcm" ) ;
      ossPrintf ( "Successful to stop sdbcm"OSS_NEWLINE ) ;
   }

done:
   PD_TRACE_EXITRC ( SDB_CMSTOP_MAIN, rc );
   return rc ;
error:
   goto error ;
}

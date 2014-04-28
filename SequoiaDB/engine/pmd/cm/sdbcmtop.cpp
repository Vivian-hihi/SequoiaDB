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
#include "rtnCM.hpp"
#include "ossProc.hpp"
#include "utilParam.hpp"
#include "utilStr.hpp"
#include "ossVer.h"
#include "pmdDef.hpp"
#include "utilParam.hpp"
#include "pmdOptions.h"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include <string>
#include <iostream>
#include <vector>

#if defined (_LINUX)
   #include <dirent.h>
   #include <sys/types.h>
   #include <signal.h>
#endif

using namespace std;

/*
   Macro define
*/
#define SDBCMTOP_LOG_FILE_NAME      "sdbcmtop.log"
#define SDBCMTOP_TIMEOUT            ( 30000 )

#define COMMANDS_OPTIONS \
       ( PMD_COMMANDS_STRING (PMD_OPTION_HELP, ",h"), "help" ) \
       ( PMD_OPTION_VERSION, "version" )

/*
   Function implement
*/
void init ( po::options_description &desc )
{
   PMD_ADD_PARAM_OPTIONS_BEGIN ( desc )
      COMMANDS_OPTIONS
   PMD_ADD_PARAM_OPTIONS_END
}

void displayArg ( po::options_description &desc )
{
   std::cout << "Usage:  sdbcmtop [OPTION]" <<std::endl;
   std::cout << desc << std::endl ;
}

#if defined (_LINUX)

   // PD_TRACE_DECLARE_FUNCTION ( SDB_CMSTOP_TERMPROC, "terminateWithTimeout" )
   INT32 terminateWithTimeout ( pid_t &pid )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CMSTOP_TERMPROC );
      PD_TRACE1 ( SDB_CMSTOP_TERMPROC, PD_PACK_INT(pid) ) ;

      UINT32 timeout = 0 ;
      rc = ossTerminateProcess( pid, FALSE ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed sending SIGTERM to %d, errno=%d",
                  pid, ossGetLastError() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      // wait until process terminate
      while ( ossIsProcessRunning( pid ) )
      {
         ossSleep( OSS_ONE_SEC ) ;
         timeout += OSS_ONE_SEC ;
         if ( timeout > SDBCMTOP_TIMEOUT )
         {
            break ;
         }
      }

      if ( timeout > SDBCMTOP_TIMEOUT )
      {
         ossPrintf ( "FAILED"OSS_NEWLINE ) ;
         rc = SDB_TIMEOUT ;
      }
      else
      {
         ossPrintf ( "DONE"OSS_NEWLINE ) ;
      }

   done :
      PD_TRACE_EXITRC ( SDB_CMSTOP_TERMPROC, rc );
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_LNX_STOPSDBCM, "stopSdbcm" )
   INT32 stopSdbcm ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_LNX_STOPSDBCM ) ;
      DIR *dirp = NULL ;
      struct dirent *dp = NULL ;
      UINT32 timecount = 0 ;

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
            timecount = 0 ;
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

            if ( 0 != ossStrcmp( commandLine, PMDDMN_SVCNAME_DEFAULT ) )
            {
               continue ;
            }

            // get pid
            pid = atoi ( dp->d_name ) ;
            // verify
            ossSnprintf ( tempName, OSS_MAX_PATHSIZE, "%d", pid ) ;
            if ( ossStrncmp ( tempName, dp->d_name, OSS_MAX_PATHSIZE ) == 0 )
            {
               rc = terminateWithTimeout( pid ) ;
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

   INT32 stopSdbcm ()
   {
      return ossStopService( PMDDMN_SVCNAME_DEFAULT,
                             SDBCMTOP_TIMEOUT ) ;
   }

#endif // _LINUX

// PD_TRACE_DECLARE_FUNCTION ( SDB_CMSTOP_MAIN, "main" )
INT32 main ( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CMSTOP_MAIN ) ;
   po::options_description desc ( "Command options" ) ;
   po::variables_map vm ;
   ossResultCode result ;
   CHAR dialogFile[ OSS_MAX_PATHSIZE + 1 ] = {0} ;

   rc = ossGetEWD ( dialogFile, OSS_MAX_PATHSIZE ) ;
   if ( rc )
   {
      ossPrintf ( "Failed to get excutable file's working "
                  "directory"OSS_NEWLINE ) ;
      goto error ;
   }
   rc = engine::utilCatPath( dialogFile, OSS_MAX_PATHSIZE, SDBCM_LOG_PATH ) ;
   if ( rc )
   {
      ossPrintf( "Failed to build dialog path: %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }
   rc = engine::utilCatPath( dialogFile, OSS_MAX_PATHSIZE,
                             SDBCMTOP_LOG_FILE_NAME ) ;
   if ( rc )
   {
      ossPrintf( "Failed to build dialog file: %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }
   // enable pd log
   sdbEnablePD( dialogFile ) ;
   setPDLevel( PDINFO ) ;

   init ( desc ) ;
   // validate arguments
   rc = engine::utilReadCommandLine ( argc, argv, desc, vm ) ;
   if ( rc )
   {
      PD_LOG( PDERROR, "Invalid arguments, rc: %d", rc ) ;
      displayArg ( desc ) ;
      goto done ;
   }
   /// read cmd first
   if ( vm.count( PMD_OPTION_HELP ) )
   {
      displayArg( desc ) ;
      rc = SDB_PMD_HELP_ONLY ;
      goto done ;
   }
   if ( vm.count( PMD_OPTION_VERSION ) )
   {
      ossPrintVersion( "Sdb CM Stop version" ) ;
      rc = SDB_PMD_VERSION_ONLY ;
      goto done ;
   }

   // stop cm
   rc = stopSdbcm () ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to stop sdbcm, rc: %d", rc ) ;
      ossPrintf ( "Failed to stop sdbcm, rc: %d"OSS_NEWLINE, rc ) ;
   }
   else
   {
      PD_LOG ( PDEVENT, "Successful to stop sdbcm" ) ;
      ossPrintf ( "Successful to stop sdbcm"OSS_NEWLINE ) ;
   }

done:
   PD_TRACE_EXITRC ( SDB_CMSTOP_MAIN, rc ) ;
   return rc ;
error:
   goto error ;
}


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
#include "rtnCM.hpp"
#include "pmdDef.hpp"
#include "utilParam.hpp"
#include "pmdOptions.h"
#include "utilStr.hpp"
#include "ossVer.h"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include <string>
#include <iostream>

using namespace std ;

#define SDBCMART_LOG_FILE_NAME      "sdbcmart.log"

#define COMMANDS_OPTIONS \
       ( PMD_COMMANDS_STRING (PMD_OPTION_HELP, ",h"), "help" ) \
       ( PMD_OPTION_VERSION, "version" )

// initialize options
void init ( po::options_description &desc )
{
   PMD_ADD_PARAM_OPTIONS_BEGIN ( desc )
      COMMANDS_OPTIONS
   PMD_ADD_PARAM_OPTIONS_END
}

void displayArg ( po::options_description &desc )
{
   std::cout << "Usage:  sdbcmart [OPTION]" <<std::endl;
   std::cout << desc << std::endl ;
}

#if defined (_WINDOWS)

   INT32 startSdbcm ( list<const CHAR*> &argv )
   {
      return ossStartService( PMDDMN_SVCNAME_DEFAULT ) ;
   }

#elif defined (_LINUX)

   INT32 startSdbcm ( list<const CHAR*> &argv )
   {
      return ossStartProcess( argv ) ;
   }
#endif // _WINDOWS

// PD_TRACE_DECLARE_FUNCTION ( SDB_CMSTART_MAIN, "main" )
INT32 main ( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CMSTART_MAIN );
   list<const CHAR*> argvs ;
   CHAR dialogFile[ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   CHAR progName[OSS_MAX_PATHSIZE+1] = {0};
   po::options_description desc ( "Command options" ) ;
   po::variables_map vm ;
   ossResultCode result ;

   rc = ossGetEWD ( progName, OSS_MAX_PATHSIZE ) ;
   if ( rc )
   {
      ossPrintf ( "Failed to get excutable file's working "
                  "directory"OSS_NEWLINE ) ;
      goto error ;
   }
   ossStrncat( progName, OSS_FILE_SEP, 1 ) ;

   // build dialog info
   rc = engine::utilBuildFullPath( progName, SDBCM_LOG_PATH,
                                   OSS_MAX_PATHSIZE, dialogFile ) ;
   if ( rc )
   {
      ossPrintf( "Failed to build dialog path: %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }
   rc = engine::utilCatPath( dialogFile, OSS_MAX_PATHSIZE,
                             SDBCMART_LOG_FILE_NAME ) ;
   if ( rc )
   {
      ossPrintf( "Failed to build dialog file: %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }
   // enable pd log
   sdbEnablePD( dialogFile ) ;
   setPDLevel( PDINFO ) ;

   ossStrncat ( progName, PMDDMN_SVCNAME_DEFAULT,
                sizeof( PMDDMN_SVCNAME_DEFAULT ) ) ;

   init ( desc ) ;
   // validate arguments
   rc = engine::utilReadCommandLine( argc, argv, desc, vm ) ;
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
      ossPrintVersion( "Sdb CM Start version" ) ;
      rc = SDB_PMD_VERSION_ONLY ;
      goto done ;
   }

   argvs.push_back( progName ) ;
   for ( INT32 i = 1; i < argc ; ++i )
   {
      argvs.push_back(argv[i]) ;
   }

   rc = startSdbcm ( argvs ) ;
   if ( rc )
   {
      ossPrintf ( "Failed to start sdbcm"OSS_NEWLINE ) ;
   }
   else
   {
      ossPrintf ( "Successful to start sdbcm"OSS_NEWLINE ) ;
   }

done:
   PD_TRACE_EXITRC ( SDB_CMSTART_MAIN, rc );
   return SDB_OK == rc ? 0 : 1 ;
error:
   goto error ;
}


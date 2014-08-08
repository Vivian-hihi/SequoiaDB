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

   Source File Name = pmdDaemon.cpp

   Descriptive Name = pmdDaemon

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/17/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#include "pmdDaemon.hpp"
#include "ossProc.hpp"
#include "utilStr.hpp"
#include "pd.hpp"
#include "ossUtil.h"
#include "ossVer.h"

using namespace engine;

INT32 main( INT32 argc, CHAR** argv )
{
   INT32 rc = SDB_OK ;
   CHAR dialogFile[ OSS_MAX_PATHSIZE + 1 ] = {0} ;
   cCMService svc;
   cPmdDaemon daemon( PMDDMN_SVCNAME_DEFAULT ) ;

   rc = ossGetEWD( dialogFile, OSS_MAX_PATHSIZE ) ;
   if ( rc )
   {
      ossPrintf( "Failed to get working directory, rc: %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }
   rc = engine::utilCatPath( dialogFile, OSS_MAX_PATHSIZE, SDBCM_LOG_PATH ) ;
   if ( rc )
   {
      ossPrintf( "Failed to make dialog path, rc: %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }
   rc = engine::utilCatPath( dialogFile, OSS_MAX_PATHSIZE,
                             PMDDMN_DIALOG_FILE_NAME ) ;
   if ( rc )
   {
      ossPrintf( "Failed to make dialog path, rc: %d"OSS_NEWLINE, rc ) ;
      goto error ;
   }

   // enable pd log
   sdbEnablePD( dialogFile ) ;
   setPDLevel( PDINFO ) ;

   PD_LOG( PDEVENT, "Start cmd[Ver: %d.%d, Release: %d, Build: %s]...",
           SDB_ENGINE_VERISON_CURRENT, SDB_ENGINE_SUBVERSION_CURRENT,
           SDB_ENGINE_RELEASE_CURRENT, SDB_ENGINE_BUILD_TIME ) ;

   rc = svc.init();
   PD_RC_CHECK( rc, PDERROR, "Failed to init cm(rc=%d)", rc ) ;
   rc = daemon.addChildrenProcess( &svc ) ;
   PD_RC_CHECK( rc, PDERROR, "Failed to add childrenProcess(rc=%d)", rc ) ;
   rc = daemon.init();
   if ( rc != SDB_OK )
   {
      ossPrintf( "Failed to init daemon process(rc=%d)", rc ) ;
      goto error;
   }

   rc = daemon.run( argc, argv );
   PD_RC_CHECK( rc, PDERROR, "Execute failed(rc=%d)", rc ) ;

done:
   daemon.stop() ;
   PD_LOG( PDEVENT, "Stop programme." ) ;
   return rc;
error:
   goto done ;
}


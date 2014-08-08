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

   Source File Name = pmdCMMain.cpp

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
#include "ossErr.h"
#include "utilStr.hpp"
#include "ossProc.hpp"
#include "ossUtil.hpp"
#include "omagentMgr.hpp"
#include "pd.hpp"
#include "ossVer.h"
#include "pmd.hpp"
#include "pmdProc.hpp"

namespace engine
{
   void pmdOnQuit()
   {
      PMD_SHUTDOWN_DB( SDB_INTERRUPT ) ;
      iPmdProc::stop( 0 ) ;
   }

   INT32 pmdThreadMainEntry( INT32 argc, CHAR** argv )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      CHAR currentPath[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
      CHAR dialogPath[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
      CHAR dialogFile[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;

      // 1. get root path
      rc = ossGetEWD( currentPath, OSS_MAX_PATHSIZE ) ;
      if ( rc )
      {
         std::cout << "Get current path failed: " << rc << std::endl ;
         goto error ;
      }

      // 2. enable dialog
      rc = utilBuildFullPath( currentPath, SDBCM_LOG_PATH,
                              OSS_MAX_PATHSIZE, dialogPath ) ;
      if ( rc )
      {
         std::cout << "Build dialog path failed: " << rc << std::endl ;
         goto error ;
      }
      // make sure the dir exist
      rc = ossMkdir( currentPath, OSS_CREATE|OSS_READWRITE ) ;
      if ( rc && SDB_FE != rc )
      {
         std::cout << "Create dialog dir: " << currentPath << " failed: "
                   << rc << std::endl ;
         goto error ;
      }
      rc = utilBuildFullPath( dialogPath, SDBCM_DIALOG_FILE_NAME,
                              OSS_MAX_PATHSIZE, dialogFile ) ;
      if ( rc )
      {
         std::cout << "Build dialog path failed: " << rc << std::endl ;
         goto error ;
      }
      sdbEnablePD( dialogFile ) ;
      setPDLevel( PDINFO ) ;

      PD_LOG( PDEVENT, "Start cm[Ver: %d.%d, Release: %d, Build: %s]...",
              SDB_ENGINE_VERISON_CURRENT, SDB_ENGINE_SUBVERSION_CURRENT,
              SDB_ENGINE_RELEASE_CURRENT, SDB_ENGINE_BUILD_TIME ) ;

      // 3. init param
      rc = sdbGetOMAgentOptions()->init( currentPath ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to init config, rc: %d", rc ) ;
         goto error ;
      }

      // 4. print all config
      {
         string configs ;
         sdbGetOMAgentOptions()->toString( configs ) ;
         PD_LOG( PDEVENT, "All configs: %s", configs.c_str() ) ;
      }

      // 5. handlers and init global mem
      rc = pmdEnableSignalEvent( dialogPath, (PMD_ON_QUIT_FUNC)pmdOnQuit ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to enable trap, rc: %d", rc ) ;

      // 6. register agent cb
      PMD_REGISTER_CB( sdbGetOMAgentMgr() ) ;

      // 7. init krcb
      rc = krcb->init() ;
      PD_RC_CHECK( rc, PDERROR, "Failed to init krcb, rc: %d", rc ) ;

      // 8. change process name
#if defined (_LINUX)
      {
         CHAR pmdProcessName [ OSS_RENAME_PROCESS_BUFFER_LEN + 1 ] = {0} ;
         ossSnprintf ( pmdProcessName, OSS_RENAME_PROCESS_BUFFER_LEN,
                       SDBCM_NAME_PATTERN,
                       sdbGetOMAgentOptions()->getCMServiceName() ) ;
         ossEnableNameChanges ( argc, argv ) ;
         ossRenameProcess ( pmdProcessName ) ;
      }
#endif // _LINUX

      // Now master thread get into big loop and check shutdown flag
      while ( PMD_IS_DB_UP )
      {
         ossSleepsecs ( 1 ) ;
      }
      rc = krcb->getExitCode() ;

   done:
      PMD_SHUTDOWN_DB( rc ) ;
      pmdSetQuit() ;
      krcb->destroy () ;
      PD_LOG ( PDEVENT, "Stop programme, exit code: %d",
               krcb->getExitCode() ) ;
      return rc == SDB_OK ? 0 : 1 ;
   error:
      goto done ;
   }

}

INT32 main( INT32 argc, CHAR** argv )
{
   return engine::pmdThreadMainEntry( argc, argv ) ;
}


/*******************************************************************************


   Copyright (C) 2011-2016 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = pmdSEAdapterMain.cpp

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/

#include "core.hpp"
#include "ossVer.hpp"
#include "ossProc.hpp"
#include "pd.hpp"
#include "pmd.hpp"
#include "pmdProc.hpp"
#include "seAdptDef.hpp"
#include "seAdptMgr.hpp"

namespace engine
{
   #define COMMANDS_OPTIONS \
      ( PMD_COMMANDS_STRING ( PMD_OPTION_HELP, ",h"), "help" ) \
      ( PMD_OPTION_VERSION, "version" )

   void displayArg( po::options_description &desc )
   {
      std::cout << "Usage:  sdbseadapter [OPTIONS]" << std::endl ;
      std::cout << desc << std::endl ;
   }

   INT32 initArgs( INT32 argc, CHAR **argv, po::variables_map &vm )
   {
      INT32 rc = SDB_OK ;
      po::options_description desc( "Command options" ) ;
      po::options_description all( "Command options" ) ;

      PMD_ADD_PARAM_OPTIONS_BEGIN( all )
         COMMANDS_OPTIONS
      PMD_ADD_PARAM_OPTIONS_END

      PMD_ADD_PARAM_OPTIONS_BEGIN( desc )
         COMMANDS_OPTIONS
      PMD_ADD_PARAM_OPTIONS_END

      rc = utilReadCommandLine( argc, argv, all, vm ) ;
      if ( rc )
      {
         std::cerr << "Invalid arguments: " << rc << std::endl ;
         displayArg( desc ) ;
         goto error ;
      }

      if ( vm.count( PMD_OPTION_HELP ) )
      {
         displayArg( desc ) ;
         rc = SDB_PMD_HELP_ONLY ;
         goto done ;
      }
      if ( vm.count( PMD_OPTION_VERSION ) )
      {
         ossPrintVersion( "sdbseadapter version" ) ;
         rc = SDB_PMD_VERSION_ONLY ;
         goto done ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

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
      CHAR verText[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
      INT32 delSig[] = { 17, 0 } ;
      po::variables_map vm ;

      rc = initArgs( argc, argv, vm ) ;
      if ( rc )
      {
         if ( SDB_PMD_HELP_ONLY == rc || SDB_PMD_VERSION_ONLY == rc )
         {
            rc = SDB_OK ;
            goto done ;
         }
         goto error ;
      }

      rc = ossGetEWD( currentPath, OSS_MAX_PATHSIZE ) ;
      if ( rc )
      {
         std::cerr << "Get current path failed[ " << rc << " ]" << std:: endl ;
         goto error ;
      }

      ossChDir( currentPath ) ;

      // TODO: which path to put diaglog?
      // Temporary: use the current path.
      rc = ossGetEWD( dialogPath, OSS_MAX_PATHSIZE ) ;
      if ( rc )
      {
         std::cerr << "Get current path failed[ " << rc << " ]" << std:: endl ;
         goto error ;
      }

      rc = utilBuildFullPath( currentPath, SDB_SEADPT_LOG_FILE_NAME,
                              OSS_MAX_PATHSIZE, dialogFile ) ;
      if ( rc )
      {
         std::cerr << "Build dialog path failed[ " << rc << " ]" << std::endl ;
         goto error ;
      }
      sdbEnablePD( dialogFile ) ;

      ossSprintVersion( "Version", verText, OSS_MAX_PATHSIZE, FALSE ) ;
      PD_LOG( PDEVENT, "Start sdbseadapter[%s]...", verText ) ;

      rc = sdbGetSeAdptOptions()->init( currentPath ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Initialize configuration failed[ %d ]", rc ) ;
         goto error ;
      }

      setPDLevel( sdbGetSeAdptOptions()->getDiagLevel() ) ;

      // Print all configurations in log file.
      {
         string configs ;
         sdbGetSeAdptOptions()->toString( configs ) ;
         PD_LOG( PDEVENT, "All configs:\n%s", configs.c_str() ) ;
      }

      rc = pmdEnableSignalEvent( dialogPath, (PMD_ON_QUIT_FUNC)pmdOnQuit,
                                 delSig ) ;
      PD_RC_CHECK( rc, PDERROR, "Enable trap failed[ %d ]", rc ) ;

      PMD_REGISTER_CB( sdbGetSeAdapterCB() ) ;

      rc = krcb->init() ;
      PD_RC_CHECK( rc, PDERROR, "Initialize krcb failed[ %d ]", rc ) ;

      while ( PMD_IS_DB_UP() )
      {
         ossSleep( 1 ) ;
      }
      rc = krcb->getShutdownCode() ;

   done:
      PMD_SHUTDOWN_DB( rc ) ;
      pmdSetQuit() ;
      krcb->destroy() ;
      PD_LOG( PDEVENT, "Stop program, exit code: %d",
              krcb->getShutdownCode() ) ;
      return SDB_OK == rc ? 0 : 1 ;
   error:
      goto done ;
   }
}

INT32 main( INT32 argc, CHAR** argv )
{
   return engine::pmdThreadMainEntry( argc, argv ) ;
}


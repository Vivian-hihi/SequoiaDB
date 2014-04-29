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

   Source File Name = pmdMain.cpp

   Descriptive Name = Process MoDel Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for SequoiaDB,
   and all other process-initialization code.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include <iostream>
#include <string>
#include "ossVer.h"
#include "pmd.hpp"
#include "pmdEDUMgr.hpp"
#include "pd.hpp"
#include "msgMessage.hpp"
#include "../bson/bson.h"
#include "ossStackDump.hpp"
#include "ossEDU.hpp"
#include "rtn.hpp"
#include "ossAtomic.hpp"
#include "pmdCommon.hpp"
#include "ossProc.hpp"
#include "pmdStartup.hpp"
#include "catDef.hpp"
#include "authDef.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "optQgmStrategy.hpp"
#include "rtnBackgroundJob.hpp"
#include "rtnPageCleanerJob.hpp"
#include "pmdCB.hpp"
#include "omManager.hpp"

using namespace std;
using namespace bson;

namespace engine
{
   /*
    * This function resolve all input arguments from command line
    * It first construct options_description to register all
    * possible arguments we may have
    * And then it will to load from config file
    * Then it will parse command line input again to override config file
    * Basically we want to make sure all parameters that
    * specified in config file
    * can be simply overrided from commandline
    */
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDRESVARGS, "pmdResolveArguments" )
   INT32 pmdResolveArguments( INT32 argc, CHAR** argv )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDRESVARGS ) ;
      rc = pmdGetKRCB()->getOptionCB()->init( argc, argv );
      // if user only ask for help information, we simply return
      if ( SDB_PMD_HELP_ONLY == rc || SDB_PMD_VERSION_ONLY == rc )
      {
         PMD_SHUTDOWN_DB( SDB_OK ) ;
         rc = SDB_OK;
         goto done;
      }
      else if ( rc )
      {
         goto error;
      }

   done :
      PD_TRACE_EXITRC ( SDB_PMDRESVARGS, rc );
      return rc ;
   error :
      goto done ;
   }

   void pmdOnQuit()
   {
      PMD_SHUTDOWN_DB( SDB_INTERRUPT ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDREBLDDB, "pmdRebuildDB" )
   static INT32 pmdRebuildDB ()
   {
      INT32 rc                 = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDREBLDDB );
      pmdKRCB *krcb            = pmdGetKRCB () ;
      pmdEDUMgr *eduMgr        = krcb->getEDUMgr () ;
      pmdEDUCB *cb             = NULL ;
      SDB_START_TYPE startType = krcb->getStartType () ;
      if ( SDB_START_CRASH == startType )
      {
         PD_LOG ( PDEVENT, "Crash recovery is required, perform full database "
                  "rebuild" ) ;
         // memory if free by end of the function
         // we create a dummy pmdEDUCB for rtnRebuilDB, this is not a real
         // agent!
         cb = SDB_OSS_NEW pmdEDUCB ( eduMgr, EDU_TYPE_AGENT ) ;
         if ( !cb )
         {
            PD_LOG ( PDERROR, "Failed to allocate memory for cb" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         rc = rtnRebuildDB ( cb ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to rebuild database, rc = %d", rc ) ;
            goto error ;
         }
         pmdGetStartup().ok ( TRUE ) ;
      }
   done :
      if ( cb )
         SDB_OSS_DEL cb ;
      PD_TRACE_EXITRC ( SDB_PMDREBLDDB, rc );
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDSYSINIT, "pmdSysInit" )
   INT32 pmdSysInit ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDSYSINIT ) ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      SDB_DMSCB *dmsCB = krcb->getDMSCB() ;
      SDB_DPSCB *dpsCB = krcb->getDPSCB() ;

      rtnPageCleanerJob *pcJob = NULL ;
      SDB_ROLE dbRole ;

      //analysis the start type
      rc = pmdGetStartup().init() ;
      PD_RC_CHECK ( rc, PDERROR,
                    "start up check failed[rc:%d]", rc ) ;

      rc = getQgmStrategyTable()->init() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Init qgm strategy table failed, rc: %d", rc ) ;
         goto error ;
      }

      // check the database role, we load storage units when the role is data,
      // catalog or auth
      dbRole = krcb->getDBRole () ;
      if ( SDB_ROLE_DATA       == dbRole ||
           SDB_ROLE_AUTH       == dbRole ||
           SDB_ROLE_CATALOG    == dbRole ||
           SDB_ROLE_STANDALONE == dbRole ||
           SDB_ROLE_COORD      == dbRole ||
           SDB_ROLE_OM         == dbRole )
      {
         // only data and standalone role load all collectionspaces
         if ( SDB_ROLE_DATA       == dbRole ||
              SDB_ROLE_STANDALONE == dbRole )
         {
            rc = rtnLoadCollectionSpaces ( pmdGetOptionCB()->getDbPath(),
                                           pmdGetOptionCB()->getIndexPath(),
                                           dmsCB ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to load collection spaces" ) ;
               goto error ;
            }
         }
         else if ( SDB_ROLE_CATALOG == dbRole )
         {
            rtnLoadCollectionSpace ( CAT_SYS_SPACE_NAME,
                                     pmdGetOptionCB()->getDbPath(),
                                     pmdGetOptionCB()->getIndexPath(),
                                     dmsCB, FALSE ) ;
            rtnLoadCollectionSpace ( AUTH_SPACE,
                                     pmdGetOptionCB()->getDbPath(),
                                     pmdGetOptionCB()->getIndexPath(),
                                     dmsCB, FALSE ) ;
            rtnLoadCollectionSpace ( CAT_PROCEDURES_SPACE_NAME,
                                     pmdGetOptionCB()->getDbPath(),
                                     pmdGetOptionCB()->getIndexPath(),
                                     dmsCB, FALSE ) ;
         }
         else if ( SDB_ROLE_OM == dbRole )
         {
            sdbGetOMManager()->initialize() ;
         }

         // initialize temp space
         rc = dmsCB->getTempCB()->init () ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to initialize temp cb, rc:%d", rc ) ;
            goto error ;
         }
      }
      else
      {
         PD_LOG ( PDERROR, "Invalid db role: %d", dbRole ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( SDB_ROLE_COORD != dbRole )
      {
         rc = dpsCB->init( pmdGetOptionCB()->getReplLogPath(),
                           pmdGetOptionCB()->getReplLogBuffSize() ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to initialize dps cb, rc = %d", rc ) ;
            goto error ;
         }
      }

      if ( SDB_ROLE_COORD == dbRole )
      {
         rc = krcb->getFMPCB()->init() ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to init fmpCB:%d", rc ) ;
            goto error ;
         }
      }

      // initialize bsp
      if ( SDB_ROLE_COORD != dbRole )
      {
         rc = krcb->getBPSCB()->init () ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to initialize bufferpool service, "
                     "rc = %d", rc ) ;
            goto error ;
         }
      }

      if ( SDB_ROLE_STANDALONE == dbRole )
      {
         // we perform full database rebuild ONLY IN STANDALONE mode!!!
         // In non-standalone mode, database is going to sync with other nodes,
         // which is performed in CLS component
         rc = pmdRebuildDB () ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to rebuild database, rc = %d", rc ) ;
            goto error ;
         }
      }

      for ( UINT32 i = 0; i < pmdGetOptionCB()->getPageCleanNum (); ++i )
      {
         // Once all collectionspaces are loaded, let's start up page cleaners
         pcJob = SDB_OSS_NEW rtnPageCleanerJob() ;
         if ( NULL == pcJob )
         {
            PD_LOG ( PDERROR, "Failed to alloc memory for page cleaner job" ) ;
            rc = SDB_OOM ;
            goto error ;
         }

         // start the job
         rc = rtnGetJobMgr()->startJob( pcJob, RTN_JOB_MUTEX_NONE, NULL ) ;
         if ( SDB_RTN_MUTEX_JOB_EXIST == rc )
         {
            rc = SDB_OK ;
         }
         // if we failed to start the job, no worry, it's not a big deal
         if ( rc )
         {
            PD_LOG ( PDWARNING, "Failed to start page cleaner job, rc = %d",
                     rc ) ;
            rc = SDB_OK ;
         }
      }
   done :
      PD_TRACE_EXITRC ( SDB_PMDSYSINIT, rc );
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDPOSTINIT, "pmdPostInit" )
   INT32 pmdPostInit ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDPOSTINIT );
      rtnLoadJob *loadJob = SDB_OSS_NEW rtnLoadJob() ;
      if ( NULL == loadJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for loadJob" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( loadJob, RTN_JOB_MUTEX_NONE, NULL ) ;
      if ( SDB_RTN_MUTEX_JOB_EXIST == rc )
      {
         rc = SDB_OK ;
      }
   done :
      PD_TRACE_EXITRC ( SDB_PMDPOSTINIT, rc );
      return rc ;
   error :
      goto done ;
   }

   INT32 pmdSysExistance()
   {
      pmdKRCB *krcb = pmdGetKRCB() ;

      if ( krcb->getClsCB() && krcb->getReplCB() )
      {
         // stop repl-sync
         krcb->getReplCB()->setStatus( CLS_BS_CLOSED ) ;
         // wait all repl-sync log processed
         krcb->getReplCB()->getBucket()->waitEmpty() ;
      }

      return SDB_OK ;
   }

// based on millisecond
#define PMD_START_WAIT_TIME         ( 300000 )

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDMSTTHRDMAIN, "pmdMasterThreadMain" )
   INT32 pmdMasterThreadMain ( INT32 argc, CHAR** argv )
   {
      INT32      rc       = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDMSTTHRDMAIN );
      pmdKRCB   *krcb     = pmdGetKRCB () ;
      pmdEDUMgr *eduMgr   = krcb->getEDUMgr () ;
      EDUID      agentEDU = PMD_INVALID_EDUID ;
      SDB_ROLE   dbrole ;
      UINT32     startTimerCount = 0 ;

      rc = krcb->init() ;
      if ( rc )
      {
         ossPrintf( "Failed to init krcb, rc: %d"OSS_NEWLINE, rc ) ;
         goto error ;
      }

      // 1. read command line first
      rc = pmdResolveArguments ( argc, argv ) ;
      if ( rc )
      {
         ossPrintf( "Failed resolving arguments(error=%d), exit"OSS_NEWLINE,
                    rc ) ;
         goto error ;
      }
      if ( PMD_IS_DB_DOWN )
      {
         return rc ;
      }

      // 2. enalble pd log
      sdbEnablePD( pmdGetOptionCB()->getDiagLogPath(),
                   pmdGetOptionCB()->diagFileNum() ) ;
      setPDLevel( (PDLEVEL)( pmdGetOptionCB()->getDiagLevel() ) ) ;

      PD_LOG ( ( getPDLevel() > PDEVENT ? PDEVENT : getPDLevel() ) ,
               "Start sequoiadb(%s) [Ver: %d.%d, Release: %d, Build: %s]...",
               pmdGetOptionCB()->krcbRole(), SDB_ENGINE_VERISON_CURRENT,
               SDB_ENGINE_SUBVERSION_CURRENT, SDB_ENGINE_RELEASE_CURRENT,
               SDB_ENGINE_BUILD_TIME ) ;

      // 3. printf all configs
      {
         BSONObj confObj ;
         krcb->getOptionCB()->toBSON( confObj ) ;
         PD_LOG( PDEVENT, "All configs: %s", confObj.toString().c_str() ) ;
      }

      // 4. handlers and init global mem
      rc = pmdEnableSignalEvent( pmdGetOptionCB()->getDiagLogPath(),
                                 (PMD_ON_QUIT_FUNC)pmdOnQuit ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to enable trap, rc: %d", rc ) ;

      // initialize variables
      rc = pmdSysInit () ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to initialize, rc: %d", rc ) ;

      dbrole = krcb->getDBRole () ;

      eduMgr->startEDU( EDU_TYPE_SYNCCLOCK, NULL, &agentEDU ) ;
      eduMgr->regSystemEDU( EDU_TYPE_SYNCCLOCK, agentEDU ) ;

      if ( SDB_ROLE_COORD != dbrole )
      {
         // Then start log global writer thread
         eduMgr->startEDU ( EDU_TYPE_LOGGW, NULL, &agentEDU ) ;
         eduMgr->regSystemEDU ( EDU_TYPE_LOGGW, agentEDU ) ;
         // Start log notify
         eduMgr->startEDU( EDU_TYPE_LOGGNTY, NULL, &agentEDU ) ;
         eduMgr->regSystemEDU ( EDU_TYPE_LOGGNTY, agentEDU ) ;
      }

      if ( SDB_ROLE_STANDALONE == dbrole || SDB_ROLE_DATA == dbrole )
      {
         eduMgr->startEDU ( EDU_TYPE_DPSROLLBACK_TASK, NULL, &agentEDU );
         eduMgr->regSystemEDU ( EDU_TYPE_DPSROLLBACK_TASK, agentEDU );
         rc = eduMgr->waitUntil( EDU_TYPE_DPSROLLBACK_TASK, PMD_EDU_WAITING ) ;
         PD_RC_CHECK( rc, PDERROR, "Wait rollback-task active failed, rc: %d", rc ) ;
      }

      // start cluster mgr
      if ( SDB_ROLE_STANDALONE != dbrole && SDB_ROLE_COORD != dbrole &&
           SDB_ROLE_OM != dbrole )
      {
         krcb->setBusinessOK( FALSE ) ;
         // Then start cluster thread
         eduMgr->startEDU ( EDU_TYPE_CLUSTER, NULL, &agentEDU ) ;
         rc = eduMgr->waitUntil( EDU_TYPE_CLUSTER, PMD_EDU_WAITING ) ;
         PD_RC_CHECK( rc, PDERROR, "Wait CLSMGR active failed, rc: %d", rc ) ;

         // Start cluster shard
         rc = eduMgr->startEDU( EDU_TYPE_CLUSTERSHARD, NULL, &agentEDU ) ;
         PD_RC_CHECK( rc, PDERROR, "Start CLSSHARD edu failed, rc: %d", rc ) ;
      }

      // start catalog service(after cluster mgr)
      if ( SDB_ROLE_CATALOG == dbrole )
      {
         // Then start catalog main controller thread
         eduMgr->startEDU ( EDU_TYPE_CATMAINCONTROLLER, NULL, &agentEDU ) ;
         rc = eduMgr->waitUntil( EDU_TYPE_CATMAINCONTROLLER, PMD_EDU_WAITING ) ;
         PD_RC_CHECK( rc, PDERROR, "Wait CatMainController active failed, rc: %d", rc ) ;

         // Then start catalog manager thread
         eduMgr->startEDU ( EDU_TYPE_CATCATALOGUEMANAGER, NULL, &agentEDU ) ;
         rc = eduMgr->waitUntil( EDU_TYPE_CATCATALOGUEMANAGER, PMD_EDU_WAITING ) ;
         PD_RC_CHECK( rc, PDERROR, "Wait CatalogMgr active failed, rc: %d", rc ) ;

         // Then start node manager thread
         eduMgr->startEDU ( EDU_TYPE_CATNODEMANAGER, NULL, &agentEDU ) ;
         rc = eduMgr->waitUntil( EDU_TYPE_CATNODEMANAGER, PMD_EDU_WAITING ) ;
         PD_RC_CHECK( rc, PDERROR, "Wait CatNodeMgr active failed, rc: %d", rc ) ;
      }

      // start catalog network(after catalog service and before shard & repl network)
      if ( SDB_ROLE_CATALOG == dbrole )
      {
         eduMgr->startEDU ( EDU_TYPE_CATNETWORK, NULL, &agentEDU ) ;
         eduMgr->regSystemEDU ( EDU_TYPE_CATNETWORK, agentEDU ) ;
         rc = eduMgr->waitUntil( EDU_TYPE_CATNETWORK, PMD_EDU_RUNNING ) ;
         PD_RC_CHECK( rc, PDERROR, "Wait CATNET active failed, rc: %d", rc ) ;
      }

      // start shard & repl net work
      if ( SDB_ROLE_STANDALONE != dbrole && SDB_ROLE_COORD != dbrole &&
           SDB_ROLE_OM != dbrole )
      {
         rc = krcb->getClsCB()->startNet() ;
         PD_RC_CHECK( rc, PDERROR, "Start shard or repl net failed, rc: %d", rc ) ;

         // wait shard net work
         rc = eduMgr->waitUntil( EDU_TYPE_SHARDR, PMD_EDU_RUNNING ) ;
         PD_RC_CHECK( rc, PDERROR, "Wait SHARDNET active failed, rc: %d", rc ) ;

         // wait repl net work
         rc = eduMgr->waitUntil( EDU_TYPE_REPR, PMD_EDU_RUNNING ) ;
         PD_RC_CHECK( rc, PDERROR, "Wait REPLNET active failed, rc: %d", rc ) ;
      }

      if  ( SDB_ROLE_COORD == dbrole )
      {
         // Then start coord network thread
         eduMgr->startEDU ( EDU_TYPE_COORDNETWORK, NULL, &agentEDU ) ;
         eduMgr->regSystemEDU ( EDU_TYPE_COORDNETWORK, agentEDU ) ;
         rc = eduMgr->waitUntil( agentEDU , PMD_EDU_RUNNING ) ;
         PD_RC_CHECK( rc, PDERROR, "Wait CoordNet active failed, rc: %d", rc ) ;
         pmdGetStartup().ok( TRUE );
      }

      // Then start tcp lisening thread and other helper threads
      eduMgr->startEDU ( EDU_TYPE_TCPLISTENER, NULL, &agentEDU ) ;
      eduMgr->regSystemEDU ( EDU_TYPE_TCPLISTENER, agentEDU ) ;

      // wait until tcp listener starts
      rc = eduMgr->waitUntil ( agentEDU, PMD_EDU_RUNNING ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait TCPListen active failed, rc: %d", rc ) ;

      // Then start http listening thread
      if ( SDB_ROLE_OM != dbrole )
      {
         eduMgr->startEDU ( EDU_TYPE_HTTPLISTENER, NULL, &agentEDU ) ;
         eduMgr->regSystemEDU ( EDU_TYPE_HTTPLISTENER, agentEDU ) ;
      }
      else
      {
         eduMgr->startEDU ( EDU_TYPE_RESTLISTENER, NULL, &agentEDU ) ;
         eduMgr->regSystemEDU ( EDU_TYPE_RESTLISTENER, agentEDU ) ;
      }
      rc = eduMgr->waitUntil( agentEDU, PMD_EDU_RUNNING ) ;
      PD_RC_CHECK( rc, PDERROR, "Wait HTTPListen active failed, rc: %d", rc ) ;

      // wait until business is ok
      while ( PMD_IS_DB_UP && startTimerCount < PMD_START_WAIT_TIME &&
              !krcb->isBusinessOK() )
      {
         ossSleepmillis( 100 ) ;
         startTimerCount += 100 ;
      }

      if ( PMD_IS_DB_DOWN )
      {
         rc = krcb->getExitCode() ;
         PD_LOG( PDERROR, "Start failed, rc: %d", rc ) ;
         goto error ;
      }
      else if ( startTimerCount >= PMD_START_WAIT_TIME )
      {
         PMD_SHUTDOWN_DB( SDB_TIMEOUT ) ;
         PD_LOG( PDERROR, "Start failed(wait business ative timeout)" ) ;
         rc = SDB_TIMEOUT ;
         goto error ;
      }

#if defined (_LINUX)
      {
         // once all threads starts ( especially we need to make sure the
         // TcpListener thread is successfully started ), we can rename the
         // process. Otherwise if TcpListener failed
         CHAR pmdProcessName [ OSS_RENAME_PROCESS_BUFFER_LEN + 1 ] = {0} ;
         ossSnprintf ( pmdProcessName, OSS_RENAME_PROCESS_BUFFER_LEN,
                       ENGINE_NAME_PATTERN,
                       pmdGetOptionCB()->getServiceAddr() ) ;
         ossEnableNameChanges ( argc, argv ) ;
         ossRenameProcess ( pmdProcessName ) ;
      }
#elif defined (_WINDOWS)
      // Then start windows listener thread for "backdoor" listening
      eduMgr->startEDU ( EDU_TYPE_WINDOWSLISTENER, NULL, &agentEDU ) ;
      eduMgr->regSystemEDU ( EDU_TYPE_WINDOWSLISTENER, agentEDU ) ;
#endif
      // pmdPostInit () ;
      rc = pmdPostInit() ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to call pmdPostInit, rc=%d", rc ) ;
         goto error ;
      }
      // Now master thread get into big loop and check shutdown flag
      while ( PMD_IS_DB_UP )
      {
         ossSleepsecs ( 1 ) ;
      }
      rc = krcb->getExitCode() ;

   done :
      PMD_SHUTDOWN_DB( rc ) ;
      pmdSysExistance () ;
      krcb->destroy () ;
      pmdGetStartup().final() ;
      PD_LOG ( PDEVENT, "Stop sequoiadb, exist code: %d",
               krcb->getExitCode() ) ;
      PD_TRACE_EXITRC ( SDB_PMDMSTTHRDMAIN, rc );
      return rc ;
   error :
      goto done ;
   }

}

/**************************************/
/*   DATABASE MAIN FUNCTION           */
/**************************************/
//PD_TRACE_DECLARE_FUNCTION ( SDB_PMDMAIN, "main" )
INT32 main ( INT32 argc, CHAR** argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_PMDMAIN );
   rc = engine::pmdMasterThreadMain ( argc, argv ) ;
   PD_TRACE_EXITRC ( SDB_PMDMAIN, rc );
   return rc ;
}


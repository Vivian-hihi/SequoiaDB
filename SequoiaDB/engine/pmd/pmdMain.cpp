/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include "core.hpp"
#include "pmd.hpp"
#include "pmdEDUMgr.hpp"
#include "pd.hpp"
#include "pmdSignalHandler.hpp"
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
#include "pmdCB.hpp"

using namespace std;
using namespace bson;
namespace po = boost::program_options ;
namespace engine
{
   extern BSONObj _retObj[] ;
   extern BSONObj gUndefinedKeys [] ;

   extern boost::thread_specific_ptr<oss_edu_data> _ossEduData ;

   /*
    * This function resolve all input arguments from command line
    * It first construct options_description to register all
    * possible arguments we may have
    * And then it will call pmdLoadConfigure to load from config file
    * Then it will parse command line input again to override config file
    * Basically we want to make sure all parameters that
    * specified in config file
    * can be simply overrided from commandline
    */
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDRESVARGS, "pmdResolveArguments" )
   INT32 pmdResolveArguments(INT32 argc, CHAR** argv)
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDRESVARGS );
      rc = pmdGetKRCB()->getOptionCB()->init( argc, argv );
      // if user only ask for help information, we simply return
      if ( SDB_PMD_HELP_ONLY == rc || SDB_PMD_VERSION_ONLY == rc )
      {
         PMD_SHUTDOWN_DB( SDB_OK ) ;
         rc = SDB_OK;
         goto done;
      }
      if ( SDB_OK != rc )
      {
         goto error;
      }
      else
      {
         /// do nothing
      }

   done :
      PD_TRACE_EXITRC ( SDB_PMDRESVARGS, rc );
      return rc ;
   error :
      goto done ;
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

      // 0 - 4095 is for error
      // 4096 is for success
      // 4097-8192 are for warning
      for ( SINT32 i = -SDB_MAX_ERROR; i <= SDB_MAX_WARNING ; i ++ )
      {
         BSONObjBuilder berror ;
         berror.append ( OP_ERRNOFIELD, i ) ;
         berror.append ( OP_ERRDESP_FIELD, getErrDesp ( i ) ) ;
         _retObj[i+SDB_MAX_ERROR] = berror.done().copy() ;
      }

      // initialize undefined keys, from 1 field to 10 fields
      for ( SINT32 i = 0; i < IXM_MAX_PREALLOCATED_UNDEFKEY; ++i )
      {
         BSONObjBuilder b ;
         for ( SINT32 j = 0; j <= i; ++j )
         {
            b.appendUndefined("") ;
         }
         gUndefinedKeys[i] = b.obj() ;
      }

      if ( _ossEduData.get()==0 )
      {
         _ossEduData.reset(SDB_OSS_NEW oss_edu_data());
      }

      // check the database role, we load storage units when the role is data,
      // catalog or auth
      dbRole = krcb->getDBRole () ;
      if ( SDB_ROLE_DATA       == dbRole ||
           SDB_ROLE_AUTH       == dbRole ||
           SDB_ROLE_CATALOG    == dbRole ||
           SDB_ROLE_STANDALONE == dbRole ||
           SDB_ROLE_COORD      == dbRole )
      {
         // only data and standalone role load all collectionspaces
         if ( SDB_ROLE_DATA       == dbRole ||
              SDB_ROLE_STANDALONE == dbRole )
         {
            rc = rtnLoadCollectionSpaces ( krcb->getDBPath(),
                                           krcb->getIndexPath(), dmsCB ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to load collection spaces" ) ;
               goto error ;
            }
         }
         else if ( SDB_ROLE_CATALOG == dbRole )
         {
            rtnLoadCollectionSpace ( CAT_SYS_SPACE_NAME, krcb->getDBPath(),
                                     krcb->getIndexPath(), dmsCB, FALSE ) ;
            rtnLoadCollectionSpace ( AUTH_SPACE, krcb->getDBPath(),
                                     krcb->getIndexPath(), dmsCB, FALSE ) ;
            rtnLoadCollectionSpace ( CAT_PROCEDURES_SPACE_NAME,
                                     krcb->getDBPath(),
                                     krcb->getIndexPath(), dmsCB, FALSE ) ;
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
         rc = dpsCB->init( krcb->getLogPath(), krcb->getLogBufSize() ) ;
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
      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB() ;

      if ( krcb->getClsCB() && krcb->getReplCB() )
      {
         // stop repl-sync
         krcb->getReplCB()->setStatus( CLS_BS_CLOSED ) ;
         // wait all repl-sync log processed
         krcb->getReplCB()->getBucket()->waitEmpty() ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

#if defined (_LINUX)

   // this structure should not be inherited from SDBObject because it will be
   // assigned by array
   struct _signalInfo
   {
      const CHAR *name ;
      INT32       handle ;
   } ;

   static _signalInfo signalHandleMap [] = {
      { "Unknow", 0 },
      { "SIGHUP", 1 },     //1
      { "SIGINT", 1 },     //2
      { "SIGQUIT", 1 },    //3
      { "SIGILL", 1 },     //4
      { "SIGTRAP", 1 },    //5
      { "SIGABRT", 1 },    //6
      { "SIGBUS", 1 },     //7
      { "SIGFPE", 1 },     //8
      { "SIGKILL", 1 },    //9
      { "SIGUSR1", 0 },    //10
      { "SIGSEGV", 1 },    //11
      { "SIGUSR2", 0 },    //12
      { "SIGPIPE", 0 },    //13
      { "SIGALRM", 0 },    //14
      { "SIGTERM", 1 },    //15
      { "SIGSTKFLT", 0 },  //16
      { "SIGCHLD", 0 },    //17
      { "SIGCONT", 0 },    //18
      { "SIGSTOP", 1 },    //19
      { "SIGTSTP", 0 },    //20
      { "SIGTTIN", 0 },    //21
      { "SIGTTOU", 0 },    //22
      { "SIGURG", 0 },     //23
      { "SIGXCPU", 0 },    //24
      { "SIGXFSZ", 0 },    //25
      { "SIGVTALRM", 0 },  //26
      { "SIGPROF", 0 },    //27
      { "SIGWINCH", 0 },   //28
      { "SIGIO", 0 },      //29
      { "SIGPWR", 1 },     //30
      { "SIGSYS", 1 },     //31
      { "UNKNOW", 0 },     //32
      { "UNKNOW", 0 },     //33
      { "SIGRTMIN", 0 },   //34
      { "SIGRTMIN+1", 0 }, //35
      { "SIGRTMIN+2", 0 }, //36
      { "SIGRTMIN+3", 0 }, //37
      { "SIGRTMIN+4", 0 }, //38
      { "SIGTTMIN+5", 0 }, //39
      { "SIGRTMIN+6", 0 }, //40
      { "SIGRTMIN+7", 0 }, //41
      { "SIGTTMIN+8", 0 }, //42
      { "SIGRTMIN+9", 0 }, //43
      { "SIGRTMIN+10", 0 },//44
      { "SIGRTMIN+11", 0 },//45
      { "SIGRTMIN+12", 0 },//46
      { "SIGRTMIN+13", 0 },//47
      { "SIGRTMIN+14", 0 },//48
      { "SIGRTMIN+15", 0 },//49
      { "SIGRTMAX-14", 0 },//50
      { "SIGRTMAX-13", 0 },//51
      { "SIGRTMAX-12", 0 },//52
      { "SIGRTMAX-11", 0 },//53
      { "SIGRTMAX-10", 0 },//54
      { "SIGRTMAX-9", 0 }, //55
      { "SIGRTMAX-8", 0 }, //56
      { "SIGRTMAX-7", 0 }, //57
      { "SIGRTMAX-6", 0 }, //58
      { "SIGRTMAX-5", 0 }, //59
      { "SIGRTMAX-4", 0 }, //60
      { "SIGRTMAX-3", 0 }, //61
      { "SIGRTMAX-2", 0 }, //62
      { "SIGRTMAX-1", 0 }, //63
      { "SIGRTMAX", 0 },   //64
   };

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDSIGHND, "pmdSignalHandler" )
   void pmdSignalHandler ( INT32 sigNum )
   {
      PD_TRACE_ENTRY ( SDB_PMDSIGHND );
      if ( sigNum > 0 && sigNum <= OSS_MAX_SIGAL )
      {
         // avoid calling PD_LOG since localtime_r is not signal safe
         //PD_LOG ( PDEVENT, "Recieve signal[%d:%s, %s]",
         //   sigNum, signalHandleMap[sigNum].name,
         //   signalHandleMap[sigNum].handle ? "QUIT" : "IGNORE" ) ;
         if ( signalHandleMap[sigNum].handle ) // quit
         {
            PMD_SHUTDOWN_DB( SDB_INTERRUPT ) ;
         }
      }
      PD_TRACE_EXIT ( SDB_PMDSIGHND );
   }
#endif

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDSTPSIGHND, "pmdSetupSignalHandler" )
   INT32 pmdSetupSignalHandler ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDSTPSIGHND );
#if defined (_LINUX)
      ossSigSet sigSet ;
      struct sigaction newact ;
      ossMemset ( &newact, 0, sizeof(newact)) ;
      sigemptyset ( &newact.sa_mask ) ;
#endif
      pmdKRCB *krcb = pmdGetKRCB () ;
      ossSetInEngine () ;
      ossSetTrapExceptionPath ( krcb->getDiagLogPath() ) ;
#if defined (_LINUX)
      // Sigsegv
      // newact.sa_sigaction = ( OSS_SIGFUNCPTR ) ossSignalSigsegv ;
      newact.sa_sigaction = ( OSS_SIGFUNCPTR ) ossEDUCodeTrapHandler ;
      newact.sa_flags |= SA_SIGINFO ;
      newact.sa_flags |= SA_ONSTACK ;
      if ( sigaction ( SIGSEGV, &newact, NULL ) )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to setup signal handler for SigSegV" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      if ( sigaction ( SIGBUS, &newact, NULL ) )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to setup signal handler for SigBus" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      // stack dump signals
      newact.sa_sigaction = ( OSS_SIGFUNCPTR ) pmdEDUCodeTrapHandler ;
      newact.sa_flags |= SA_SIGINFO ;
      newact.sa_flags |= SA_ONSTACK ;
      // capture the user stack dump signal
      // 23 for linux
      if ( sigaction ( OSS_STACK_DUMP_SIGNAL, &newact, NULL ) )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to setup signal handler for dump signal" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      // capture the internal user stack dump signal
      if ( sigaction ( OSS_STACK_DUMP_SIGNAL_INTERNAL, &newact, NULL ) )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to setup signal handler for dump signal" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      sigSet.fillSet () ;
      sigSet.sigDel ( SIGSEGV ) ;
      sigSet.sigDel ( SIGBUS ) ;
      sigSet.sigDel ( SIGALRM ) ;
      sigSet.sigDel ( SIGPROF ) ;
      sigSet.sigDel ( OSS_STACK_DUMP_SIGNAL ) ;
      sigSet.sigDel ( OSS_STACK_DUMP_SIGNAL_INTERNAL ) ;
      rc = ossRegisterSignalHandle( sigSet, (SIG_HANDLE)pmdSignalHandler ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDWARNING, "Failed to register signals, rc = %d", rc ) ;
         // we do not abort startup process if any signal handler can't be
         // installed
         rc = SDB_OK ;
      }
   done :
#endif
      PD_TRACE_EXITRC ( SDB_PMDSTPSIGHND, rc );
      return rc ;
#if defined (_LINUX)
   error :
      goto done ;
#endif
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
      PD_RC_CHECK( rc, PDERROR, "Failed to init krcb, rc: %d", rc ) ;

      /*
       * This is the master thread
       * First we need to open configure files to load configurations
       * Then we should load all signal handlers and initialize
       * all global memory
       * Once we know if we are data node or coord node,
       * we are going to start related threads
       * basically we need to spawn tcp listener to receive network
       * request (for both data/coord nodes)
       * And then we put master thread into a loop to check global
       * "shutdown" status
       * If there's no shutdown request, we are going to keep loop
       * (with sleep to avoid eat all CPU)
       * The actual database works are done by agent threads
       */
      // read command line first
      rc = pmdResolveArguments ( argc, argv ) ;

      SDB_VALIDATE_GOTOERROR ( SDB_OK == rc,
                               rc, "Failed resolving arguments, exit" )
      if ( PMD_IS_DB_DOWN )
      {
         return rc ;
      }

      PD_LOG ( PDDEBUG, "Master thread starts" ) ;

      // handlers and init global mem
      rc = pmdSetupSignalHandler () ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to setup signal handler, rc: %d", rc ) ;

      // initialize variables
      rc = pmdSysInit () ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to initialize, rc: %d", rc ) ;

      dbrole = krcb->getDBRole () ;

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
      if ( SDB_ROLE_STANDALONE != dbrole && SDB_ROLE_COORD != dbrole )
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
      if ( SDB_ROLE_STANDALONE != dbrole && SDB_ROLE_COORD != dbrole )
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
      eduMgr->startEDU ( EDU_TYPE_HTTPLISTENER, NULL, &agentEDU ) ;
      eduMgr->regSystemEDU ( EDU_TYPE_HTTPLISTENER, agentEDU ) ;
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
         CHAR pmdProcessName [ PMD_ENGINE_NAME_BUF_LEN + 1 ] = {0} ;
         ossSnprintf ( pmdProcessName, PMD_ENGINE_NAME_BUF_LEN,
                       PMD_ENGINE_NAME_PATTERN, krcb->getServiceAddr() ) ;
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
      PD_LOG ( PDEVENT, "Master thread exits, exist code: %d",
               krcb->getExitCode() ) ;
      PD_TRACE_EXITRC ( SDB_PMDMSTTHRDMAIN, rc );
      return rc ;
   error :
      goto done ;
   }

#if defined (_WINDOWS)
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDCTRLHND, "pmdCtrlHandler" )
   BOOL pmdCtrlHandler( DWORD fdwCtrlType )
   {
      BOOLEAN ret = FALSE ;
      PD_TRACE_ENTRY ( SDB_PMDCTRLHND );
      switch( fdwCtrlType )
      {
      // Handle the CTRL-C signal.
      case CTRL_C_EVENT:
         PMD_SHUTDOWN_DB( SDB_INTERRUPT ) ;
         printf( "Ctrl-C event\n\n" );
         Beep( 750, 300 );
         ret = TRUE ;
         goto done ;

      // CTRL-CLOSE: confirm that the user wants to exit.
      case CTRL_CLOSE_EVENT:
         Beep( 600, 200 );
         printf( "Ctrl-Close event\n\n" );
         ret = TRUE ;
         goto done ;

      // Pass other signals to the next handler.
      case CTRL_BREAK_EVENT:
         Beep( 900, 200 );
         printf( "Ctrl-Break event\n\n" );
         ret = FALSE ;
         goto done ;

      case CTRL_LOGOFF_EVENT:
         Beep( 1000, 200 );
         printf( "Ctrl-Logoff event\n\n" );
         ret = FALSE ;
         goto done ;

      case CTRL_SHUTDOWN_EVENT:
         Beep( 750, 500 );
         printf( "Ctrl-Shutdown event\n\n" );
         ret = FALSE ;
         goto done ;

      default:
         ret = FALSE ;
         goto done ;
      }
   done :
      PD_TRACE1 ( SDB_PMDCTRLHND, PD_PACK_INT(ret) );
      PD_TRACE_EXIT ( SDB_PMDCTRLHND );
      return ret ;
   }
#endif

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDONETMINIT, "pmdOnetimeInit" )
   void pmdOnetimeInit()
   {
   #if defined (_WINDOWS)
      PD_TRACE_ENTRY ( SDB_PMDONETMINIT );
      ossSymMutexInit ( TRUE ) ;
      SetConsoleCtrlHandler( (PHANDLER_ROUTINE) pmdCtrlHandler, TRUE ) ;
      PD_TRACE_EXIT ( SDB_PMDONETMINIT );
   #endif
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
   engine::pmdOnetimeInit() ;
   rc = engine::pmdMasterThreadMain ( argc, argv ) ;
   PD_TRACE_EXITRC ( SDB_PMDMAIN, rc );
   return rc ;
}


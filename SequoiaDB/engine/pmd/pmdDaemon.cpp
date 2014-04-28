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
          10/09/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdDaemon.hpp"
#include "ossErr.h"
#include "pd.hpp"
#include "ossEDU.hpp"
#include "ossShMem.hpp"
#include "ossUtil.h"
#include "ossProc.hpp"
#include "pmd.hpp"
#include "pmdWinService.hpp"

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

#if defined (_LINUX)
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace engine
{
   iPmdDMNChildProc * cPmdDaemon::_process = NULL;

   _pmdDMNProcInfo::_pmdDMNProcInfo()
   {
      init();
   }

   void _pmdDMNProcInfo::init()
   {
      SDB_ASSERT( sizeof(PMDDMN_SHM_TAG) <= sizeof(szTag),
                  "share-memory-tag is out of length" );
      if ( !isInit() )
      {
         stat = PMDDMN_SHM_STAT_CHILDREN;
         pid = OSS_INVALID_PID;
         cmd = PMDDMN_SHM_CMD_INVALID;
         exitCode = SDB_OK;
         sn = 0;
         ossStrcpy( szTag, PMDDMN_SHM_TAG );
      }
   }

   BOOLEAN _pmdDMNProcInfo::isInit()
   {
      if ( ossStrcmp( szTag, PMDDMN_SHM_TAG ) == 0 )
      {
         return TRUE;
      }
      return FALSE;
   }

   INT32 _pmdDMNProcInfo::setDMNCMD( pmdDMNSHMCmd dmnCMD )
   {
      SDB_ASSERT( PMDDMN_SHM_CMD_INVALID
                  == ( dmnCMD & PMDDMN_SHM_CMD_CHL_MASK ),
                  "invalid daemon-command!" );
      if ( PMDDMN_SHM_CMD_INVALID ==
         ( cmd & PMDDMN_SHM_CMD_DMN_MASK ) )
      {
         cmd = cmd | ( dmnCMD & PMDDMN_SHM_CMD_DMN_MASK );
         return SDB_OK;
      }
      return SDB_PERM;
   }

   INT32 _pmdDMNProcInfo::getDMNCMD()
   {
      INT32 dmnCMD = cmd & PMDDMN_SHM_CMD_DMN_MASK;
      if ( PMDDMN_SHM_CMD_INVALID != dmnCMD )
      {
         cmd = cmd & PMDDMN_SHM_CMD_CHL_MASK;
      }
      return dmnCMD;
   }

   INT32 _pmdDMNProcInfo::setCHLCMD( pmdDMNSHMCmd chlCMD )
   {
      SDB_ASSERT( PMDDMN_SHM_CMD_INVALID
                  == ( chlCMD & PMDDMN_SHM_CMD_DMN_MASK ),
                  "invalid children-command!" );
      if ( PMDDMN_SHM_CMD_INVALID ==
         ( cmd & PMDDMN_SHM_CMD_CHL_MASK ) )
      {
         cmd = cmd | ( chlCMD & PMDDMN_SHM_CMD_CHL_MASK);
         return SDB_OK;
      }
      return SDB_PERM;
   }

   INT32 _pmdDMNProcInfo::getCHLCMD()
   {
      INT32 chlCMD = cmd & PMDDMN_SHM_CMD_CHL_MASK;
      if ( PMDDMN_SHM_CMD_INVALID != chlCMD )
      {
         cmd = cmd & PMDDMN_SHM_CMD_DMN_MASK;
      }
      return chlCMD;
   }

   iPmdDMNChildProc::iPmdDMNChildProc()
   {
      // first run: wait for 1 cycle to check if the child is start
      _deadTime = PMDDMN_SHMSTAT_EXPRIRED_TIMES - 1;
      _procInfo = NULL;
      _syncExit = TRUE;
      _pid = OSS_INVALID_PID;
      ossMemset( _execName, 0, sizeof( _execName) );
#if defined (_LINUX)
      _shmMid = -1;
      _shmKey = PMDDMN_SHMKEY_DEFAULT;
#elif defined (_WINDOWS)
      _shmMid = NULL;
      _shmKey = NULL;
#endif
   }

   iPmdDMNChildProc::~iPmdDMNChildProc()
   {
#if defined (_WINDOWS)
      SAFE_OSS_FREE( _shmKey );
#endif
   }

   INT32 iPmdDMNChildProc::init( ossSHMKey shmKey )
   {
      INT32 rc = SDB_OK;
      UINT32 len = 0;
#if defined (_WINDOWS)
      UINT32 keyLen = 0;
#endif
      ossMemset( _execName, 0, sizeof( _execName) );
      rc = ossGetEWD( _execName, OSS_MAX_PATHSIZE );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to get working directory(rc=%d)",
                  rc );
      len = ossStrlen( _execName );
      len = len + 1 + ossStrlen( getProgramName() );
#if defined (_WINDOWS)
      len += ossStrlen(".exe");
      keyLen = ossStrlen( shmKey );
      SAFE_OSS_FREE( _shmKey );
      _shmKey = (CHAR *)SDB_OSS_MALLOC( keyLen + 1 );
      if ( _shmKey )
      {
         ossStrcpy( _shmKey, shmKey );
      }
#elif defined ( _LINUX )
      _shmKey = shmKey;
#endif
      PD_CHECK( len <= OSS_MAX_PATHSIZE, SDB_INVALIDARG, error, PDERROR,
               "length of working directory is longer than expected!" );
      ossStrncat( _execName, OSS_FILE_SEP, 1 );
      ossStrncat( _execName, getProgramName(),
                  ossStrlen( getProgramName() ) );
#if defined (_WINDOWS)
      ossStrncat( _execName, ".exe", ossStrlen(".exe") );
#endif
   done:
      return rc;
   error:
      goto done;
   }

   INT32 iPmdDMNChildProc::allocSHM()
   {
      INT32 rc = SDB_OK;
#if defined ( _WINDOWS )
      PD_CHECK( _shmKey != NULL, SDB_OOM, error, PDERROR,
               "failed to get key because of malloc failed!" );
#endif
      rc = allocSHM( _shmKey );
#if defined ( _WINDOWS )
   done:
      return rc;
   error:
      goto done;
#else
   return rc;
#endif
   }

   INT32 iPmdDMNChildProc::allocSHM( ossSHMKey shmKey )
   {
      INT32 rc = SDB_OK;
      if ( _procInfo != NULL )
      {
         goto done;
      }
      ossSHMFree( _shmMid, (CHAR **)(&_procInfo) );
      _procInfo = ( pmdDMNProcInfo *)ossSHMAlloc( shmKey,
                                                sizeof( pmdDMNProcInfo ),
                                                OSS_SHM_CREATE, _shmMid );
#if defined (_LINUX)
      PD_CHECK( _procInfo != NULL && _shmMid >= 0, SDB_OOM, error, PDERROR,
               "failed to allocate share-memory(key:%u)", shmKey );
#elif defined (_WINDOWS)
      PD_CHECK( _procInfo != NULL && _shmMid != 0, SDB_OOM, error, PDERROR,
               "failed to allocate share-memory(key:%s)", shmKey );
#else
      rc = SDB_OOM;
      goto error;
#endif
      _procInfo->init();
   done:
      return rc;
   error:
      goto done;
   }

   void iPmdDMNChildProc::freeSHM()
   {
      ossSHMFree( _shmMid, (CHAR **)(&_procInfo) );
   }

   INT32 iPmdDMNChildProc::attachSHM()
   {
      INT32 rc = SDB_OK;
#if defined ( _WINDOWS )
      PD_CHECK( _shmKey != NULL, SDB_OOM, error, PDERROR,
               "failed to get key because of malloc failed!" );
#endif
      rc = attachSHM( _shmKey );
#if defined ( _WINDOWS )
   done:
      return rc;
   error:
      goto done;
#else
      return rc;
#endif
   }

   INT32 iPmdDMNChildProc::attachSHM( ossSHMKey shmKey )
   {
      INT32 rc = SDB_OK;
      if ( NULL != _procInfo )
      {
         goto done;
      }
      ossSHMFree( _shmMid, (CHAR **)(&_procInfo) );
      _procInfo = ( pmdDMNProcInfo *)ossSHMAttach( shmKey,
                                                sizeof( pmdDMNProcInfo ),
                                                _shmMid );
#if defined (_LINUX)
      PD_CHECK( _procInfo != NULL && _shmMid >= 0, SDB_OOM, error, PDERROR,
               "failed to get share-memory(key:%u)", shmKey );
#elif defined (_WINDOWS)
      PD_CHECK( _procInfo != NULL && _shmMid != 0, SDB_OOM, error, PDERROR,
               "failed to get share-memory(key:%s)", shmKey );
#else
      rc = SDB_OOM;
      goto error;
#endif
      _procInfo->init();
   done:
      return rc;
   error:
      goto done;
   }

   void iPmdDMNChildProc::detachSHM()
   {
      ossSHMDetach( _shmMid, (CHAR **)(&_procInfo) );
   }

   BOOLEAN iPmdDMNChildProc::isChildRunning()
   {
      SDB_ASSERT( _procInfo,
                  "_procInfo can't be null, call active at first" );
      BOOLEAN isRunning = FALSE;
      static BOOLEAN isFirstRun = TRUE;
      _pid = OSS_INVALID_PID;
      if ( _procInfo != NULL )
      {
         pmdDMNProcInfo procInfo = *_procInfo;
         if ( PMDDMN_SHM_STAT_CHILDREN != procInfo.stat )
         {
            INT32 rc = SDB_OK;
            isRunning = TRUE;
            _deadTime = 0;
            if ( OSS_INVALID_PID == procInfo.pid
               && !isFirstRun )
            {
               isRunning = FALSE;
            }
            else
            {
               isRunning = TRUE;
            }
            rc = DMNProcessCMD( _procInfo->getDMNCMD() );
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "Failed to process command(rc=%d)", rc ) ;
            }
            _pid = procInfo.pid;
            ++(_procInfo->sn);
            _procInfo->pid = OSS_INVALID_PID;
            _procInfo->stat = PMDDMN_SHM_STAT_CHILDREN;
         }
         else if ( _deadTime++ < PMDDMN_SHMSTAT_EXPRIRED_TIMES )
         {
            isRunning = TRUE;
         }
         else
         {
            _deadTime = 0;
         }
      }
      isFirstRun = FALSE;
      return isRunning;
   }

   INT32 iPmdDMNChildProc::DMNProcessCMD( INT32 cmd )
   {
      if ( PMDDMN_SHM_CMD_DMN_QUIT == cmd )
      {
         iPmdProc::stop();
         PD_LOG( PDEVENT, "stop by children-process!" );
      }
      return SDB_OK;
   }

   INT32 iPmdDMNChildProc::ChildProcessCMD( INT32 cmd )
   {
      return SDB_OK;
   }

   INT32 iPmdDMNChildProc::active()
   {
      INT32 rc = SDB_OK;
      rc = attachSHM();
      if ( SDB_OK == rc )
      {
         UINT32 i = 3;
         UINT32 sn = _procInfo->sn;
         while( i-- > 0 )
         {
            if ( sn != _procInfo->sn
               && (sn + 1) != _procInfo->sn )
            {
               rc = SDB_PERM;
               PD_RC_CHECK( rc, PDERROR, "Don't repeat start the process" );
            }
            ossSleep( PMDDMN_INTERVAL_TIME_DMN );
         }
      }
      else
      {
         rc = allocSHM();
         PD_RC_CHECK( rc, PDERROR, "Failed to allocate share-memory(rc=%d)",
                      rc ) ;
      }
   done:
      return rc;
   error:
      detachSHM();
      goto done;
   }

   void iPmdDMNChildProc::deactive()
   {
      freeSHM();
   }

   INT32 iPmdDMNChildProc::start()
   {
      INT32 rc = SDB_OK;
      ossResultCode result;
      OSSPID pid = OSS_INVALID_PID;
#if defined  (_LINUX)
      OSSPID childPid = OSS_INVALID_PID;
      childPid = fork();
      PD_CHECK( childPid >= 0, SDB_SYS, error, PDERROR,
               "fork() error!" );
      if ( 0 == childPid )
      {
#endif
      rc = ossExec( getExecuteFile(), getArguments(), NULL,
                     0, pid, result, NULL, NULL );
#if defined  (_LINUX)
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to start service(rc=%d)", rc );
      }
      exit(0);
      }
      else if ( -1 == childPid )
      {
         rc = SDB_SYS;
         PD_LOG( PDERROR, "Failed to fork!" );
      }
      else
      {
         waitpid( childPid, NULL, 0 );
      }
#endif

      PD_RC_CHECK( rc, PDERROR, "failed to start service(rc=%d)", rc );
   done:
      return rc;
   error:
      goto done;
   }

   INT32 iPmdDMNChildProc::stop()
   {
      if ( NULL != _procInfo )
      {
         INT32 tryTimes = 0;
         BOOLEAN isForce = FALSE;
         while( isChildRunning() )
         {
            if ( tryTimes++ >= PMDDMN_STOP_CHILD_MAX_TRY_TIMES )
            {
               isForce = TRUE;
            }
            if ( OSS_INVALID_PID != _pid )
            {
               ossTerminateProcess( _pid, isForce );
               ossSleep( PMDDMN_STOP_CHILD_WAIT_TIME );
               PD_LOG( PDEVENT, "stop the service process(%u)...",
                     _pid );
            }
            else if ( tryTimes >= PMDDMN_STOP_CHILD_MAX_TRY_TIMES )
            {
               break;
            }
            ossSleep( PMDDMN_INTERVAL_TIME_DMN );
         }
      }
      return SDB_OK;
   }

   void iPmdDMNChildProc::syncProcesserInfo()
   {
      _syncExit = FALSE;
      INT32 rc = SDB_OK;
      while( isRunning() )
      {
         rc = attachSHM();
         if ( SDB_OK == rc )
         {
            if ( _procInfo->isInit()
               && PMDDMN_SHM_STAT_DAEMON != _procInfo->stat )
            {
               _procInfo->pid = ossGetCurrentProcessID();
               rc = ChildProcessCMD( _procInfo->cmd );
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR,
                        "failed to process command(rc=%d)", rc );
               }
               ++(_procInfo->sn);
               _procInfo->stat = PMDDMN_SHM_STAT_DAEMON;
            }
            detachSHM();
         }
         ossSleep( PMDDMN_INTERVAL_TIME );
      }
      rc = attachSHM();
      if ( SDB_OK == rc )
      {
         if ( _procInfo->isInit() )
         {
            PD_LOG( PDEVENT, "stop service..." );
            rc = _procInfo->setDMNCMD( PMDDMN_SHM_CMD_DMN_QUIT );
            if ( SDB_OK !=rc )
            {
               PD_LOG( PDWARNING,
                     "daemon process is not stop!" );
            }
         }
         detachSHM();
      }
      _syncExit = TRUE;
   }

   INT32 iPmdDMNChildProc::startSyncThr()
   {
      boost::thread thrd( boost::bind( &iPmdDMNChildProc::syncProcesserInfo,
                           this ) );
      thrd.detach();
      return SDB_OK;
   }

   INT32 iPmdDMNChildProc::run( INT32 argc, CHAR **argv )
   {
      INT32 rc = SDB_OK;

      rc = startSyncThr();
      PD_RC_CHECK( rc, PDERROR, "Failed to start processor-info-sync"
                   "-thread(rc=%d)", rc ) ;
      rc = svcMain( argc, argv ) ;
      PD_RC_CHECK( rc, PDERROR, "Execute failed(rc=%d)", rc ) ;
      while ( !_syncExit )
      {
         ossSleep( PMDDMN_INTERVAL_TIME ) ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   const CHAR* iPmdDMNChildProc::getExecuteFile()
   {
      return _execName;
   }

   cPmdDaemon::cPmdDaemon( const CHAR *pDMNSvcName )
   {
      SDB_ASSERT( pDMNSvcName, "service name can't be null!" );

      UINT32 len = ossStrlen( pDMNSvcName );
      if ( len > 0 && len <= OSS_MAX_PATHSIZE )
      {
         ossStrcpy( _procName, pDMNSvcName );
      }
      else
      {
         ossStrcpy( _procName, PMDDMN_SVCNAME_DEFAULT );
      }
   }

   cPmdDaemon::~cPmdDaemon()
   {
      if ( _process != NULL )
      {
         _process->deactive();
         _process = NULL;
      }
   }

   INT32 cPmdDaemon::addChildrenProcess( iPmdDMNChildProc *childProc )
   {
      SDB_ASSERT( childProc, "childProc can't be null!" );
      _process = childProc;
      return _process->active();
   }

   INT32 cPmdDaemon::_run( INT32 argc, CHAR **argv )
   {
      INT32 rc = SDB_OK;
      INT32 retryTimes = 0;
      SDB_ASSERT( _process, "children process can't be null!" );
      if ( NULL == _process )
      {
         rc = SDB_INVALIDARG;
         goto error;
      }
      while( isRunning() )
      {
         /*if ( retryTimes >= _maxRetryTimes )
         {
            PD_RC_CHECK( rc, PDERROR,
                        "failed to start the process(retry times:%d), exit!",
                        retryTimes );
         }*/

         if ( !( _process->isChildRunning() ) )
         {
            PD_LOG( PDEVENT, "Start the service process...(times:%d)",
                    retryTimes );
            ++retryTimes;
            rc = _process->start();
            if ( SDB_OK != rc )
            {
               PD_LOG( PDWARNING, "Failed to start the process(rc=%d), "
                       "retrying...", rc ) ;
            }
         }
         else
         {
            retryTimes = 0;
         }
         ossSleep( PMDDMN_INTERVAL_TIME_DMN );
      }
   done:
      return rc;
   error:
      goto done;
   }

   INT32 cPmdDaemon::init()
   {
      SDB_ASSERT( _process, "children process can't be null!" ) ;
      return SDB_OK ;
   }

   INT32 cPmdDaemon::run( INT32 argc, CHAR **argv )
   {
      INT32 rc = SDB_OK;
#if defined (_WINDOWS)
      rc = pmdWinstartService( _procName, &cPmdDaemon::_run ) ;
#elif defined (_LINUX)
      ossEnableNameChanges ( argc, argv ) ;
      ossRenameProcess ( _procName ) ;
      rc = _run( argc, argv );
#endif
      PD_RC_CHECK( rc, PDERROR, "Failed to start the service(rc=%d)", rc ) ;
   done:
      return rc;
   error:
      goto done;
   }

   void cPmdDaemon::stop()
   {
      if ( NULL != _process )
      {
         _process->stop();
      }
   }
}
